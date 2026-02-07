#include "../../include/Mapper/MMC5.hpp"

#include <algorithm>

namespace HunNes {

MMC5::MMC5(std::vector<u8> &prgCode, std::vector<u8> &chrROM, int mirroring) : Mapper(prgCode, chrROM, mirroring) {
    const u32 prg8kBanks = std::max<u32>(1, prgCode.size() / 0x2000);
    const u8 bank0 = static_cast<u8>((prg8kBanks > 3 ? prg8kBanks - 4 : 0) & 0x7F);
    const u8 bank1 = static_cast<u8>((prg8kBanks > 2 ? prg8kBanks - 3 : 0) & 0x7F);
    const u8 bank2 = static_cast<u8>((prg8kBanks > 1 ? prg8kBanks - 2 : 0) & 0x7F);
    const u8 bank3 = static_cast<u8>((prg8kBanks - 1) & 0x7F);

    // Power-on default should map the reset vector from the end of PRG-ROM.
    prgBankRegs[0] = 0x80 | bank0;
    prgBankRegs[1] = 0x80 | bank1;
    prgBankRegs[2] = 0x80 | bank2;
    prgBankRegs[3] = 0x80 | bank3;
}

bool MMC5::prgRamWritable() const {
    // MMC5 requires a two-step unlock sequence for PRG-RAM writes.
    return (prgRamProtect1 & 0x03) == 0x02 && (prgRamProtect2 & 0x03) == 0x01;
}

u8 MMC5::readMappedPrg(u8 reg, u16 offset, u16 windowSize, bool forceRom) const {
    const bool useRom = forceRom || (reg & 0x80) != 0;

    u8 sizeMask = 0x7F;
    if (windowSize == 0x4000) {
        sizeMask = 0x7E;
    } else if (windowSize == 0x8000) {
        sizeMask = 0x7C;
    }

    if (useRom) {
        if (prgCode.empty()) {
            return 0;
        }

        const u32 bankCount = std::max<u32>(1, prgCode.size() / 0x2000);
        const u32 bank = (reg & sizeMask) % bankCount;
        const u32 prgAddress = (bank * 0x2000 + offset) % prgCode.size();
        return prgCode[prgAddress];
    }

    const u32 ramBanks = kPrgRamSize / 0x2000;
    const u32 bank = (reg & sizeMask) % ramBanks;
    const u32 ramAddress = (bank * 0x2000 + offset) % kPrgRamSize;
    return prgRam[ramAddress];
}

void MMC5::writeMappedPrg(u8 reg, u16 offset, u16 windowSize, bool forceRom, u8 data) {
    const bool useRom = forceRom || (reg & 0x80) != 0;
    if (useRom || !prgRamWritable()) {
        return;
    }

    u8 sizeMask = 0x7F;
    if (windowSize == 0x4000) {
        sizeMask = 0x7E;
    } else if (windowSize == 0x8000) {
        sizeMask = 0x7C;
    }

    const u32 ramBanks = kPrgRamSize / 0x2000;
    const u32 bank = (reg & sizeMask) % ramBanks;
    const u32 ramAddress = (bank * 0x2000 + offset) % kPrgRamSize;
    prgRam[ramAddress] = data;
}

u8 MMC5::read(u16 address) {
    if (address >= 0x6000 && address <= 0x7FFF) {
        const u32 ramBanks = kPrgRamSize / 0x2000;
        const u32 bank = (prgRamBank & 0x7F) % ramBanks;
        const u32 ramAddress = bank * 0x2000 + (address - 0x6000);
        return prgRam[ramAddress];
    }

    if (address < 0x8000) {
        return 0;
    }

    if (prgMode == 0) {
        return readMappedPrg(prgBankRegs[3], address - 0x8000, 0x8000, true);
    }

    if (prgMode == 1) {
        if (address < 0xC000) {
            return readMappedPrg(prgBankRegs[1], address - 0x8000, 0x4000, false);
        }
        return readMappedPrg(prgBankRegs[3], address - 0xC000, 0x4000, true);
    }

    if (prgMode == 2) {
        if (address < 0xC000) {
            return readMappedPrg(prgBankRegs[1], address - 0x8000, 0x4000, false);
        }
        if (address < 0xE000) {
            return readMappedPrg(prgBankRegs[2], address - 0xC000, 0x2000, false);
        }
        return readMappedPrg(prgBankRegs[3], address - 0xE000, 0x2000, true);
    }

    if (address < 0xA000) {
        return readMappedPrg(prgBankRegs[0], address - 0x8000, 0x2000, false);
    }
    if (address < 0xC000) {
        return readMappedPrg(prgBankRegs[1], address - 0xA000, 0x2000, false);
    }
    if (address < 0xE000) {
        return readMappedPrg(prgBankRegs[2], address - 0xC000, 0x2000, false);
    }
    return readMappedPrg(prgBankRegs[3], address - 0xE000, 0x2000, true);
}

void MMC5::write(u16 address, u8 data) {
    if (address >= 0x5100 && address <= 0x5FFF) {
        switch (address) {
            case 0x5100:
                prgMode = data & 0x03;
                break;
            case 0x5101:
                chrMode = data & 0x03;
                break;
            case 0x5102:
                prgRamProtect1 = data;
                break;
            case 0x5103:
                prgRamProtect2 = data;
                break;
            case 0x5105:
                // Nametable mapping is not currently delegated to mapper logic.
                break;
            case 0x5113:
                prgRamBank = data;
                break;
            case 0x5114:
            case 0x5115:
            case 0x5116:
            case 0x5117:
                prgBankRegs[address - 0x5114] = data;
                break;
            case 0x5120:
            case 0x5121:
            case 0x5122:
            case 0x5123:
            case 0x5124:
            case 0x5125:
            case 0x5126:
            case 0x5127:
                chrRegsA[address - 0x5120] = static_cast<u16>(((upperChrBits & 0x03) << 8) | data);
                break;
            case 0x5128:
            case 0x5129:
            case 0x512A:
            case 0x512B:
                chrRegsB[address - 0x5128] = static_cast<u16>(((upperChrBits & 0x03) << 8) | data);
                break;
            case 0x5130:
                upperChrBits = data & 0x03;
                break;
            default:
                break;
        }
        return;
    }

    if (address >= 0x6000 && address <= 0x7FFF) {
        if (prgRamWritable()) {
            const u32 ramBanks = kPrgRamSize / 0x2000;
            const u32 bank = (prgRamBank & 0x7F) % ramBanks;
            const u32 ramAddress = bank * 0x2000 + (address - 0x6000);
            prgRam[ramAddress] = data;
        }
        return;
    }

    if (address < 0x8000) {
        return;
    }

    if (prgMode == 0) {
        writeMappedPrg(prgBankRegs[3], address - 0x8000, 0x8000, true, data);
        return;
    }

    if (prgMode == 1) {
        if (address < 0xC000) {
            writeMappedPrg(prgBankRegs[1], address - 0x8000, 0x4000, false, data);
        } else {
            writeMappedPrg(prgBankRegs[3], address - 0xC000, 0x4000, true, data);
        }
        return;
    }

    if (prgMode == 2) {
        if (address < 0xC000) {
            writeMappedPrg(prgBankRegs[1], address - 0x8000, 0x4000, false, data);
        } else if (address < 0xE000) {
            writeMappedPrg(prgBankRegs[2], address - 0xC000, 0x2000, false, data);
        } else {
            writeMappedPrg(prgBankRegs[3], address - 0xE000, 0x2000, true, data);
        }
        return;
    }

    if (address < 0xA000) {
        writeMappedPrg(prgBankRegs[0], address - 0x8000, 0x2000, false, data);
    } else if (address < 0xC000) {
        writeMappedPrg(prgBankRegs[1], address - 0xA000, 0x2000, false, data);
    } else if (address < 0xE000) {
        writeMappedPrg(prgBankRegs[2], address - 0xC000, 0x2000, false, data);
    } else {
        writeMappedPrg(prgBankRegs[3], address - 0xE000, 0x2000, true, data);
    }
}

u32 MMC5::mapChrAddress(u16 address) const {
    const u32 bankCount1k = std::max<u32>(1, chrROM.size() / 0x400);
    const u16 slot = (address / 0x400) & 0x7;
    const u16 slotOffset = address & 0x3FF;

    u32 bank1k = 0;
    if (chrMode == 0) {
        bank1k = chrRegsA[7] * 8 + slot;
    } else if (chrMode == 1) {
        const u16 reg = slot < 4 ? chrRegsA[3] : chrRegsA[7];
        bank1k = reg * 4 + (slot & 0x03);
    } else if (chrMode == 2) {
        u16 reg = chrRegsA[7];
        if (slot < 2) {
            reg = chrRegsA[1];
        } else if (slot < 4) {
            reg = chrRegsA[3];
        } else if (slot < 6) {
            reg = chrRegsA[5];
        }
        bank1k = reg * 2 + (slot & 0x01);
    } else {
        bank1k = chrRegsA[slot];
    }

    bank1k %= bankCount1k;
    return (bank1k * 0x400 + slotOffset) % chrROM.size();
}

u8 MMC5::ppuread(u16 address) {
    if (address > 0x1FFF || chrROM.empty()) {
        return 0;
    }
    return chrROM[mapChrAddress(address)];
}

void MMC5::ppuwrite(u16 address, u8 data) {
    if (address > 0x1FFF || chrROM.empty()) {
        return;
    }
    chrROM[mapChrAddress(address)] = data;
}

}

