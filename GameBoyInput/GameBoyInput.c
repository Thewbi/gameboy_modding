#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

//#include "hardware/uart.h"




// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

// // UART defines
// // By default the stdout UART is `uart0`, so we will use the second one
// #define UART_ID uart1
// #define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
//#define UART_TX_PIN 4
//#define UART_RX_PIN 5

// #define BUTTONS_DPAD_PIN 27
// #define BUTTONS_OTHER_PIN 21

// #define BUTTONS_RIGHT_A_PIN 9
// #define BUTTONS_LEFT_B_PIN 12
// #define BUTTONS_UP_SELECT_PIN 16
// #define BUTTONS_DOWN_START_PIN 34

// https://deepbluembedded.com/wp-content/uploads/2023/09/Raspberry-Pi-Pico-Pinout.png
const uint BUTTONS_DPAD_PIN = 0; // GP0 (pin 1)
const uint BUTTONS_OTHER_PIN = 1; // GP1 (pin 2)
const uint BUTTONS_RIGHT_A_PIN = 2; // GP2 (pin 4)
const uint BUTTONS_LEFT_B_PIN = 3; // GP3 (pin 5)
const uint BUTTONS_UP_SELECT_PIN = 4; // GP4 (pin 6)
const uint BUTTONS_DOWN_START_PIN = 5; // GP5 (pin 7)

const uint SELECT_INPUT_PIN = 16; // GP16 (pin 21)


#ifndef LED_DELAY_MS
#define LED_DELAY_MS 500
#endif

void core1_main()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
    }
}

int main()
{
    //stdio_init_all();

    multicore_launch_core1(&core1_main);

    // // Set up our UART
    // uart_init(UART_ID, BAUD_RATE);
    // // Set the TX and RX pins by using the function select on the GPIO
    // // Set datasheet for more information on function select
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // // Use some the various UART functions to send out data
    // // In a default system, printf will also output via the default UART
    
    // // Send out a string, with CR/LF conversions
    // uart_puts(UART_ID, " Hello, UART!\n");
    
    // // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // GPIO init
    gpio_init(BUTTONS_DPAD_PIN);
    gpio_init(BUTTONS_OTHER_PIN);
    gpio_init(BUTTONS_RIGHT_A_PIN);
    gpio_init(BUTTONS_LEFT_B_PIN);
    gpio_init(BUTTONS_UP_SELECT_PIN);
    gpio_init(BUTTONS_DOWN_START_PIN);
    gpio_init(SELECT_INPUT_PIN);    

    // directions
    gpio_set_dir(BUTTONS_DPAD_PIN, GPIO_IN);
    gpio_set_dir(BUTTONS_OTHER_PIN, GPIO_IN);
    // gpio_set_dir(BUTTONS_DPAD_PIN, GPIO_OUT);
    // gpio_set_dir(BUTTONS_OTHER_PIN, GPIO_OUT);
    gpio_set_dir(BUTTONS_RIGHT_A_PIN, GPIO_OUT);
    gpio_set_dir(BUTTONS_LEFT_B_PIN, GPIO_OUT);
    gpio_set_dir(BUTTONS_UP_SELECT_PIN, GPIO_OUT);
    gpio_set_dir(BUTTONS_DOWN_START_PIN, GPIO_OUT);
    gpio_set_dir(SELECT_INPUT_PIN, GPIO_IN);

    // Pull Up/Down
    gpio_pull_up(SELECT_INPUT_PIN);

    gpio_put(BUTTONS_RIGHT_A_PIN, 1);
    gpio_put(BUTTONS_LEFT_B_PIN, 1);
    gpio_put(BUTTONS_UP_SELECT_PIN, 1);
    gpio_put(BUTTONS_DOWN_START_PIN, 1);

    // gpio_put(BUTTONS_RIGHT_A_PIN, 0);
    // gpio_put(BUTTONS_LEFT_B_PIN, 0);
    // gpio_put(BUTTONS_UP_SELECT_PIN, 0);
    // gpio_put(BUTTONS_DOWN_START_PIN, 0);

    int toggle = 0;
    uint pause = 60;

    int cycle = 0;
    int select = 0;

    while (true) {

        // if (cycle > 60) {
        //     cycle = 0;
        //     select = 1 - select;
        // }
        // cycle = cycle + 1;


        //printf("Hello, world!\n");
        //sleep_ms(1000);

        // toggle = 1 - toggle;
        // sleep_ms(250);
        // gpio_put(PICO_DEFAULT_LED_PIN, toggle);


        // // toggle = 1 - toggle;
        // gpio_put(BUTTONS_DPAD_PIN, toggle);
        // gpio_put(BUTTONS_OTHER_PIN, toggle);
        // gpio_put(BUTTONS_RIGHT_A_PIN, toggle); // 9 (ok)
        // gpio_put(BUTTONS_LEFT_B_PIN, toggle); // 4 (fail) // 11 (fail) // 12 (ok)
        // gpio_put(BUTTONS_UP_SELECT_PIN, toggle); // 7 (OK)
        // gpio_put(BUTTONS_DOWN_START_PIN, toggle); // 6 (fail)


        uint dpad_selected_value = gpio_get(BUTTONS_DPAD_PIN);
        uint other_selected_value = gpio_get(BUTTONS_OTHER_PIN);

        select = gpio_get(SELECT_INPUT_PIN);

        if (dpad_selected_value == 0) {

            gpio_put(BUTTONS_RIGHT_A_PIN, 1); // Right
            gpio_put(BUTTONS_LEFT_B_PIN, 1); // Left
            gpio_put(BUTTONS_UP_SELECT_PIN, 1); // Up
            gpio_put(BUTTONS_DOWN_START_PIN, 1); // Down

        } else if (other_selected_value == 0) {

            gpio_put(BUTTONS_RIGHT_A_PIN, 1); // A
            gpio_put(BUTTONS_LEFT_B_PIN, 1); // B
            gpio_put(BUTTONS_UP_SELECT_PIN, select); // select
            //gpio_put(BUTTONS_UP_SELECT_PIN, 1); // select
            gpio_put(BUTTONS_DOWN_START_PIN, 1); // start

        } else {

            gpio_put(BUTTONS_RIGHT_A_PIN, 1);
            gpio_put(BUTTONS_LEFT_B_PIN, 1);
            gpio_put(BUTTONS_UP_SELECT_PIN, 1);
            gpio_put(BUTTONS_DOWN_START_PIN, 1); // DOWN
            //gpio_put(BUTTONS_DOWN_START_PIN, select); // DOWN

        }


        // gpio_put(BUTTONS_RIGHT_A_PIN, 1);
        // gpio_put(BUTTONS_LEFT_B_PIN, 1);
        // gpio_put(BUTTONS_UP_SELECT_PIN, 1);
        // gpio_put(BUTTONS_DOWN_START_PIN, 1);
    }
}
