#pragma once

#include <cstring>
#include <string>
#include <functional>
#include "../../vgLib-2.0/helpers/Debugging.hpp"

// Platform-specific socket headers
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

struct OSCMessage {
    std::string address;
    float value;
    std::string stringValue;  // For string messages (file paths)
    bool isFloat;
    bool isInt;
    bool isString;
    bool hasParameters;  // New flag to indicate if message has parameters
};

class KaisekiReceiver {
private:
    int udpSocket = -1;
    bool enabled = false;
    int port = 7000;
    char buffer[2048];
    
#ifdef _WIN32
    bool wsaInitialized = false;
#endif
    
    // Statistics
    int messagesReceived = 0;
    int messagesDropped = 0;
    
    // Callback for when messages are received
    std::function<void(const OSCMessage&)> messageCallback;
    
public:
    KaisekiReceiver() {
#ifdef _WIN32
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
            wsaInitialized = true;
        }
#endif
    }
    
    ~KaisekiReceiver() {
        stop();
        
#ifdef _WIN32
        if (wsaInitialized) {
            WSACleanup();
        }
#endif
    }
    
    void setPort(int newPort) {
        if (enabled) {
            stop();
            port = newPort;
            start();
        } else {
            port = newPort;
        }
    }
    
    void setMessageCallback(std::function<void(const OSCMessage&)> callback) {
        messageCallback = callback;
    }
    
    bool start() {
        if (enabled) return true;
        
        // Create UDP socket
        udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpSocket < 0) {
            return false;
        }
        
        // Set socket to non-blocking mode
#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(udpSocket, FIONBIO, &mode);
#else
        int flags = fcntl(udpSocket, F_GETFL, 0);
        fcntl(udpSocket, F_SETFL, flags | O_NONBLOCK);
#endif
        
        // Allow address reuse
        int reuse = 1;
#ifdef _WIN32
        setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
#else
        setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
        
        // Bind to port
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        
        if (bind(udpSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
            closesocket(udpSocket);
#else
            close(udpSocket);
#endif
            udpSocket = -1;
            return false;
        }
        
        enabled = true;
        return true;
    }
    
    void stop() {
        if (!enabled) return;
        
        enabled = false;
        
        if (udpSocket >= 0) {
#ifdef _WIN32
            closesocket(udpSocket);
#else
            close(udpSocket);
#endif
            udpSocket = -1;
        }
    }
    
    void poll() {
        if (!enabled || udpSocket < 0) return;
        
        // Check for available data (non-blocking)
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(udpSocket, &readSet);
        
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;  // No timeout - immediate return
        
        int result = select(udpSocket + 1, &readSet, nullptr, nullptr, &timeout);
        
        if (result > 0 && FD_ISSET(udpSocket, &readSet)) {
            // Data is available
            struct sockaddr_in senderAddr;
            socklen_t senderAddrLen = sizeof(senderAddr);
            
#ifdef _WIN32
            int size = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                               (struct sockaddr*)&senderAddr, &senderAddrLen);
#else
            ssize_t size = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                                   (struct sockaddr*)&senderAddr, &senderAddrLen);
#endif
            
            if (size > 0) {
                parseOSCMessage(buffer, size);
            }
        }
    }
    
    bool isEnabled() const { return enabled; }
    int getPort() const { return port; }
    int getMessagesReceived() const { return messagesReceived; }
    int getMessagesDropped() const { return messagesDropped; }
    
    void resetCounters() {
        messagesReceived = 0;
        messagesDropped = 0;
    }
    
private:
    void parseOSCMessage(const char* data, int size) {
        if (size < 8) return;  // Too small to be valid OSC
        
        const char* ptr = data;
        const char* end = data + size;
        
        // Parse address pattern
        const char* addressStart = ptr;
        size_t addrLen = strnlen(addressStart, end - ptr);
        if (addrLen == 0 || addrLen >= 256) return;
        
        // Create clean address string
        std::string address(addressStart, addrLen);
        
        // Move past address with padding to 4-byte boundary
        ptr += addrLen + 1;  // +1 for null terminator
        while ((ptr - data) % 4 != 0 && ptr < end) ptr++;
        if (ptr >= end) return;
        
        // Parse type tag string
        if (*ptr != ',') return;  // Type tags must start with ','
        ptr++;  // Skip the ','
        
        // Store type tags
        const char* typeTagsStart = ptr;
        
        // Find end of type tags
        while (ptr < end && *ptr != '\0') ptr++;
        if (ptr >= end) return;
        ptr++;  // Skip null terminator
        
        // Pad to 4-byte boundary
        while ((ptr - data) % 4 != 0 && ptr < end) ptr++;
        if (ptr >= end) return;
        
        // Parse first argument based on type tag
        const char* tag = typeTagsStart;

        OSCMessage msg;
        msg.address = address;
        msg.isFloat = false;
        msg.isInt = false;
        msg.isString = false;
        msg.value = 0.0f;
        msg.hasParameters = (*tag != '\0');  // Check if there are parameters

        // DEBUG: Log detailed type tag information
        std::string typeTagStr(typeTagsStart, ptr - typeTagsStart - 1); // -1 to exclude null terminator
        VBUG << "OSC Parse: address=" << address
             << ", typeTagStr='" << typeTagStr << "'"
             << ", tagLen=" << (ptr - typeTagsStart - 1)
             << ", hasParams=" << msg.hasParameters
             << ", firstTag='" << *tag << "' (0x" << std::hex << (unsigned int)(unsigned char)*tag << std::dec << ")";

        // Handle messages with no arguments (like triggers)
        if (!msg.hasParameters) {
            VBUG << "OSC: Parameterless message detected: " << address;
            // Send the message even without parameters
            if (messageCallback) {
                messageCallback(msg);
                messagesReceived++;
            }
            return;
        }

        switch (*tag) {
            case 'f': {  // 32-bit float
                if (ptr + 4 > end) return;

                // Read 32-bit big-endian float
                uint32_t intBits =
                    ((uint32_t)(unsigned char)ptr[0] << 24) |
                    ((uint32_t)(unsigned char)ptr[1] << 16) |
                    ((uint32_t)(unsigned char)ptr[2] << 8) |
                    ((uint32_t)(unsigned char)ptr[3]);

                float value;
                std::memcpy(&value, &intBits, sizeof(float));

                msg.value = value;
                msg.isFloat = true;

                // DEBUG: Log float values, especially zeros
                VBUG << "OSC Float parsed: " << address
                     << " = " << value
                     << " (raw bits: 0x" << std::hex << intBits << std::dec << ")";
                break;
            }
            case 'i': {  // 32-bit int
                if (ptr + 4 > end) return;

                // Read 32-bit big-endian int
                int32_t intVal =
                    ((int32_t)(unsigned char)ptr[0] << 24) |
                    ((int32_t)(unsigned char)ptr[1] << 16) |
                    ((int32_t)(unsigned char)ptr[2] << 8) |
                    ((int32_t)(unsigned char)ptr[3]);

                msg.value = static_cast<float>(intVal);
                msg.isInt = true;
                break;
            }
            case 's': {  // OSC string
                // String starts at ptr
                const char* stringStart = ptr;
                size_t strLen = strnlen(stringStart, end - ptr);

                msg.stringValue = std::string(stringStart, strLen);
                msg.isString = true;
                break;
            }
            default:
                // Unsupported type
                return;
        }
        
        messagesReceived++;
        
        // Call the callback if set
        if (messageCallback) {
            messageCallback(msg);
        }
    }
};