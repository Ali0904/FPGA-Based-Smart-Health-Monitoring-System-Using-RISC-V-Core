## Basys 3 Constraints File (.xdc)
## Smart Health Monitoring System Using RISC-V Core

## -----------------------------------------------------------------------------
## 1. Clock Signal (100 MHz)
## -----------------------------------------------------------------------------
# Map the 'clk' port to pin W5 (the onboard 100MHz oscillator)
set_property PACKAGE_PIN W5 [get_ports clk]
set_property IOSTANDARD LVCMOS33 [get_ports clk]
# Define the timing constraint for the synthesis tool
create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports clk]

## -----------------------------------------------------------------------------
## 2. System Reset
## -----------------------------------------------------------------------------
# Map the 'reset' port to the central push button (btnC)
set_property PACKAGE_PIN U18 [get_ports reset]
set_property IOSTANDARD LVCMOS33 [get_ports reset]

## -----------------------------------------------------------------------------
## 3. I2C Bus (Pmod JC) - MAX30102 & SSD1306 OLED
## -----------------------------------------------------------------------------
# Map 'i2c_sda' to JC Pin 1
set_property PACKAGE_PIN K17 [get_ports i2c_sda]
set_property IOSTANDARD LVCMOS33 [get_ports i2c_sda]
# Enable internal weak pull-up (I2C requires the line to default HIGH)
set_property PULLUP true [get_ports i2c_sda]

# Map 'i2c_scl' to JC Pin 2
set_property PACKAGE_PIN M18 [get_ports i2c_scl]
set_property IOSTANDARD LVCMOS33 [get_ports i2c_scl]
set_property PULLUP true [get_ports i2c_scl]

## -----------------------------------------------------------------------------
## 4. Analog Inputs (Pmod JXADC) - LM35 Temperature Sensor
## -----------------------------------------------------------------------------
# Map 'vauxp6' to JXADC Pin 1 (XA1_P)
set_property PACKAGE_PIN J3 [get_ports vauxp6]
set_property IOSTANDARD LVCMOS33 [get_ports vauxp6]

# Map 'vauxn6' to JXADC Pin 7 (XA1_N - Ground reference for the analog pair)
set_property PACKAGE_PIN K3 [get_ports vauxn6]
set_property IOSTANDARD LVCMOS33 [get_ports vauxn6]

## Configuration properties for proper bitstream generation
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_MODE SPIx4 [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]