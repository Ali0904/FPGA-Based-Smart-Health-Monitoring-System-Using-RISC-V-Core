# FPGA Based Smart Health Monitoring System Using RISC-V Core

A complete System-on-Chip (SoC) implementation for real-time health monitoring using a RISC-V soft processor core on an FPGA. The system measures body temperature, heart rate, and blood oxygen saturation (SpO2) using external sensors, processes the data with on-chip DSP algorithms, and displays the results on an OLED screen.

## Table of Contents

- [Features](#features)
- [System Architecture](#system-architecture)
  - [Hardware Architecture](#hardware-architecture)
  - [Software Architecture](#software-architecture)
- [Components](#components)
  - [Hardware Components](#hardware-components)
  - [Software Components](#software-components)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Hardware Setup](#hardware-setup)
  - [Software Setup](#software-setup)
  - [Building the Project](#building-the-project)
  - [Programming the FPGA](#programming-the-fpga)
- [Usage](#usage)
- [Memory Map](#memory-map)
- [Technical Details](#technical-details)
- [Future Enhancements](#future-enhancements)
- [License](#license)

## Features

- **RISC-V Soft Processor**: Implements the PicoRV32 core (RV32IMC instruction set) as a soft IP on FPGA
- **Multi-Sensor Integration**:
  - **LM35 Temperature Sensor**: Analog temperature measurement via XADC (12-bit resolution)
  - **MAX30102 Pulse Oximeter**: Heart rate and SpO2 measurement via I2C
  - **SSD1306 OLED Display**: Real-time visualization of health parameters
- **On-Chip Signal Processing**: Rolling window DSP algorithm for reliable vital sign calculation
- **Custom SoC Design**: Memory-mapped peripherals with Wishbone bus interface
- **Bit-Banged I2C**: Software-controlled I2C implementation for sensor communication
- **Low Resource Utilization**: Optimized for Xilinx 7-series FPGAs (Basys 3 board)

## System Architecture

### Hardware Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      FPGA (Basys 3)                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐     ┌──────────────────────────────────┐  │
│  │  PicoRV32   │     │         Memory Map               │  │
│  │  RISC-V CPU │◄───►│ 0x0000_0000: BRAM (64KB)         │  │
│  │  (RV32IMC)  │     │ 0x4000_0000: I2C Control Reg     │  │
│  └─────────────┘     │ 0x4000_0004: XADC Data Reg       │  │
│                      └──────────────────────────────────┘  │
│                           │                │                │
│                    ┌──────┴──────┐  ┌──────┴──────┐        │
│                    │  I2C Bit-   │  │    XADC     │        │
│                    │  Bang Reg   │  │   Wizard    │        │
│                    └──────┬──────┘  └──────┬──────┘        │
│                           │                │                │
└───────────────────────────┼────────────────┼────────────────┘
                            │                │
                     ┌──────┴──────┐  ┌──────┴──────┐
                     │   I2C Bus   │  │  Analog In  │
                     │  (Pmod JC)  │  │ (Pmod JXADC)│
                     └──────┬──────┘  └──────┬──────┘
                            │                │
              ┌─────────────┼─────┐          │
              │             │     │          │
        ┌─────┴─────┐ ┌────┴────┐ │    ┌────┴────┐
        │ MAX30102  │ │ SSD1306 │ │    │  LM35   │
        │ Pulse     │ │  OLED   │ │    │  Temp   │
        │ Oximeter  │ │ Display │ │    │ Sensor  │
        └───────────┘ └─────────┘ │    └─────────┘
                                  │
                          Connected to
                          JXADC Pmod
```

### Software Architecture

The firmware (`main.c` / `new.c`) follows a bare-metal programming approach:

1. **Hardware Abstraction Layer (HAL)**:
   - Memory-mapped I/O for I2C and XADC registers
   - Bit-banging macros for I2C signal control
   - Delay functions tuned for 100MHz PicoRV32 clock

2. **Sensor Drivers**:
   - `MAX30102`: Initialization, FIFO reading, register configuration
   - `LM35`: ADC value conversion to temperature (°C)
   - `SSD1306`: OLED initialization, cursor control, font rendering

3. **Signal Processing**:
   - Rolling window buffer (100-200 samples) for continuous monitoring
   - DC component calculation for baseline removal
   - AC component extraction for peak detection
   - SpO2 calculation using red/IR ratio formula
   - Heart rate calculation using peak-to-peak interval analysis

4. **Display Driver**:
   - Custom font rendering (small and large digit fonts)
   - Sprite drawing for labels and icons
   - Double-buffered display updates

## Components

### Hardware Components

| Component | Model | Interface | Purpose |
|-----------|-------|-----------|---------|
| FPGA Board | Digilent Basys 3 | - | Main development platform |
| RISC-V Core | PicoRV32 | Internal | Soft processor implementation |
| Temperature Sensor | LM35 | Analog (XADC) | Body temperature measurement |
| Pulse Oximeter | MAX30102 | I2C | Heart rate & SpO2 measurement |
| OLED Display | SSD1306 128x64 | I2C | Real-time data visualization |

### Pin Assignments

| Signal | FPGA Pin | Pmod | Description |
|--------|----------|------|-------------|
| `clk` | W5 | - | 100MHz system clock |
| `reset` | U18 | - | System reset (btnC) |
| `i2c_sda` | K17 | JC[1] | I2C data line |
| `i2c_scl` | M18 | JC[2] | I2C clock line |
| `vauxp6` | J3 | JXADC[1] | XADC positive input |
| `vauxn6` | K3 | JXADC[7] | XADC negative input |

### Software Components

| File | Description |
|------|-------------|
| `main.c` | Main firmware with circular buffer implementation |
| `new.c` | Optimized version with sliding window and improved HR calculation |
| `start.S` | RISC-V assembly startup code (stack setup, jump to main) |
| `custom.ld` | Linker script for memory layout (64KB BRAM) |
| `picorv32.v` | PicoRV32 RISC-V core Verilog implementation |
| `soc_top.v` | SoC top-level module with memory map and peripherals |
| `soc.py` | LiteX/Migen SoC generator script (alternative build method) |
| `basys.xdc` | Xilinx constraints file for Basys 3 board |

## Project Structure

```
FPGA-Based-Smart-Health-Monitoring-System-Using-RISC-V-Core/
├── README.md                    # This file
├── .gitignore                   # Git ignore rules
│
├── RTL/                         # Verilog HDL sources (when organized)
│   ├── picorv32.v               # PicoRV32 RISC-V processor core
│   ├── soc_top.v                # SoC top-level with memory map
│   └── xadc_wiz/               # Xilinx XADC wizard IP
│
├── Firmware/                    # C/Assembly sources (when organized)
│   ├── main.c                   # Main application (circular buffer)
│   ├── new.c                    # Optimized version (sliding window)
│   ├── start.S                  # Assembly startup code
│   └── custom.ld                # Linker script
│
├── Vivado/                      # Vivado project files
│   └── HM/                      # Hardware project directory
│       ├── HM.xpr               # Vivado project file
│       └── HM.srcs/             # Source files and IP
│
├── Constraints/                 # FPGA constraints
│   └── basys.xdc                # Basys 3 pin assignments
│
└── Docs/                        # Documentation
    ├── *.pdf                    # Project report
    └── *.pptx                   # Presentation slides
```

## Getting Started

### Prerequisites

**Hardware Requirements:**
- Digilent Basys 3 FPGA board (Xilinx Artix-7 XC7A35T)
- MAX30102 pulse oximeter module
- LM35 temperature sensor
- SSD1306 128x64 OLED display
- Pmod cables for connections
- JXADC Pmod for analog input

**Software Requirements:**
- Xilinx Vivado Design Suite (2020.2 or later)
- RISC-V GCC Toolchain (`riscv64-unknown-elf-gcc`)
- Python 3.x (for LiteX build method, optional)
- Terminal program for serial communication (optional)

### Hardware Setup

1. **Connect the MAX30102 sensor**:
   - SDA → JC1 (K17)
   - SCL → JC2 (M18)
   - VCC → 3.3V
   - GND → GND

2. **Connect the SSD1306 OLED**:
   - SDA → JC1 (shared with MAX30102)
   - SCL → JC2 (shared with MAX30102)
   - VCC → 3.3V
   - GND → GND

3. **Connect the LM35 sensor**:
   - VOUT → JXADC1 (J3)
   - VCC → 3.3V
   - GND → GND

4. **Connect the FPGA**:
   - Basys 3 board via USB programming cable
   - Ensure Pmod JC and JXADC are accessible

### Software Setup

#### Option 1: Vivado GUI Build

1. Open Vivado and create a new project
2. Add the Verilog source files (`picorv32.v`, `soc_top.v`)
3. Add the XADC wizard IP (configure for 12-bit, 1Msps)
4. Import the constraints file (`basys.xdc`)
5. Run synthesis, implementation, and bitstream generation
6. Generate the memory initialization file (`main.mem`)

#### Option 2: LiteX/Migen Build (Python)

```bash
# Install LiteX and dependencies
pip install migen litex litex-boards

# Generate the SoC
python soc.py

# Build the bitstream
cd build
make
```

### Building the Firmware

```bash
# Set up RISC-V toolchain path
export PATH=/path/to/riscv/bin:$PATH

# Compile the firmware
riscv64-unknown-elf-gcc -nostdlib -nostartfiles \
    -T custom.ld \
    start.S main.c \
    -o firmware.elf

# Generate memory initialization file
objcopy -O verilog firmware.elf firmware.mem

# Alternative: Use objdump for debugging
riscv64-unknown-elf-objdump -d firmware.elf > firmware.dis
```

### Programming the FPGA

1. **Generate bitstream** in Vivado (File → Export → Export Bitstream)
2. **Program the FPGA**:
   - Open Hardware Manager in Vivado
   - Connect to the Basys 3 board
   - Program the device with the generated `.bit` file

3. **Load firmware** (if using external memory):
   - The firmware is pre-loaded into BRAM via `main.mem`
   - Regenerate bitstream after firmware changes

## Usage

1. **Power on** the Basys 3 board
2. **Place finger** on the MAX30102 sensor (steady pressure, no movement)
3. **Observe** the OLED display:
   - Top row: Temperature in °C (e.g., `36.5°C`)
   - Middle row: SpO2 percentage in large digits (e.g., `98`)
   - Bottom row: Heart rate in BPM with heart icon (e.g., `72`)
4. **Monitor** the readings update every ~0.5 seconds
5. **Press btnC** to reset the system if needed

## Memory Map

| Address Range | Size | Description |
|---------------|------|-------------|
| `0x0000_0000` - `0x0000_FFFF` | 64KB | Block RAM (firmware + stack) |
| `0x4000_0000` | 4 bytes | I2C Control Register |
| `0x4000_0004` | 4 bytes | XADC Data Register (LM35) |

### I2C Register Bits

| Bit | Direction | Description |
|-----|-----------|-------------|
| 0 | R/W | SCL output (1=High-Z, 0=Low) |
| 1 | R/W | SDA output (1=High-Z, 0=Low) |
| 2 | R | SDA input (read actual pin state) |

## Technical Details

### Clock Configuration
- **System Clock**: 100MHz (onboard oscillator)
- **PicoRV32 Clock**: 100MHz (direct connection)
- **I2C Speed**: ~100kHz (bit-banged with delay loops)

### ADC Configuration (XADC)
- **Resolution**: 12-bit
- **Input Channel**: Vaux6 (JXADC1)
- **Sampling Rate**: 1Msps (XADC wizard)
- **Voltage Range**: 0-3.3V
- **Temperature Calculation**: `T(°C) = (ADC_value / 4095) × 100`

### Signal Processing Parameters
- **Window Size**: 100-200 samples (1-2 seconds at 100Hz)
- **Sample Rate**: 100Hz (10ms intervals)
- **SpO2 Formula**: `SpO2 = 110 - 25 × (Red_AC × IR_DC) / (IR_AC × Red_DC)`
- **Heart Rate**: Peak-to-peak interval analysis with threshold detection

### Resource Utilization (Estimated)
- **LUTs**: ~1,500 (PicoRV32 + peripherals)
- **FFs**: ~800
- **BRAM**: 32 (64KB memory)
- **DSPs**: 0 (all processing in software)

## Troubleshooting

1. **No display on OLED**:
   - Check I2C connections (SDA/SCL)
   - Verify pull-up resistors are enabled
   - Ensure 3.3V power supply

2. **Invalid sensor readings**:
   - Verify LM35 connection to JXADC1
   - Check MAX30102 I2C address (0x57)
   - Ensure stable finger placement

3. **Build errors**:
   - Verify RISC-V toolchain installation
   - Check Vivado version compatibility
   - Ensure all source files are included

## Future Enhancements

- [ ] Add WiFi/Bluetooth module for wireless data transmission
- [ ] Implement data logging to SD card
- [ ] Add multiple patient monitoring capability
- [ ] Integrate machine learning for anomaly detection
- [ ] Create a mobile app for remote monitoring
- [ ] Add alarm system for critical vital signs
- [ ] Implement low-power mode for battery operation
- [ ] Add ECG monitoring capability

## References

- [PicoRV32 Documentation](https://github.com/cliffordwolf/picorv32)
- [MAX30102 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX30102.pdf)
- [Xilinx XADC User Guide](https://www.xilinx.com/support/documentation/user_guides/ug480_7Series_XADC.pdf)
- [SSD1306 Datasheet](https://www.solomon-systech.com/wp-content/uploads/2011/12/SSD1306.pdf)

## License

This project is developed for educational purposes as part of a Final Year Project (FYP). Please contact the author for any usage beyond academic work.

**Author**: Ali Haider  
**GitHub**: [@Ali0904](https://github.com/Ali0904)