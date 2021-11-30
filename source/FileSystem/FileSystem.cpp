#include "FileSystem.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

std::string File::executablePath;
std::string File::rootDirectory;
std::string File::shaderDirectory;

std::string File::readFromFile(const std::string& pPath)
{
    if (!exists(pPath))
    {
       LOG_ERROR("File with path {} doesnt exist", pPath);
       return "";
    }
    
    std::ifstream file;
    // ensure ifstream objects can throw exceptions
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        file.open(pPath);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();

        return stream.str();
    }
    catch (std::ifstream::failure e)
    {
        LOG_ERROR("File not successfully read, exception thrown: {}", e.what());
    }

    return "";
}

bool File::exists(const std::string& pPath)
{
    return std::filesystem::exists(pPath);
}

void File::setupDirectories(const std::string &pExecutePath)
{
    ZEPHYR_ASSERT(!pExecutePath.empty(), "Cannot initialise directories with no executable path given")
    executablePath = pExecutePath;
    std::replace(executablePath.begin(), executablePath.end(), '\\', '/');

    const auto &found = executablePath.find("Zephyr");
    ZEPHYR_ASSERT(found != std::string::npos, "Failed to find Zephyr in the supplied executable path {}", executeDirectory)

    rootDirectory = executablePath.substr(0, found + 6); // offset substr by length of "Zephyr"
    LOG_INFO("Root directory initialised to \"{}\"", rootDirectory);

    shaderDirectory = rootDirectory + "/source/Renderer/GraphicsContext/Shaders/";
    LOG_INFO("Shader directory initialised to \"{}\"", shaderDirectory);
};