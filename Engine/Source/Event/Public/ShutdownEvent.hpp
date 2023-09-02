#pragma once

#include "Event.hpp"

class ShutdownEvent: public Event
{
public:
    EVENT_BODY("ShutdownEvent")
};