#pragma once

#include "Event.hpp"

namespace RightEngine
{
    class KeyEvent : public Event
    {
    public:
        EVENT_BODY("KeyEvent")

        explicit KeyEvent(int keycode) : keycode(keycode)
        {}

        inline int GetKeyCode() const
        { return keycode; }

    private:
        int keycode;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        EVENT_BODY("KeyPressedEvent")

        KeyPressedEvent(int keycode, int repeatCount)
                : KeyEvent(keycode), repeatCount(repeatCount)
        {}

        inline int GetRepeatCount() const
        { return repeatCount; }

    private:
        int repeatCount;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        EVENT_BODY("KeyReleasedEvent")

        explicit KeyReleasedEvent(int keycode)
                : KeyEvent(keycode)
        {}
    };

    class KeyTypedEvent : public KeyEvent
    {
    public:
        EVENT_BODY("KeyTypedEvent")

        explicit KeyTypedEvent(int keycode)
                : KeyEvent(keycode)
        {}
    };
}