#pragma once

#include <array>

#include "Mapper.hpp"

namespace HunNes {

class MMC5 : public Mapper {
   public:
    MMC5(std::vector<u8> &prgCode, std::vector<u8> &chrROM, int mirroring);
    ~MMC5() override = default;

    u8 read(u16 address) override;
    void write(u16 address, u8 data) override;
    u8 ppuread(u16 address) override;
    void ppuwrite(u16 address, u8 data) override;

   private:
    static constexpr u32 kPrgRamSize = 128 * 1024;

    u8 readMappedPrg(u8 reg, u16 offset, u16 windowSize, bool forceRom) const;
    void writeMappedPrg(u8 reg, u16 offset, u16 windowSize, bool forceRom, u8 data);
    u32 mapChrAddress(u16 address) const;
    bool prgRamWritable() const;

    std::array<u8, kPrgRamSize> prgRam = {};

    u8 prgMode = 3;
    u8 chrMode = 3;
    u8 upperChrBits = 0;
    u8 prgRamProtect1 = 0;
    u8 prgRamProtect2 = 0;

    u8 prgRamBank = 0;
    std::array<u8, 4> prgBankRegs = {};

    std::array<u16, 8> chrRegsA = {};
    std::array<u16, 4> chrRegsB = {};
};

};

