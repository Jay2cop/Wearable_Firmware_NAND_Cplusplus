#ifndef W25N01GW_HPP
#define W25N01GW_HPP

#include "W25N01GW_HAL.hpp"
#include <stdint.h>

namespace NandProperties {
    constexpr uint32_t MaxPages = 65535;
    constexpr uint16_t PageSize = 2112;
    constexpr uint8_t PagesPerBlock = 64;
}

enum class NandCommand : uint8_t {
    Reset               = 0xFF,
    JedecID             = 0x9F,
    ReadStatusReg       = 0x05,
    WriteStatusReg      = 0x01,
    WriteEnable         = 0x06,
    WriteDisable        = 0x04,
    SectorErase         = 0x20,
    PageProgram         = 0x02,
    ReadData            = 0x03,
	EnableReset			= 0x66,
};



class W25N01GW {
public:
    W25N01GW();

    NandError reset();
    NandError readJedecID(char* idBuffer);
    NandError writeEnable();
    NandError sectorErase(uint32_t address);
    NandError pageProgram(uint32_t address, const char* data, uint32_t len);
    NandError read(uint32_t address, char* buffer, uint32_t len);

private:
    NandError waitForReady(uint32_t timeout_ms = 100);
    uint8_t readStatusRegister();
};

#endif // W25N01GW_HPP
