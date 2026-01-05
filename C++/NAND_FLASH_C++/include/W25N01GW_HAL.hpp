#ifndef W25N01GW_HAL_HPP
#define W25N01GW_HAL_HPP

#include <stdint.h>

enum class NandError {
    Success,
    Busy,
    WriteEnableFailed,
    EraseFailed,
    ProgrammingFailed,
    ReadFailed,
    Timeout,
    SpiError
};

class W25N01GW_HAL {
public:
    static NandError init(uint32_t baudrate);
    static void csLow();
    static void csHigh();
    static NandError transceive(const uint8_t* tx_buf, uint8_t* rx_buf, uint32_t len);
    static void delay(uint32_t ms);
};

#endif // W25N01GW_HAL_HPP
