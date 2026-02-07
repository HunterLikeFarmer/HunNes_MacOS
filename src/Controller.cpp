#include "../include/Controller.hpp"

namespace HunNes {

// read the controller value is by serial shifting the bit
u8 Controller::read(u16 address) {
    // MMIO is mapping joy stick 1 value to address 0x4016
    if (address == 0x4016) {
        if (strobe) {
            return 0x40 | (btnState & 1);
        }

        JOY1 = 0x80 | (btnStateLocked & 1);
        btnStateLocked >>= 1;
        return JOY1;
    } else {
        return JOY2;
    }
}

void Controller::write(u16 address, u8 data) {
    if (address == 0x4016) {
        if (strobe && !(data & 0x1)) {
            btnStateLocked = btnState;
        }

        strobe = data & 0x1;
    } else {
    }
}

// update the bit states
void Controller::setButtonPressed(ControllerButton button, bool pressed) {
    u8 bit = static_cast<u8>(button);
    u8 mask = static_cast<u8>(1U << bit);
    btnState = pressed ? static_cast<u8>(btnState | mask) : static_cast<u8>(btnState & ~mask);
}

}  
