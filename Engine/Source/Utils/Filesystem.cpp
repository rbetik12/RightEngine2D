#include "Filesystem.hpp"

using namespace RightEngine;

std::filesystem::path Filesystem::OpenFileDialog(const std::vector<std::string>& filters)
{
   return impl->OpenFilesystemDialog(filters);
}

std::filesystem::path Filesystem::SaveFileDialog(const std::vector<std::string>& extensions)
{
    return impl->SaveFilesystemDialog();
}

void Filesystem::Init()
{
    CreateImpl();
}
