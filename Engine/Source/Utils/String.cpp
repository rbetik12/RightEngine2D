#include "String.hpp"

using namespace RightEngine;


std::vector<std::string> String::Split(std::string string, const std::string& delimiter)
{
    std::vector<std::string> strings;
    size_t pos = 0;
    std::string token;
    while ((pos = string.find(delimiter)) != std::string::npos) {
        token = string.substr(0, pos);
        strings.push_back(token);
        string.erase(0, pos + delimiter.length());
    }

    strings.push_back(string);

    return strings;
}
