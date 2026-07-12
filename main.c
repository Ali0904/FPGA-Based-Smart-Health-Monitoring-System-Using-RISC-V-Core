#include <stdint.h>

// --- Hardware Memory Pointers ---
#define REG_I2C  (*((volatile uint32_t*) 0x40000000))
#define REG_XADC (*((volatile uint32_t*) 0x40000004))

// --- I2C Bit Manipulation Macros ---
#define SCL_HIGH() REG_I2C |= 0x01
#define SCL_LOW()  REG_I2C &= ~0x01
#define SDA_HIGH() REG_I2C |= 0x02
#define SDA_LOW()  REG_I2C &= ~0x02
#define SDA_READ() ((REG_I2C >> 2) & 0x01)

// --- Basic Delays ---
void delay_us(uint32_t us) {
    for(volatile uint32_t i = 0; i < (us * 10); i++); 
}

// --- I2C Core Functions ---
void i2c_start() {
    SDA_HIGH(); SCL_HIGH(); delay_us(5);
    SDA_LOW(); delay_us(5); SCL_LOW(); delay_us(5);
}

void i2c_stop() {
    SDA_LOW(); delay_us(5);
    SCL_HIGH(); delay_us(5); SDA_HIGH(); delay_us(5);
}

uint8_t i2c_write(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        if (data & (1 << i)) SDA_HIGH();
        else SDA_LOW();
        delay_us(2); SCL_HIGH(); delay_us(5); SCL_LOW(); delay_us(2);
    }
    SDA_HIGH(); delay_us(2); 
    SCL_HIGH(); delay_us(2);
    uint8_t ack = SDA_READ();
    SCL_LOW(); delay_us(5);
    return ack;
}

uint8_t i2c_read(uint8_t send_ack) {
    uint8_t data = 0;
    SDA_HIGH(); 
    for(int i = 7; i >= 0; i--) {
        delay_us(2); SCL_HIGH(); delay_us(2);
        if(SDA_READ()) data |= (1 << i); 
        SCL_LOW(); delay_us(2);
    }
    if(send_ack) SDA_LOW(); else SDA_HIGH();
    delay_us(2); SCL_HIGH(); delay_us(5); SCL_LOW(); delay_us(2); SDA_HIGH(); 
    return data;
}

// --- MAX30102 Biometric Sensor ---
#define MAX_ADDR 0x57

void max30102_write_reg(uint8_t reg, uint8_t val) {
    i2c_start(); i2c_write(MAX_ADDR << 1); i2c_write(reg); i2c_write(val); i2c_stop();
}

void max30102_init() {
    max30102_write_reg(0x09, 0x40); // Reset
    delay_us(10000);
    max30102_write_reg(0x09, 0x03); // SpO2 mode
    max30102_write_reg(0x0A, 0x27); // 100Hz sample rate (effective)
    max30102_write_reg(0x0C, 0x24); // Red amplitude
    max30102_write_reg(0x0D, 0x24); // IR amplitude
}

void max30102_read_fifo(uint32_t *red, uint32_t *ir) {
    i2c_start(); i2c_write(MAX_ADDR << 1); i2c_write(0x07); 
    i2c_start(); i2c_write((MAX_ADDR << 1) | 1); 
    uint8_t r1 = i2c_read(1), r2 = i2c_read(1), r3 = i2c_read(1);
    uint8_t i1 = i2c_read(1), i2 = i2c_read(1), i3 = i2c_read(0); 
    i2c_stop();
    *red = (((uint32_t)r1 << 16) | ((uint32_t)r2 << 8) | r3) & 0x03FFFF;
    *ir  = (((uint32_t)i1 << 16) | ((uint32_t)i2 << 8) | i3) & 0x03FFFF;
}

// --- LM35 Temperature Sensor ---
float read_lm35_tempC() {
    uint32_t raw_adc = REG_XADC & 0xFFF; 
    return ((float)raw_adc / 4095.0f) * 100.0f; 
}

// --- OLED SSD1306 Display Drivers ---
#define OLED_ADDR 0x3C

const uint8_t mini_font[][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00}, {0x42,0x61,0x51,0x49,0x46}, 
    {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39}, 
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03}, {0x36,0x49,0x49,0x49,0x36}, 
    {0x06,0x49,0x49,0x29,0x1E}, {0x00,0x00,0x00,0x00,0x00}, {0x00,0x60,0x60,0x00,0x00}
};

