#pragma once

#include <array>

#include "Mapper.hpp"

namespace HunNes {

class MMC3 : public Mapper {
   public:
    MMC3(std::vector<u8> &prgCode, std::vector<u8> &chrROM, int mirroring) : Mapper(prgCode, chrROM, mirroring) {}
    ~MMC3() override = default;

    u8 read(u16 address) override;
    void write(u16 address, u8 data) override;
    u8 ppuread(u16 address) override;
    void ppuwrite(u16 address, u8 data) override;

   private:
    u32 mapPrgAddress(u16 address) const;
    u32 mapChrAddress(u16 address) const;
    u32 getPrgBankFromReg(u8 reg) const;

    u8 bankSelect = 0;
    std::array<u8, 8> bankRegs = {0, 0, 0, 0, 0, 0, 0, 1};
    std::array<u8, 0x2000> prgRam = {};
    bool prgRamEnabled = true;
    bool prgRamWriteProtected = false;

    // Kept for register compatibility; IRQ signaling is not wired into the CPU yet.
    u8 irqLatch = 0;
    bool irqReload = false;
    bool irqEnabled = false;
};

};

