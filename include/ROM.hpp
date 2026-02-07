#pragma once

#include <stdint.h>
#include <stdio.h>

#include <bitset>
#include <string>
#include <vector>

#include "INESBus.hpp"

namespace HunNes {

class Mapper;
// INES is the binary file, much like elf file for UNIX system
struct INESHeader {
    //Header 16 byte
    char nes[4];
    // This field means how many 16KB program exists exist
    u8 prgIn16kb;
    // THis field means how many 8KB CHR-ROM exist
    u8 chrIn8kb;
    u8 flags6;
    u8 flags7;
    u8 prgRamIn8kb;
    u8 flags9;
    u8 flags10;
    char zeros[5];
};

class ROM {
   public:
    std::vector<u8> getChrData() { return chrData; };
    std::vector<u8> getPrgCode() { return prgCode; };
    void open(std::string);
    void printHeader();
    int getMirroring();
    Mapper *getMapper();

   private:
    INESHeader header;
    std::vector<u8> trainer;    // Trainer usually contains mapper register translation and CHR-RAM caching code
    std::vector<u8> prgCode;    // Byte arrays of all opcodes (instructions). Each element is one byte
    std::vector<u8> chrData;
    int mirroring = 0;
    u8 mapperNum = 0;
};

};  
