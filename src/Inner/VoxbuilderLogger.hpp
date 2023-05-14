#ifndef VOXBUILDER_LOGGER_HPP
#define VOXBUILDER_LOGGER_HPP

#include <fstream>
#include <string>

class VoxbuilderLogger
{
public:
    static VoxbuilderLogger& getInstance()
    {
        static VoxbuilderLogger instance; // Guaranteed to be destroyed, instantiated on first use.
        return instance;
    }

    void log(const std::string& message)
    {
        logFile << message << std::endl;
    }

private:
    std::ofstream logFile;

    // Constructor
    VoxbuilderLogger()
    {
        std::string log_file_path = rack::asset::user("voxbuilder.log");
        logFile.open(log_file_path);
    }

    // C++ 11
    // =======
    // We can use the better technique of deleting the methods
    // we don't want.
    VoxbuilderLogger(VoxbuilderLogger const&) = delete;
    void operator=(VoxbuilderLogger const&)   = delete;
};

#endif // VOXBUILDER_LOGGER_HPP