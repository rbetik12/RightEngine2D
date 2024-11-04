#pragma once

#include <Engine/Service/IService.hpp>
#include <Engine/Service/Window/Keycodes.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace engine
{

// Service is responsible for current window drawing and input events processing
class ENGINE_API WindowService final : public Service<WindowService>
{
public:
    WindowService();
    virtual ~WindowService() override;

    virtual void        Update(float dt) override;
    virtual void        PostUpdate(float dt) override;

    glm::ivec2          Extent() const;
    glm::vec2           WindowScale() const;

    GLFWwindow*         Window() { return m_window; }
    const GLFWwindow*   Window() const { return m_window; }

    // All keycodes are in GLFW documentation
    // https://www.glfw.org/docs/3.3/group__keys.html
    bool                KeyButtonPressed(KeyButton button) const;

    // All mouse buttons are written here
    // https://www.glfw.org/docs/3.3/group__buttons.html
    bool                MouseButtonPressed(MouseButton button) const;

    glm::vec2           MousePos() const;
    // Mouse position from the last frame
    glm::vec2           PrevMousePos() const;

private:
    GLFWwindow* m_window = nullptr;
};

} // namespace engine
