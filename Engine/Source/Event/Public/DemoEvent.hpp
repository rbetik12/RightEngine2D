#pragma once

#include "Event.hpp"

class DemoEvent : public Event
{
public:
    EVENT_BODY("DemoEvent")
};