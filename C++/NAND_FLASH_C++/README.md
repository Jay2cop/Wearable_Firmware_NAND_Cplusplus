# BlueNRG-1 NAND Flash Driver (C++)

## Overview
BlueNRG-1 + Winbond W25N01GW NAND flash driver refactored from C to C++. The firmware isolates SPI + DMA hardware control behind a HAL while presenting a class-based NAND API. It has been validated on hardware by flashing the generated .bn image and verifying erase/program/read cycles through UART logs.

## Hardware / stack
- BlueNRG-1 SDK-EVAL board (or equivalent BlueNRG-1 MCU design)
- Winbond W25N01GW 1 Gbit NAND flash
- SPI bus with DMA and UART logging for telemetry

## What I refactored
- Converted the C routines into `W25N01GW`/`W25N01GW_HAL` classes for clearer responsibilities
- Introduced an explicit HAL boundary so SPI/DMA initialization stays separate from NAND command logic
- Clarified buffer ownership by keeping TX/RX pages inside the HAL and copying user data at the edges
- Reduced global state and exposed a safer public API for init/read/program/erase flows

## How to build / integrate
This repository is meant to live inside an ST BlueNRG-1 SDK workspace (Keil µVision, IAR, or SDK-generated projects). It is not a standalone build system.
- Copy the files from `src/` into your SDK project’s source folder and the headers from `include/` into the project’s include directory.
- Add `include/` to the compiler’s include search paths.
- Ensure the SPI pins, DMA channels, and chip-select GPIO defined in `W25N01GW_HAL.cpp` match your board wiring.
- Build the BlueNRG project with your IDE/toolchain and flash the resulting `.bn` firmware onto the target.

## Smoke test
The `Nand_Flash_Main.cpp` sample performs the following sequence over UART:
- Reset the NAND (`flash.reset()`).
- Read the JEDEC ID and print it to confirm SPI communication.
- Erase the selected sector/block.
- Program a page with sample data.
- Read back the programmed page and compare it, printing success/fail so you can confirm on the UART console.

## Project structure
- `Nand_Flash_Main.cpp` – demo application and UART-driven smoke test
- `src/` – driver and HAL implementations (`W25N01GW.cpp`, `W25N01GW_HAL.cpp`)
- `include/` – public headers for the driver (`W25N01GW.hpp`, `W25N01GW_HAL.hpp`)
- `docs/` – architecture notes and other documentation
