#pragma once
#include <stdint.h>

#include <vector>

#include "../Common/Typedefs.hpp"

namespace HunNes {


// Mapper is like a virtual memory between the CPU/PPU Ram and ROM
// It decides which block of memory in the ROM (could be 100 kb magnitude) will be mapped to the RAM (really small, 4kb)
// and it decides how instructions (opcodes) can be read from/written to in the ROM 
class Mapper {
   public:
    Mapper(std::vector<u8> &prgCode, std::vector<u8> &chrROM, int mirroring) : prgCode(prgCode),
                                                                               chrROM(chrROM),
                                                                               mirroring(mirroring) {}
    
    // Virtual functions, should be overwritten by child classes
    virtual ~Mapper() {}
    virtual u8 read(u16 address) = 0;
    virtual void write(u16 address, u8 data) = 0;
    virtual u8 ppuread(u16 address);
    virtual void ppuwrite(u16 address, u8 data);
    int getMirroring() { return mirroring; }

   protected:
    std::vector<u8> prgCode;    // The program ROM where the CPU instructions live
    std::vector<u8> chrROM;     // The graphic ROM where PPU gets pattern data
    int mirroring;              // How many table mirroing is configured
};

};  
