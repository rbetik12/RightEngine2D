#pragma once

#include "Event.hpp"
#include <glm/vec2.hpp>

class MouseMovedEvent : public Event
{
public:
    EVENT_BODY("MouseMovedEvent")

    MouseMovedEvent(float x, float y) : mouseX(x), mouseY(y)
    {}

    inline const float& GetX() const
    { return mouseX; }

    inline const float& GetY() const
    { return mouseY; }

private:
    float mouseX, mouseY;
};

class MouseButtonPressedEvent : public Event
{
public:
    EVENT_BODY("MouseButtonPressedEvent");

    explicit MouseButtonPressedEvent(int button) : button(button)
    {}

    inline const int& GetButton() const
    { return button; }

private:
    int button;
};

class MouseButtonReleasedEvent : public MouseButtonPressedEvent
{
public:
    EVENT_BODY("MouseButtonReleasedEvent");

    explicit MouseButtonReleasedEvent(int button) : MouseButtonPressedEvent(button)
    {}
};

class MouseScrollEvent : public Event
{
public:
    EVENT_BODY("MouseScrollEvent");

    explicit MouseScrollEvent(double xOffset, double yOffset) : offset(xOffset, yOffset)
    {}

    const glm::vec2& GetOffset() const
    {
        return offset;
    }

private:
    glm::vec2 offset;
};

