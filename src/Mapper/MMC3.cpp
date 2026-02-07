#include "../../include/Mapper/MMC3.hpp"

#include <algorithm>

namespace HunNes {

u32 MMC3::getPrgBankFromReg(u8 reg) const {
    const u32 bankCount = std::max<u32>(1, prgCode.size() / 0x2000);
    return (reg & 0x3F) % bankCount;
}

u32 MMC3::mapPrgAddress(u16 address) const {
    const u32 bankCount = std::max<u32>(1, prgCode.size() / 0x2000);
    const u32 lastBank = bankCount - 1;
    const u32 secondLastBank = bankCount > 1 ? bankCount - 2 : lastBank;
    const bool prgMode = (bankSelect & 0x40) != 0;

    const u32 bank8000 = prgMode ? secondLastBank : getPrgBankFromReg(bankRegs[6]);
    const u32 bankA000 = getPrgBankFromReg(bankRegs[7]);
    const u32 bankC000 = prgMode ? getPrgBankFromReg(bankRegs[6]) : secondLastBank;
    const u32 bankE000 = lastBank;

    u32 selectedBank = 0;
    u16 bankOffset = 0;

    if (address >= 0x8000 && address <= 0x9FFF) {
        selectedBank = bank8000;
        bankOffset = address - 0x8000;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        selectedBank = bankA000;
        bankOffset = address - 0xA000;
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        selectedBank = bankC000;
        bankOffset = address - 0xC000;
    } else {
        selectedBank = bankE000;
        bankOffset = address - 0xE000;
    }

    return (selectedBank * 0x2000 + bankOffset) % prgCode.size();
}

u32 MMC3::mapChrAddress(u16 address) const {
    const u32 bankCount = std::max<u32>(1, chrROM.size() / 0x400);
    const u16 slot = (address / 0x400) & 0x7;
    const u16 slotOffset = address & 0x3FF;
    const bool chrMode = (bankSelect & 0x80) != 0;

    std::array<u8, 8> chrBanks = {};

    if (!chrMode) {
        chrBanks[0] = bankRegs[0] & 0xFE;
        chrBanks[1] = bankRegs[0] | 0x01;
        chrBanks[2] = bankRegs[1] & 0xFE;
        chrBanks[3] = bankRegs[1] | 0x01;
        chrBanks[4] = bankRegs[2];
        chrBanks[5] = bankRegs[3];
        chrBanks[6] = bankRegs[4];
        chrBanks[7] = bankRegs[5];
    } else {
        chrBanks[0] = bankRegs[2];
        chrBanks[1] = bankRegs[3];
        chrBanks[2] = bankRegs[4];
        chrBanks[3] = bankRegs[5];
        chrBanks[4] = bankRegs[0] & 0xFE;
        chrBanks[5] = bankRegs[0] | 0x01;
        chrBanks[6] = bankRegs[1] & 0xFE;
        chrBanks[7] = bankRegs[1] | 0x01;
    }

    const u32 bank = chrBanks[slot] % bankCount;
    return (bank * 0x400 + slotOffset) % chrROM.size();
}

u8 MMC3::read(u16 address) {
    if (address >= 0x6000 && address <= 0x7FFF) {
        if (!prgRamEnabled) {
            return 0;
        }
        return prgRam[address - 0x6000];
    }

    if (address < 0x8000 || prgCode.empty()) {
        return 0;
    }

    return prgCode[mapPrgAddress(address)];
}

void MMC3::write(u16 address, u8 data) {
    if (address >= 0x6000 && address <= 0x7FFF) {
        if (prgRamEnabled && !prgRamWriteProtected) {
            prgRam[address - 0x6000] = data;
        }
        return;
    }

    if (address < 0x8000) {
        return;
    }

    if (address >= 0x8000 && address <= 0x9FFF) {
        if ((address & 1) == 0) {
            bankSelect = data;
        } else {
            bankRegs[bankSelect & 0x07] = data;
        }
        return;
    }

    if (address >= 0xA000 && address <= 0xBFFF) {
        if ((address & 1) == 0) {
            // MMC3: 0 = vertical, 1 = horizontal.
            mirroring = (data & 1) ? 0 : 1;
        } else {
            prgRamWriteProtected = (data & 0x40) != 0;
            prgRamEnabled = (data & 0x80) != 0;
        }
        return;
    }

    if (address >= 0xC000 && address <= 0xDFFF) {
        if ((address & 1) == 0) {
            irqLatch = data;
        } else {
            irqReload = true;
        }
        return;
    }

    if ((address & 1) == 0) {
        irqEnabled = false;
    } else {
        irqEnabled = true;
    }
}

u8 MMC3::ppuread(u16 address) {
    if (address > 0x1FFF || chrROM.empty()) {
        return 0;
    }

    return chrROM[mapChrAddress(address)];
}

void MMC3::ppuwrite(u16 address, u8 data) {
    if (address > 0x1FFF || chrROM.empty()) {
        return;
    }

    chrROM[mapChrAddress(address)] = data;
}

}

