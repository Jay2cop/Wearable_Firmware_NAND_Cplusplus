# BlueNRG-1 NAND Flash Driver (C++)

BlueNRG-1 + Winbond W25N01GW NAND flash driver refactored from C to C++ for a wearables data logger. The firmware layers a class-based driver on top of the BlueNRG SPI + DMA HAL and has been validated on hardware by flashing the generated .bn and exercising erase/program/read paths.

## Hardware / stack
- BlueNRG-1 reference board (SDK-EVAL) with UART logging enabled
- Winbond W25N01GW 1‑Gbit NAND flash
- SPI bus with DMA transfers and UART output for telemetry

## Refactor focus
- Replaced C routines with a C++ class-based driver interface (init, read, program, erase)
- Introduced a HAL wrapper so platform-specific SPI/DMA setup stays isolated from NAND command sequencing
- Encapsulated TX/RX buffers and reduced globals to clarify ownership and provide a safer API surface

## Build & flash
This repository is meant to be integrated into the ST BlueNRG-1 SDK workspace (Keil µVision, IAR, or the SDK makefiles). Use the SDK’s project generator and then drop these sources in.

Integration steps:
1. Copy the contents of `src/` into your SDK project’s source directory and `include/` into its include path.
2. Add `include/` to the compiler’s include search list.
3. Ensure the SPI pins, DMA channels, and chip-select GPIO in `W25N01GW_HAL.cpp` match your board wiring.
4. Add `Nand_Flash_Main.cpp` as an application entry point to exercise the driver or integrate the class into your existing firmware.
5. Build the project inside the BlueNRG IDE (Keil/IAR) and flash the produced `.bn` using ST’s flashing tool.

`Nand_Flash_Main.cpp` serves as a demo app that runs a reset → JEDEC ID read → sector erase → page program → readback compare cycle while printing results over UART.

## Smoke test procedure
1. Reset the flash (`flash.reset()`).
2. Read and print the JEDEC ID to ensure SPI comms are healthy.
3. Erase the target sector or block.
4. Program a test page using `pageProgram`.
5. Read back using `read` and compare against the original buffer; UART output should report “Success! Data matches.”

## Project structure
- `Nand_Flash_Main.cpp` – example app and UART test harness
- `src/W25N01GW.cpp`, `src/W25N01GW_HAL.cpp` – NAND command logic and HAL bindings
- `include/W25N01GW.hpp`, `include/W25N01GW_HAL.hpp` – public interfaces for the driver and HAL
