#include "boards/pico.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"
#include <iostream>

int main () {

    stdio_init_all();
    // static const uint led_pin = 25;
    static const float pio_freq = 2000;

    // Choose pio instance (0 or 1, there are total of 2 PIO instances available)
    PIO pio = pio0;

    // Get the first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);

    // Add PIO program to PIO instruction memory. 
    // SDK will find location and return with the memory offset of the program
    // Finds the location where there is enough space for our program. 
    uint offset = pio_add_program(pio, &blink_program);

    // Calculate the PIO clock divider
    float div = (float)clock_get_hz(clk_sys) / pio_freq;

    // Initialize the program using the helper function in blink.pio file
    blink_program_init(pio, sm, offset, PICO_DEFAULT_LED_PIN, div);

    // Start running PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);

    while (true) {
        sleep_ms(1000);
        std::cout << "Message from processor" << std::endl;
    }
}