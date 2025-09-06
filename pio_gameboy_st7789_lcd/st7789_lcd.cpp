#include <stdio.h>
#include "pico/stdlib.h"
#include "st7789.hpp"

#include <stdbool.h>
#include "hardware/pio.h"
//#include "gblcd.pio.h"
#include "st7789_lcd.pio.h"

// only for the start icon
#include "wkm.h"


int main() {

    stdio_init_all();
    
    // Create LCD object
    st7789::ST7789 lcd;

//#define PIN_DIN 19      // SPI0 TX (MOSI) // GP19 (DIN)
//#define PIN_CLK 18      // SPI0 SCK // GP18 (CLK)
//#define PIN_CS 17       // SPI0 CSn // GP17 (CS)
//#define PIN_DC 16       // SPI0 RX // GP16 (DC)
//#define PIN_RESET 21    // GPIO // GP21 (RST)
//#define PIN_BL 20       // GPIO // GP20 (BL)
    
    // Configure and initialize LCD
    st7789::Config config;

    config.spi_inst = spi0;

    config.pin_din = 19;        // MOSI // SPI0 TX (MOSI) // GP19 (DIN)
    config.pin_sck = 18;        // SCK // SPI0 SCK // GP18 (CLK)
    config.pin_cs = 17;         // CS // SPI0 CSn // GP17 (CS)
    //config.pin_dc = 20;       // DC // SPI0 RX // GP16 (DC)
    config.pin_dc = 16;         // DC // SPI0 RX // GP16 (DC)
    //config.pin_reset = 15;    // RESET // GPIO // GP21 (RST)
    config.pin_reset = 21;      // RESET // GPIO // GP21 (RST)
    //config.pin_bl = 10;       // Backlight // GPIO // GP20 (BL)
    config.pin_bl = 20;         // Backlight // GPIO // GP20 (BL)

    config.width = 240;
    config.height = 320;

    config.rotation = st7789::ROTATION_0;
    
    // DMA
    config.dma.enabled = true;
    config.dma.buffer_size = 480; // Bytes per line of pixels
    
    // Initialize LCD
    if (!lcd.begin(config)) {
        printf("LCD initialization failed!\n");
        return -1;
    }
    
    // Rotation
    uint8_t current_rotation = st7789::ROTATION_90;
    lcd.setRotation((st7789::Rotation)current_rotation);
    
    // Clear screen
    lcd.clearScreen(0x00FF); // White background
    sleep_ms(1000);
    
    // Display WKM Logo
    lcd.drawImage(40, 0, 240, 240, wkm_data);
    
    PIO pio = pio0;
    uint state_machine_id = 0;
    uint offset = pio_add_program(pio, &st7789_lcd_program);
    st7789_lcd_program_init(pio, state_machine_id, offset);

    int x = 0;
    int y = 0;
    bool vSyncPrev = false;
    bool vSyncCurrent = false;
    bool vSyncFallingEdgeDetected = false;
    bool firstRun = false;

    uint16_t screenBuffer[23040];
    uint16_t data0;
    uint16_t data1;
    uint16_t vSync;

    while (true) {

        uint32_t result = pio_sm_get_blocking(pio, state_machine_id);
        vSync = (result >> 31) & 1;

        vSyncCurrent = vSync;
        if (!vSyncCurrent && vSyncPrev) {
            vSyncFallingEdgeDetected = true;
        }
        vSyncPrev = vSyncCurrent;

        // Wait for VSync
        if (!vSyncFallingEdgeDetected) {
            continue;
        }

        if (!firstRun) {
            firstRun = true;
            lcd.clearScreen(0x0000); // Clear screen on first run
        }

        // Populate the screen buffer with pixel data
        for (y = 0; y < 144; y++) {
            for (x = 0; x < 160; x++) {

                if (x > 0 | y > 0) {
                    result = pio_sm_get_blocking(pio, state_machine_id);
                }

                data0 = (result >> 29) & 1;
                data1 = (result >> 30) & 1;
                
                if (data0 && data1) screenBuffer[y * 160 + x] = 0x0000;
                else if (data0 && !data1) screenBuffer[y * 160 + x] = 0xEF7B;
                else if (!data0 && data1) screenBuffer[y * 160 + x] = 0x18C6;
                else screenBuffer[y * 160 + x] = 0xFFFF;
            }
        }

        lcd.drawImage(80, 48, 160, 144, screenBuffer);

        vSyncFallingEdgeDetected = false;
    }

    return 0;
} 