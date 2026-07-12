from migen import *
from litex_boards.platforms import digilent_basys3
from litex.soc.cores.cpu import PicoRV32
from litex.soc.integration.soc_core import SoCConfig, SoCData
from litex.soc.integration.builder import Builder
from litex.soc.interconnect import wishbone

# --- Custom SoC Class ---

class HealthMonitorSoC(Module):
    def __init__(self, platform):
        # 100MHz Clock and Reset from Constraints 
        self.clock_domains.cd_sys = ClockDomain()
        self.specials += [
            Instance("IBUF", i_I=platform.request("clk"), o_O=self.cd_sys.clk),
            Instance("BUFG", i_I=self.cd_sys.clk, o_O=self.cd_sys.clk),
        ]
        self.reset = platform.request("reset")

        # CPU: PicoRV32 [cite: 2, 3]
        self.submodules.cpu = PicoRV32(platform, variant="minimal")
        
        # BRAM: 64KB (mapped at 0x0000_0000) [cite: 4, 14]
        self.submodules.ram = wishbone.SRAM(64*1024, read_only=False)
        self.cpu.add_memory_peripheral("ram", 0x00000000, 0x10000)

        # ---------------------------------------------------------------------
        # I2C Bit-Bang Peripheral (Mapped at 0x4000_0000) [cite: 16, 17]
        # ---------------------------------------------------------------------
        i2c_pads = platform.request("i2c") # SDA: K17, SCL: M18 [cite: 21]
        self.i2c_reg = Signal(2)
        
        # Drive Open-Drain Logic 
        self.comb += [
            i2c_pads.scl.oe.eq(~self.i2c_reg[0]),
            i2c_pads.scl.o.eq(0),
            i2c_pads.sda.oe.eq(~self.i2c_reg[1]),
            i2c_pads.sda.o.eq(0),
        ]
        
        # Register Interface
        i2c_bus = wishbone.Interface()
        self.cpu.add_memory_peripheral("i2c_reg", 0x40000000, 0x4)
        
        self.sync += [
            If(i2c_bus.cyc & i2c_bus.stb & i2c_bus.we,
               self.i2c_reg.eq(i2c_bus.dat_w[0:2])
            ),
            i2c_bus.dat_r.eq(Cat(self.i2c_reg, i2c_pads.sda.i, Replicate(0, 29))) [cite: 17]
        ]

        # ---------------------------------------------------------------------
        # XADC Peripheral (Mapped at 0x4000_0004) [cite: 18, 19]
        # ---------------------------------------------------------------------
        vaux = platform.request("vauxp6") # Pins J3, K3 [cite: 21]
        adc_raw = Signal(16)
        
        self.specials += Instance("XADC",
            p_INIT_48=0x0700, # Sequence registers
            p_INIT_49=0x0040, # Enable Vaux6
            i_DCLK=self.cd_sys.clk,
            i_RESET=self.reset,
            i_VAUXP6=vaux.p,
            i_VAUXN6=vaux.n,
            o_DO=adc_raw,
            # Simple continuous sampling mode for LM35
        )

        adc_bus = wishbone.Interface()
        self.cpu.add_memory_peripheral("adc_reg", 0x40000004, 0x4)
        self.comb += adc_bus.dat_r.eq(adc_raw[4:16]) # Bits 15:4 

# --- Platform Definitions (Basys 3) ---

def main():
    platform = digilent_basys3.Platform()
    
    # Add I2C Pins to Platform
    platform.add_extension([
        ("i2c", 0,
            Subsignal("sda", Pins("K17"), IOStandard("LVCMOS33")),
            Subsignal("scl", Pins("M18"), IOStandard("LVCMOS33")),
        ),
        ("vauxp6", 0, Pins("J3"), IOStandard("LVCMOS33")),
        ("vauxn6", 0, Pins("K3"), IOStandard("LVCMOS33")),
    ])

    soc = HealthMonitorSoC(platform)
    builder = Builder(soc, output_dir="build", csr_csv="csr.csv")
    builder.build(build_name="health_monitor")

if __name__ == "__main__":
    main()