#pragma once
#include <string>
#include <sstream> // Required for std::ostringstream

// Include rack.hpp for the standard DEBUG macro
#include "rack.hpp"

// Default to production mode unless DEVELOPMENT is explicitly defined
// #define DEVELOPMENT

namespace vgLib_v2
{
    /**
     * @class VbugStreamer
     * @brief A temporary helper class to capture streamed output for VBUG.
     *
     * This class uses the RAII pattern to collect streamed output and send it to
     * VCV Rack's DEBUG macro. Only active when DEVELOPMENT is defined.
     */
    class VbugStreamer
    {
    private:
        #ifdef DEVELOPMENT
        std::ostringstream stream_; // Use ostringstream to build the string
        #endif

    public:
        // Constructor - currently empty
        VbugStreamer() {}
        
        // Destructor - DEBUG call happens here (only in development builds)
        ~VbugStreamer()
        {
            #ifdef DEVELOPMENT
            // Get the complete string from the stream
            std::string output = stream_.str();
            // Call the VCV Rack DEBUG macro with the C-style string
            DEBUG("%s", output.c_str());
            #endif
        }
        
        // Overload operator<< for various types using a template
        template <typename T>
        VbugStreamer& operator<<(const T& value)
        {
            #ifdef DEVELOPMENT
            stream_ << value; // Append data to the internal stream
            #endif
            return *this;     // Return *this to allow chaining
        }
        
        // Overload for stream manipulators like std::endl
        typedef std::ostream& (*ManipFn)(std::ostream&);
        VbugStreamer& operator<<(ManipFn manip) {
            #ifdef DEVELOPMENT
            manip(stream_);
            #endif
            return *this;
        }
    };
} // namespace vgLib_v2

/**
 * @def VBUG
 * @brief A conditional debugging macro for vgLib-2.0.
 *
 * In development builds, creates a VbugStreamer that forwards messages to VCV Rack's DEBUG.
 * In regular/production builds, compiles to a no-op.
 */
#ifdef DEVELOPMENT
    // In development builds, VBUG works normally
    #define VBUG vgLib_v2::VbugStreamer()
#else
    // In production builds, VBUG does nothing
    #define VBUG if(false) vgLib_v2::VbugStreamer()
#endif