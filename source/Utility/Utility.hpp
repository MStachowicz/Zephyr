#pragma once

#include <functional>
#include <filesystem>
#include <string>
#include <random>

#include "glm/gtx/hash.hpp"
#include "Logger.hpp"

namespace util
{
    template <typename T>
    static constexpr auto toIndex(const T& pEnum) noexcept // Returns the underlying type. Used to convert enum types to indexes into arrays
    {
        return static_cast<std::underlying_type_t<T>>(pEnum);
    }

    // Produces a random floating-point value in the interval min to max
    // Satisfies all requirements of RandomNumberDistribution (https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution)
    template<class T>
    static T getRandom(const T& pMin, const T& pMax)
    {
        // Use std::uniform_real_distribution to transform the random unsigned int generated by std::mt19937 into a type in [pMin, pMax).
        // Each call to dis(gen) generates a new random.

        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<T> dis(pMin, pMax);
        return dis(gen);
    }

    // Produces random floating-point values in the interval min to max
    // Satisfies all requirements of RandomNumberDistribution (https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution)
    template<class T, size_t Size>
    static void fillRandomNumbers(const T& pMin, const T& pMax, std::array<T, Size>& pArray)
    {
        // Use std::uniform_real_distribution to transform the random unsigned int generated by std::mt19937 into a type in [pMin, pMax).
        // Each call to dis(gen) generates a new random.

        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<T> dis(pMin, pMax);

        for (size_t i = 0; i < pArray.size(); i++)
            pArray[i] = dis(gen);
    }

    // New-from-old combination function for hash values.
    // Takes the previous seed value pSeed and hashes pValue onto it.
    template <class T>
    inline void HashCombine(std::size_t& pSeed, const T& pValue)
    {
        // The magic number "0x9e3779b9" is supposed to be 32 random bits, where each is equally likely to be 0 or 1, and with no simple correlation between the bits.
        // Uses a common way to find a string of such bits using the binary expansion of an irrational number; in this case the reciprocal of the golden ratio:
        // phi = (1 + sqrt(5)) / 2
        // 2^32 / phi = 0x9e3779b9
        // So including this number 'randomly' changes each bit of the seed; this means that consecutive values will be far apart. Including the shifted versions of the old seed makes sure that,
        // even if hash_value() has a fairly small range of values, differences will soon be spread across all the bits.
        std::hash<T> hasher;
        pSeed ^= hasher(pValue) + 0x9e3779b9 + (pSeed << 6) + (pSeed >> 2);
    }

    inline glm::mat4 GetModelMatrix(const glm::vec3& pPosition, const glm::vec3& pRotation, const glm::vec3& pScale)
    {
        glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pPosition);
        model = glm::rotate(model, glm::radians(pRotation.x), glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, glm::radians(pRotation.y), glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, glm::radians(pRotation.z), glm::vec3(0.0, 0.0, 1.0));
        model = glm::scale(model, pScale);
        return model;
    }

    class File
    {
    public:
        static inline std::filesystem::path executablePath;
        static inline std::filesystem::path rootDirectory;
        static inline std::filesystem::path GLSLShaderDirectory;
        static inline std::filesystem::path textureDirectory;

        static bool exists(const std::filesystem::path& pPath)  { return std::filesystem::exists(pPath); }
        //static bool exists(const std::string& pPath)            { return std::filesystem::exists(pPath); }

        static void initialise(const std::string& pExecutePath)
        {
            std::string exectuablePathStr = pExecutePath;
            std::replace(exectuablePathStr.begin(), exectuablePathStr.end(), '\\', '/');
            executablePath = exectuablePathStr;
            ZEPHYR_ASSERT(util::File::exists(executablePath), "Could not find the exectuable path")

            const auto &found = exectuablePathStr.find("Zephyr");
            ZEPHYR_ASSERT(found != std::string::npos, "Failed to find Zephyr string in the supplied executable path {}", executablePath.string()) // #C++20 if switched logger to use std::format, direct use of std::filesystem::path is available
            rootDirectory = exectuablePathStr.substr(0, found + 6); // offset substr by length of "Zephyr"
            ZEPHYR_ASSERT(util::File::exists(rootDirectory), "Could not find the rootDirectory path")

            GLSLShaderDirectory = rootDirectory.string() + "/source/OpenGLAPI/GLSL/";
            ZEPHYR_ASSERT(util::File::exists(GLSLShaderDirectory), "Could not find the GLSL shader directory")

            textureDirectory = rootDirectory.string() + "/source/Resources/Textures/";
            ZEPHYR_ASSERT(util::File::exists(textureDirectory), "Could not find the texture directory")

            LOG_INFO("Executable location initialised to: \"{}\"", executablePath.string());
            LOG_INFO("Root directory initialised to: \"{}\"", rootDirectory.string());
            LOG_INFO("Texture directory initialised to: \"{}\"", textureDirectory.string());
            LOG_INFO("GLSL Shader directory initialised to: \"{}\"", GLSLShaderDirectory.string());
        }

        static void ForEachFile(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction)
        {
            ZEPHYR_ASSERT(util::File::exists(pDirectory), "Directory does not exist, cannot iterate over its contents.")

            for (const auto& entry : std::filesystem::directory_iterator(pDirectory))
                pFunction(entry);
        }
        static void ForEachFileRecursive(const std::filesystem::path& pDirectory, const std::function<void(const std::filesystem::directory_entry& pEntry)>& pFunction)
        {
            ZEPHYR_ASSERT(util::File::exists(pDirectory), "Directory does not exist, cannot iterate over its contents.")

            for (const auto& entry : std::filesystem::recursive_directory_iterator(pDirectory))
                pFunction(entry);
        }
    };
}