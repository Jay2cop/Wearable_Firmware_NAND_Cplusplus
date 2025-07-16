#include "W25N01GW_HAL.hpp"
#include <string.h>

extern "C" {
#include "BlueNRG1_spi.h"
#include "BlueNRG1_dma.h"
#include "BlueNRG1_sysCtrl.h"
#include "SDK_EVAL_Config.h"
#include "SDK_EVAL_SPI.h"
}

#define NAND_SPI_BUFF_SIZE 2112

volatile uint8_t spi_buffer_tx_NAND[NAND_SPI_BUFF_SIZE];
volatile uint8_t spi_buffer_rx_NAND[NAND_SPI_BUFF_SIZE];

/**
 * @brief Initializes the hardware abstraction layer.
 * @param baudrate 	The SPI communication speed.
 * @return NandError::Success on success, an error code otherwise.
 */
NandError W25N01GW_HAL::init(uint32_t baudrate) {
    SPI_InitType SPI_InitStructure;
    GPIO_InitType GPIO_InitStructure;
    DMA_InitType DMA_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_SPI |
    						   CLOCK_PERIPH_DMA, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = SDK_EVAL_SPI_PERIPH_OUT_PIN |
    							  SDK_EVAL_SPI_PERIPH_SCLK_PIN |
								  SDK_EVAL_SPI_PERIPH_IN_PIN;
    GPIO_InitStructure.GPIO_Mode = Serial0_Mode;
    GPIO_InitStructure.GPIO_Pull = ENABLE;
    GPIO_InitStructure.GPIO_HighPwr = DISABLE;
    GPIO_Init(&GPIO_InitStructure);

    // Assigning DIO11 for cs line
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Output;
    GPIO_Init(&GPIO_InitStructure);

    // Setting cs line initially high
    csHigh();

    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_BaudRate = baudrate;
    SPI_Init(&SPI_InitStructure);

    SPI_SetDummyCharacter(0xFF);
    SPI_ClearTXFIFO();
    SPI_ClearRXFIFO();
    SPI_SetMasterCommunicationMode(SPI_FULL_DUPLEX_MODE);

    DMA_InitStructure.DMA_PeripheralBaseAddr = SPI_DR_BASE_ADDR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)spi_buffer_tx_NAND;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)NAND_SPI_BUFF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CH_SPI_TX, &DMA_InitStructure);

    DMA_InitStructure.DMA_PeripheralBaseAddr = SPI_DR_BASE_ADDR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)spi_buffer_rx_NAND;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)NAND_SPI_BUFF_SIZE;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA_CH_SPI_RX, &DMA_InitStructure);

    DMA_FlagConfig(DMA_CH_SPI_RX, DMA_FLAG_TC, ENABLE);
    SPI_DMACmd(SPI_DMAReq_Tx | SPI_DMAReq_Rx, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = HIGH_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SPI_Cmd(ENABLE);

    return NandError::Success;
}

/**
 * @brief Sets the chip select (CS) pin to low.
 */
void W25N01GW_HAL::csLow() {
    GPIO_ResetBits(GPIO_Pin_11);
}

/**
 * @brief Sets the chip select (CS) pin to high.
 */
void W25N01GW_HAL::csHigh() {
    GPIO_SetBits(GPIO_Pin_11);
}

/**
 * @brief Performs a full-duplex SPI transaction.
 * @param tx_buf 	Pointer to the buffer of data to transmit.
 * @param rx_buf 	Pointer to the buffer to store received data.
 * @param len    	The number of bytes to transmit and receive.
 * @return NandError::Success on success, an error code otherwise.
 */
NandError W25N01GW_HAL::transceive(const uint8_t* tx_buf, uint8_t* rx_buf,
							 uint32_t len)
{
    if (tx_buf) {
        memcpy((void*)spi_buffer_tx_NAND, tx_buf, len);
    } else {
        // If no tx_buf, send dummy bytes
        memset((void*)spi_buffer_tx_NAND, 0xFF, len);
    }

    spi_eot = RESET;
    DMA_CH_SPI_TX->CNDTR = len;
    DMA_CH_SPI_RX->CNDTR = len;
    DMA_CH_SPI_RX->CCR_b.EN = SET;
    DMA_CH_SPI_TX->CCR_b.EN = SET;
    while (spi_eot == RESET);

    if (rx_buf)
    {
        memcpy(rx_buf, (const void*)spi_buffer_rx_NAND, len);
    }

    return NandError::Success;
}

/**
 * @brief Delays execution for a specified number of milliseconds.
 * @param ms 		The delay time in milliseconds.
 */
void W25N01GW_HAL::delay(uint32_t ms) {
    volatile uint32_t count;
    uint32_t delay = ms * 1000;
    for (count = 0; count < delay; ++count) {
        __asm__ volatile("nop");
    }
}