const uint8_t large_font[10][20] = {
    {0xE0,0xF8,0x1C,0x0E,0x0E,0x0E,0x0E,0x1C,0xF8,0xE0, 0x07,0x1F,0x38,0x70,0x70,0x70,0x70,0x38,0x1F,0x07},
    {0x00,0x0E,0x0E,0xFE,0xFE,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00},
    {0x1C,0x1E,0x0E,0x0E,0x0E,0x0E,0x8E,0xFE,0x7C,0x38, 0x70,0x78,0x3C,0x1E,0x0F,0x07,0x03,0x01,0x00,0x00},
    {0x0C,0x0E,0x0E,0x0E,0x0E,0x8E,0xCE,0xFE,0x7C,0x38, 0x30,0x70,0x70,0x70,0x70,0x38,0x39,0x1F,0x0F,0x07},
    {0x00,0x80,0xC0,0x60,0x30,0x18,0xFE,0xFE,0x00,0x00, 0x07,0x07,0x06,0x06,0x06,0x06,0x7F,0x7F,0x06,0x06},
    {0xFE,0xFE,0x8E,0x8E,0x8E,0x8E,0x8E,0x8E,0x8E,0x8C, 0x31,0x71,0x70,0x70,0x70,0x70,0x70,0x38,0x1F,0x0F},
    {0xE0,0xF8,0x1C,0x8E,0x8E,0x8E,0x8E,0x8C,0x18,0x00, 0x0F,0x1F,0x38,0x70,0x70,0x70,0x70,0x38,0x1F,0x0F},
    {0x0E,0x0E,0x0E,0x0E,0x0E,0x8E,0xCE,0x7E,0x3E,0x1E, 0x00,0x00,0x00,0x78,0x7F,0x07,0x00,0x00,0x00,0x00},
    {0x38,0x7C,0xCE,0x8E,0x8E,0x8E,0x8E,0xCE,0x7C,0x38, 0x0E,0x1F,0x39,0x71,0x71,0x71,0x71,0x39,0x1F,0x0E},
    {0x70,0xF8,0x8C,0x0E,0x0E,0x0E,0x0E,0x1C,0xF8,0xF0, 0x00,0x18,0x31,0x71,0x71,0x71,0x71,0x38,0x1F,0x0F}
};

const uint8_t lbl_temp[] = {0x01,0x01,0x7F,0x01,0x01,0x00, 0x38,0x54,0x54,0x54,0x18,0x00, 0x7C,0x04,0x18,0x04,0x78,0x00, 0xFC,0x24,0x24,0x24,0x18,0x00, 0x00,0x36,0x36,0x00,0x00,0x00}; 
const uint8_t lbl_spo2[] = {0x24,0x4A,0x4A,0x4A,0x30,0x00, 0xFC,0x24,0x24,0x24,0x18,0x00, 0x38,0x44,0x44,0x44,0x38,0x00, 0x42,0x61,0x51,0x49,0x46,0x00, 0x00,0x36,0x36,0x00,0x00,0x00}; 
const uint8_t lbl_hr[]   = {0x7F,0x08,0x08,0x08,0x7F,0x00, 0x7F,0x09,0x19,0x29,0x46,0x00, 0x00,0x36,0x36,0x00,0x00,0x00}; 
const uint8_t icon_heart[] = {0x0C, 0x1E, 0x3F, 0x7E, 0x3F, 0x1E, 0x0C, 0x00};

void oled_command(uint8_t cmd) {
    i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x00); i2c_write(cmd); i2c_stop();
}
void oled_set_cursor(uint8_t page, uint8_t col) {
    oled_command(0xB0 | (page & 0x07)); oled_command(0x00 | (col & 0x0F)); oled_command(0x10 | (col >> 4));    
}
void oled_init() {
    oled_command(0xAE); 
    oled_command(0x20); oled_command(0x02); 
    oled_command(0xA8); oled_command(0x3F); 
    oled_command(0xDA); oled_command(0x12); // Correct 128x64 mapping
    oled_command(0x8D); oled_command(0x14); 
    oled_command(0xAF); 
}
void oled_clear() {
    for(uint8_t page = 0; page < 8; page++) {
        oled_set_cursor(page, 0);
        i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x40); 
        for(int i = 0; i < 128; i++) i2c_write(0x00); 
        i2c_stop();
    }
}
void oled_draw_sprite(const uint8_t* sprite, uint8_t width, uint8_t page, uint8_t col) {
    oled_set_cursor(page, col); i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x40);
    for(int i = 0; i < width; i++) i2c_write(sprite[i]);
    i2c_stop();
}
void oled_draw_char(uint8_t idx, uint8_t page, uint8_t col) {
    oled_draw_sprite(mini_font[idx], 5, page, col);
    oled_draw_sprite(mini_font[10], 1, page, col+5); 
}
void oled_print_number(uint32_t num, uint8_t page, uint8_t col) {
    uint8_t digits[10]; int count = 0;
    if (num == 0) { oled_draw_char(0, page, col); return; }
    while(num > 0) { digits[count++] = num % 10; num /= 10; }
    for(int i = count - 1; i >= 0; i--) { oled_draw_char(digits[i], page, col); col += 6; }
}
void oled_print_large_number(uint32_t num, uint8_t start_page, uint8_t col) {
    uint8_t digits[10]; int count = 0;
    if (num == 0) { digits[0] = 0; count = 1; } 
    else { while(num > 0) { digits[count++] = num % 10; num /= 10; } }
    
    uint8_t current_col = col;
    for(int i = count - 1; i >= 0; i--) {
        oled_set_cursor(start_page, current_col);
        i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x40);
        for(int p = 0; p < 10; p++) i2c_write(large_font[digits[i]][p]);
        i2c_stop(); current_col += 12; 
    }
    current_col = col;
    for(int i = count - 1; i >= 0; i--) {
        oled_set_cursor(start_page + 1, current_col);
        i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x40);
        for(int p = 10; p < 20; p++) i2c_write(large_font[digits[i]][p]);
        i2c_stop(); current_col += 12;
    }
}

