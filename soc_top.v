`timescale 1ns / 1ps

module soc_top (
    input  wire clk,
    input  wire reset,

    inout  wire i2c_sda,
    inout  wire i2c_scl,

    input  wire vauxp6,
    input  wire vauxn6
);

    // ---------------- CPU ----------------
    wire mem_valid, mem_instr;
    reg  mem_ready;
    wire [31:0] mem_addr, mem_wdata;
    reg  [31:0] mem_rdata;
    wire [3:0] mem_wstrb;

    picorv32 cpu (
        .clk(clk),
        .resetn(~reset),
        .mem_valid(mem_valid),
        .mem_instr(mem_instr),
        .mem_ready(mem_ready),
        .mem_addr(mem_addr),
        .mem_wdata(mem_wdata),
        .mem_wstrb(mem_wstrb),
        .mem_rdata(mem_rdata)
    );

    // ---------------- BRAM ----------------
    reg [31:0] memory [0:16383];
    initial $readmemh("main.mem", memory);

    // ---------------- XADC WIZARD ----------------
    wire [15:0] adc_raw;
    wire adc_ready;
    wire eoc_signal; // End of Conversion signal

    xadc_wiz xadc_inst (
        .dclk_in(clk),
        .reset_in(reset),
        .vauxp6(vauxp6),
        .vauxn6(vauxn6),
        .daddr_in(7'h16),     
        .den_in(eoc_signal),  
        .dwe_in(1'b0),        
        .di_in(16'h0),        
        .do_out(adc_raw),
        .drdy_out(adc_ready),
        .eoc_out(eoc_signal)  
    );

    // ---> THIS IS THE MISSING LINE <---
    wire [11:0] lm35_raw = adc_raw[15:4];

    // ---------------- I2C BIT-BANG REGISTER ----------------
    reg [1:0] i2c_reg;
    wire i2c_sda_in = i2c_sda;

    // Open drain: 1 = High-Z (pulled up), 0 = GND
    assign i2c_scl = i2c_reg[0] ? 1'bz : 1'b0;
    assign i2c_sda = i2c_reg[1] ? 1'bz : 1'b0;

    // ---------------- MEMORY MAP ----------------
    always @(posedge clk) begin
        mem_ready <= 0;

        if (mem_valid && !mem_ready) begin
            mem_ready <= 1; // 1-cycle latency

            // RAM (0x0000_0000 to 0x0000_FFFF)
            if (mem_addr < 32'h0001_0000) begin
                // CPU Write Logic (CRITICAL for the C stack to work)
                if (mem_wstrb[0]) memory[mem_addr[15:2]][7:0]   <= mem_wdata[7:0];
                if (mem_wstrb[1]) memory[mem_addr[15:2]][15:8]  <= mem_wdata[15:8];
                if (mem_wstrb[2]) memory[mem_addr[15:2]][23:16] <= mem_wdata[23:16];
                if (mem_wstrb[3]) memory[mem_addr[15:2]][31:24] <= mem_wdata[31:24];
                mem_rdata <= memory[mem_addr[15:2]];
            end

            // I2C Register (0x4000_0000)
            else if (mem_addr == 32'h4000_0000) begin
                if (|mem_wstrb) i2c_reg <= mem_wdata[1:0]; // CPU writes
                mem_rdata <= {29'b0, i2c_sda_in, i2c_reg}; // CPU reads
            end

            // LM35 XADC Register (0x4000_0004)
            else if (mem_addr == 32'h4000_0004) begin
                mem_rdata <= {20'b0, lm35_raw}; // CPU reads only
            end
        end
    end

endmodule