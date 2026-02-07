#pragma once

#include "Common/Typedefs.hpp"
#include "INESBus.hpp"

namespace HunNes {

enum class ControllerButton : u8 {
    A = 0,
    B = 1,
    Select = 2,
    Start = 3,
    Up = 4,
    Down = 5,
    Left = 6,
    Right = 7
};

class Controller : INESBus {
    u8 JOY1 = 0;
    u8 JOY2 = 0;
    // A latched state to shift bits to CPU
    u8 btnStateLocked = 0;
    // bit state represent the 8 states of 8 input (bit masking)
    // Bit 0 A, Bit 1 B, Bit 2 Select ...
    u8 btnState = 0;
    bool strobe = false;

   public:
    //Bus
    u8 read(u16 address);
    void write(u16 address, u8 data);

    //Input
    void setButtonPressed(ControllerButton button, bool pressed);
};

};  
