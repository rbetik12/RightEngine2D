#pragma once

#include "Filesystem.hpp"

namespace RightEngine
{
    class Win32FilesystemImpl : public FilesystemImpl
    {
        virtual std::filesystem::path OpenFilesystemDialog(const std::vector<std::string>& filters) override;
        virtual std::filesystem::path SaveFilesystemDialog(const std::vector<std::string>& extensions = {}) override;
    };
}