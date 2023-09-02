#pragma once

#include <vector>
#include <string>

namespace RightEngine
{
    class String
    {
    public:
        static std::vector<std::string> Split(std::string string, const std::string& delimiter);
    };
}
