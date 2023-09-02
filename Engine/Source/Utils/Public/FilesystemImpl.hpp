#pragma once

#include <filesystem>

namespace RightEngine
{
    class FilesystemImpl
    {
    public:
        virtual std::filesystem::path OpenFilesystemDialog(const std::vector<std::string>& filters) = 0;
        virtual std::filesystem::path SaveFilesystemDialog(const std::vector<std::string>& extensions = {}) = 0;
    };
}