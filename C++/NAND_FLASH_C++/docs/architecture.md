# Architecture

```
App (Nand_Flash_Main.cpp)
  -> W25N01GW (command driver)
      -> W25N01GW_HAL (SPI/DMA + CS + delay)
          -> BlueNRG-1 SDK peripherals
```

## Command flow
- `readJedecID()`
  1. HAL drives CS low and `transceive` sends the JEDEC ID opcode with three dummy bytes.
  2. W25N01GW parses the returned manufacturer/device bytes and formats them for UART output.
  3. CS is raised immediately after the SPI burst, making the operation non-destructive.
- `sectorErase(address)`
  1. `writeEnable()` asserts the write-enable latch via opcode `0x06` and confirms it using the status register.
  2. HAL issues the sector-erase opcode with the 24-bit address while CS stays low.
  3. `waitForReady()` polls the status register until the busy bit clears before returning to the caller.
- `pageProgram(address, data, len)`
  1. `writeEnable()` prepares the device for program.
  2. Driver builds an in-memory frame `[PROGRAM_CMD | addr[23:0] | payload bytes]`.
  3. HAL streams the buffer over SPI, then `waitForReady()` spins on the busy bit to ensure the page commit completes.

## DMA / transceive behavior
- Buffers: `spi_buffer_tx_NAND` and `spi_buffer_rx_NAND` live inside `W25N01GW_HAL.cpp` as static volatile arrays sized to one NAND page (2112 bytes). Drivers copy user data into/out of these buffers before/after DMA transfers, so the application retains ownership of its source/destination pointers.
- Synchronization: `transceive()` launches DMA for TX/RX simultaneously, waits on the `spi_eot` flag driven by the BlueNRG DMA IRQ, and only returns once the burst is complete (synchronous polling).
- Chip-select: `csLow()` and `csHigh()` toggle GPIO11, wrapped around every SPI transaction in the command driver. This guarantees DMA transfers begin and end with the proper CS framing. Delays are implemented with a simple busy-wait loop for short guard times between commands.
