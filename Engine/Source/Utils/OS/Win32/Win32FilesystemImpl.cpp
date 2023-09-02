#include <Windows.h>
#include <Shlobj.h>

#include "Win32FilesystemImpl.hpp"
#include "Application.hpp"
#include "Filesystem.hpp"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <sstream>

using namespace RightEngine;

namespace
{
    std::string GetWin32FileFilterString(const std::vector<std::string>& filters)
    {
        // TODO: Implement proper filter builder
        std::stringstream filterSS;
        for (const auto& filter : filters)
        {
            filterSS << filter << "\0";
        }
        filterSS << "\0";
        return filterSS.str();
    }

    constexpr int MAX_PATH_SIZE = 260;
}

std::filesystem::path Win32FilesystemImpl::OpenFilesystemDialog(const std::vector<std::string>& filters)
{
    OPENFILENAMEA ofn;       // common dialog box structure
    CHAR szFile[MAX_PATH_SIZE] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNativeHandle()));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = GetWin32FileFilterString(filters).c_str();
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        std::string fp = ofn.lpstrFile;
        std::replace(fp.begin(), fp.end(), '\\', '/');
        return std::filesystem::path(fp);
    }

    return std::filesystem::path();
}

std::filesystem::path Win32FilesystemImpl::SaveFilesystemDialog(const std::vector<std::string>& extensions)
{
    OPENFILENAMEA ofn;
    CHAR szFile[MAX_PATH_SIZE] = { 0 };

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNativeHandle()));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("All Files(*.*)\0(*.*)");
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        std::string fp = ofn.lpstrFile;
        std::replace(fp.begin(), fp.end(), '\\', '/');
        return std::filesystem::path(fp);
    }

    return std::filesystem::path();
}

#ifdef R_WIN32
void Filesystem::CreateImpl()
{
    impl = std::make_shared<Win32FilesystemImpl>();
}
std::shared_ptr<FilesystemImpl> Filesystem::impl;
#endif