// --- Rolling Window DSP ---
#define WINDOW_SIZE 100

void calculate_vitals(uint32_t* red_buf, uint32_t* ir_buf, int *heart_rate, int *spo2) {
    uint32_t red_dc = 0, ir_dc = 0;
    uint32_t red_min = 0x3FFFF, red_max = 0;
    uint32_t ir_min = 0x3FFFF, ir_max = 0;
    
    for(int i = 0; i < WINDOW_SIZE; i++) {
        if(red_buf[i] < red_min && red_buf[i] > 0) red_min = red_buf[i];
        if(red_buf[i] > red_max) red_max = red_buf[i];
        red_dc += red_buf[i];
        
        if(ir_buf[i] < ir_min && ir_buf[i] > 0) ir_min = ir_buf[i];
        if(ir_buf[i] > ir_max) ir_max = ir_buf[i];
        ir_dc += ir_buf[i];
    }
    red_dc /= WINDOW_SIZE; 
    ir_dc /= WINDOW_SIZE;
    
    // Failsafe: No finger detected
    if (ir_dc < 50000 || (ir_max - ir_min) < 100) {
        *spo2 = 0; *heart_rate = 0; return;
    }

    uint32_t red_ac = red_max - red_min;
    uint32_t ir_ac = ir_max - ir_min;
    
    // SpO2 Formula
    float ratio = (float)(red_ac * ir_dc) / (float)(ir_ac * red_dc);
    float spo2_float = 110.0f - (25.0f * ratio); 
    if (spo2_float > 99.0f) spo2_float = 99.0f; 
    if (spo2_float < 70.0f) spo2_float = 70.0f;
    *spo2 = (int)spo2_float;

    // Heart Rate: IR Peak Detection (More stable than Red)
    int peaks = 0;
    int is_above = 0;
    uint32_t threshold = ir_dc + (ir_ac / 4); // 25% above baseline
    
    for(int i = 0; i < WINDOW_SIZE; i++) {
        if(ir_buf[i] > threshold && !is_above) {
            peaks++;
            is_above = 1;
        } else if (ir_buf[i] < ir_dc) {
            is_above = 0; // Reset crossing
        }
    }
    *heart_rate = peaks * 30; // Multiply by 30 to get Beats Per Minute (60s / 2s window)
}

// --- MAIN EXECUTION ---
int main() {
    delay_us(100000); // Boot delay
    REG_I2C = 0x03; 
    max30102_init();
    oled_init();
    oled_clear();

    oled_draw_sprite(lbl_temp, 30, 0, 0);   
    oled_draw_sprite(lbl_spo2, 30, 3, 0);   
    oled_draw_sprite(lbl_hr,   18, 6, 0);   
    oled_draw_sprite(icon_heart, 8, 0, 118); 
    
    // Circular Buffers
    uint32_t red_buf[WINDOW_SIZE] = {0};
    uint32_t ir_buf[WINDOW_SIZE] = {0};
    int buf_idx = 0;
    
    // Pre-fill the 2-second buffer to avoid false zeros at startup
    for(int i = 0; i < WINDOW_SIZE; i++) {
        max30102_read_fifo(&red_buf[i], &ir_buf[i]);
        delay_us(20000); 
    }

    while(1) {
        // Collect 25 new samples (~0.5 seconds elapsed)
        for(int i = 0; i < 25; i++) {
            max30102_read_fifo(&red_buf[buf_idx], &ir_buf[buf_idx]);
            buf_idx++;
            if(buf_idx >= WINDOW_SIZE) buf_idx = 0; // Wrap around
            delay_us(20000); // 50Hz sample rate
        }
        
        // Compute DSP on the full 100-sample window
        int hr = 0, spo2 = 0;
        calculate_vitals(red_buf, ir_buf, &hr, &spo2);
        
        float temp = read_lm35_tempC();

        // Clear data zones
        for(uint8_t r = 0; r <= 7; r++) {
            oled_set_cursor(r, 45); 
            i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x40);
            for(int i = 0; i < 65; i++) i2c_write(0x00); 
            i2c_stop();
        }
        
        // Print
        oled_print_number((uint32_t)temp, 0, 45);
        oled_draw_char(11, 0, 57); oled_draw_char(0, 0, 63);  
        
        oled_print_large_number((uint32_t)spo2, 3, 45);       
        oled_print_large_number((uint32_t)hr, 6, 45); 
    }
    return 0;
}