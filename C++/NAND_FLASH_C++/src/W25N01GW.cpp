#include "W25N01GW.hpp"
#include "W25N01GW_HAL.hpp"
#include <stdio.h>
#include <string.h>

W25N01GW::W25N01GW() {}

/**
 * @brief Resets the flash memory.
 */
NandError W25N01GW::reset()
{
    waitForReady();

    uint8_t enable_reset[] = {static_cast <uint8_t>(NandCommand::EnableReset)};
    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(enable_reset, nullptr, 1)
    							 != NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();
    W25N01GW_HAL::delay(1);

    uint8_t reset_device[] = {0x99};
    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(reset_device, nullptr, 1) !=
    							 NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();
    W25N01GW_HAL::delay(2);

    return waitForReady();
}

/**
 * @brief Reads the JEDEC ID of the flash memory.
 * @param idBuffer A buffer to store the ID.
 */
NandError W25N01GW::readJedecID(char* idBuffer)
{
    uint8_t tx_buf[4] = {static_cast<uint8_t>(NandCommand::JedecID),
    					 0xFF, 0xFF, 0xFF};
    uint8_t rx_buf[4];

    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(tx_buf, rx_buf, 4) != NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();

    sprintf(idBuffer, "%02X %02X %02X", rx_buf[1], rx_buf[2], rx_buf[3]);
    return NandError::Success;
}

/**
 * @brief Enables writing to the flash memory.
 * @return NandError::Success on success, an error code otherwise.
 */
NandError W25N01GW::writeEnable()
{
    if (waitForReady() != NandError::Success)
    {
    	return NandError::Timeout;
    }

    uint8_t cmd = static_cast<uint8_t>(NandCommand::WriteEnable);
    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(&cmd, nullptr, 1) != NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();

    if (waitForReady() != NandError::Success)
    {
    	return NandError::Timeout;
    }

    uint8_t status = readStatusRegister();

    if (status & 0x02)
    {
        return NandError::Success;
    }
    else
    {
        return NandError::WriteEnableFailed;
    }
}

/**
 * @brief Erases a sector of the flash memory.
 * @param address The address of the sector to erase.
 * @return NandError::Success on success, an error code otherwise.
 */
NandError W25N01GW::sectorErase(uint32_t address)
{
    if (writeEnable() != NandError::Success)
    {
    	return NandError::WriteEnableFailed;
    }

    uint8_t tx_buf[4] = {
        static_cast<uint8_t>(NandCommand::SectorErase),
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)address
    };

    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(tx_buf, nullptr, 4) != NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();

    return waitForReady();
}

/**
 * @brief Programs a page of the flash memory.
 * @param address The address of the page to program.
 * @param data 	  A pointer to the data to write.
 * @param len 	  The length of the data to write.
 * @return NandError::Success on success, an error code otherwise.
 */
NandError W25N01GW::pageProgram(uint32_t address, const char* data,
								uint32_t len)
{
    if (writeEnable() != NandError::Success)
    {
    	return NandError::WriteEnableFailed;
    }

    uint8_t tx_buf[NandProperties::PageSize + 4];
    tx_buf[0] = static_cast<uint8_t>(NandCommand::PageProgram);
    tx_buf[1] = (uint8_t)(address >> 16);
    tx_buf[2] = (uint8_t)(address >> 8);
    tx_buf[3] = (uint8_t)address;
    memcpy(&tx_buf[4], data, len);

    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(tx_buf, nullptr, len + 4) !=
    							 NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();

    return waitForReady();
}

/**
 * @brief Reads data from the flash memory.
 * @param address 	The address to read from.
 * @param buffer  	A buffer to store the read data.
 * @param len     	The number of bytes to read.
 * @return NandError::Success on success, an error code otherwise.
 */
NandError W25N01GW::read(uint32_t address, char* buffer, uint32_t len)
{
    if (waitForReady() != NandError::Success)
    {
    	return NandError::Timeout;
    }

    uint8_t tx_buf[NandProperties::PageSize + 4];
    tx_buf[0] = static_cast<uint8_t>(NandCommand::ReadData);
    tx_buf[1] = (uint8_t)(address >> 16);
    tx_buf[2] = (uint8_t)(address >> 8);
    tx_buf[3] = (uint8_t)address;

    uint8_t rx_buf[NandProperties::PageSize + 4];

    W25N01GW_HAL::csLow();
    if (W25N01GW_HAL::transceive(tx_buf, rx_buf, len + 4) != NandError::Success)
    {
        W25N01GW_HAL::csHigh();
        return NandError::SpiError;
    }
    W25N01GW_HAL::csHigh();

    memcpy(buffer, &rx_buf[4], len);

    return NandError::Success;
}

/**
 * @brief Waits for the flash memory to become ready.
 * @param timeout_ms The timeout in milliseconds.
 * @return NandError::Success on success, NandError::Timeout on timeout.
 */
NandError W25N01GW::waitForReady(uint32_t timeout_ms)
{
    uint32_t start = 0;
    while (readStatusRegister() & static_cast<uint8_t>
    	   (NandCommand::WriteStatusReg))
    {
        W25N01GW_HAL::delay(1);
    }
    return NandError::Success;
}

/**
 * @brief Reads the status register of the flash memory.
 * @return The value of the status register.
 */
uint8_t W25N01GW::readStatusRegister()
{
    uint8_t tx_buf[2] = {static_cast<uint8_t>(NandCommand::ReadStatusReg),
    					 0xFF};
    uint8_t rx_buf[2];

    W25N01GW_HAL::csLow();
    W25N01GW_HAL::transceive(tx_buf, rx_buf, 2);
    W25N01GW_HAL::csHigh();

    return rx_buf[1];
}
