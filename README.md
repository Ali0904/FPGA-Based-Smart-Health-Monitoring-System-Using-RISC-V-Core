# FPGA Based Smart Health Monitoring System Using RISC-V Core

This project implements a smart health monitoring system on an FPGA using a RISC-V processor core. The system is designed to monitor health parameters and provide real-time feedback.

## Project Structure

- `main.c`, `new.c`: C source code for the RISC-V firmware.
- `start.S`: Assembly startup code.
- `picorv32.v`, `soc_top.v`: Verilog modules for the RISC-V core and SoC top level.
- `soc.py`: Python script for SoC configuration (likely using LiteX or similar).
- `custom.ld`: Custom linker script for firmware compilation.
- `HM/`: Vivado project directory containing hardware design files.

## Getting Started

1. **Prerequisites**: Vivado Design Suite (for FPGA synthesis), RISC-V toolchain (for firmware compilation).
2. **Hardware**: Open `HM/HM.xpr` in Vivado to view or modify the hardware design.
3. **Firmware**: Use the RISC-V toolchain to compile the C code:
   ```
   riscv64-unknown-elf-gcc -nostdlib -nostartfiles -T custom.ld start.S main.c -o firmware.elf
   ```
4. **Programming**: Generate the bitstream in Vivado and program the FPGA.

## License

This project is for educational purposes. Please contact the author for any usage beyond academic work.