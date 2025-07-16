#include "W25N01GW.hpp"
#include "W25N01GW_HAL.hpp"
#include <stdio.h>
#include <string.h>
#include "BlueNRG1_conf.h"
#include "SDK_EVAL_Config.h"

// Helper function to print errors
void printNandError(NandError err)
{
    switch (err) {
        case NandError::Success:
            printf("Operation successful.\r\n");
            break;
        case NandError::Busy:
            printf("Error: NAND is busy.\r\n");
            break;
        case NandError::WriteEnableFailed:
            printf("Error: Failed to enable write.\r\n");
            break;
        case NandError::EraseFailed:
            printf("Error: Erase failed.\r\n");
            break;
        case NandError::ProgrammingFailed:
            printf("Error: Programming failed.\r\n");
            break;
        case NandError::ReadFailed:
            printf("Error: Read failed.\r\n");
            break;
        case NandError::Timeout:
            printf("Error: Operation timed out.\r\n");
            break;
        default:
            printf("An unknown error occurred.\r\n");
    }
}

int main(void)
{
    char data[] = "Write into NAND TEST";
    char readBuffer[128] = {0};
    uint32_t address = 0x000000;

    SystemInit();
    SdkEvalIdentification();
    SdkEvalComUartInit(UART_BAUDRATE);

    printf("--- Flash C++ Driver Write/Read ---\r\n");

    if (W25N01GW_HAL::init(8000000) != NandError::Success)
    {
        printf("Error initialising Communication.\r\n");
        while(1);
    }

    SysTick_Config(SYST_CLOCK / 1000);

    W25N01GW flash;
    flash.reset();
    printf("Flash has been reset.\r\n");

    char idBuffer[20];
    flash.readJedecID(idBuffer);
    printf("FLASH ID: %s\r\n", idBuffer);

    printf("Erasing sector at address 0x%06lX...\r\n", address);
    NandError err = flash.sectorErase(address);
    if (err != NandError::Success)
    {
        printNandError(err);
        while(1);
    }

    printf("Data to be written: '%s'\r\n", data);

    printf("Programming page at address 0x%06lX...\r\n", address);
    err = flash.pageProgram(address, data, sizeof(data) - 1);
    if (err != NandError::Success)
    {
        printNandError(err);
        while(1);
    }

    printf("Reading data back...\r\n");
    err = flash.read(address, readBuffer, sizeof(data) - 1);
    if (err != NandError::Success)
    {
        printNandError(err);
        while(1);
    }

    // Adding the null-terminate the string
    readBuffer[sizeof(data) - 1] = '\0';
    printf("Read back data: '%s'\r\n", readBuffer);

    if (strcmp(data, readBuffer) == 0) {
        printf("Success! Data matches.\r\n");
    } else {
        printf("Error! Data mismatch.\r\n");
    }

    printf("--- Finished ---\r\n");

    while(1) {}
    return 0;
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
    printf("Assertion Failed: file %s, line %lu\r\n", file, line);
    while (1) {
    }
}
#endif
