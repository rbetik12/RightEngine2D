#pragma once

#include <string>
#include <unordered_map>

namespace RightEngine
{
    class Path
    {
    public:
        static void Init();

        /*
         * Gets path in engine format and returns absolute path to the resource
         */
        static std::string Absolute(const std::string& enginePath);

        /*
         * Gets absolute path and returns one in engine format
         */
        static std::string Engine(const std::string& absolutePath);

    private:
        static std::unordered_map<std::string, std::string> aliasMap;
        static bool initialized;
    };

}
