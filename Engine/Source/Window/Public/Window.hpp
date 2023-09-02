#pragma once

#include <string>
#include <utility>
#include <memory>

namespace RightEngine
{
    class Surface;
    class Swapchain;

    class Window
    {
    public:
        inline uint32_t GetWidth() const
        { return width; }

        inline uint32_t GetHeight() const
        { return height; }

        virtual void OnUpdate() = 0;
        virtual void Swap() const = 0;

        virtual void* GetNativeHandle() const = 0;

        const std::shared_ptr<Surface>& GetSurface() const
        { return surface; }
        void SetSurface(const std::shared_ptr<Surface>& aSurface)
        { surface = aSurface; }

        static Window *Create(std::string title, uint32_t width, uint32_t height);

    protected:
        Window(const std::string& title, uint32_t width, uint32_t height) : title(title), width(width),
                                                                     height(height)
        {}

        std::string title;
        uint32_t width;
        uint32_t height;
        std::shared_ptr<Surface> surface;
    };
}