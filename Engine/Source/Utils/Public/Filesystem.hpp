#pragma once

#include "FilesystemImpl.hpp"
#include <memory>

namespace RightEngine
{
    class Filesystem
    {
    public:
        static std::filesystem::path OpenFileDialog(const std::vector<std::string>& filters = {});
        static std::filesystem::path SaveFileDialog(const std::vector<std::string>& extensions = {});

        static void Init();

    private:
        static void CreateImpl();

        static std::shared_ptr<FilesystemImpl> impl;
    };
}
