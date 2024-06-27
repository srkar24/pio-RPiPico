#include <cstdint>
#include <cstdio>
#include "hardware/pio_instructions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
// Assembly program
#include "blink.pio.h"

PIO glb_pio = pio0;

static const uint BTN1 = 16;
static const uint BASE_FREQ = 1;
static const uint SM0 = pio_claim_unused_sm(glb_pio, true); // Claim a free state machine on a PIO instance; (PIO pio, bool required)
static const uint SM1 = pio_claim_unused_sm(glb_pio, true);
static const uint SM2 = pio_claim_unused_sm(glb_pio, true);
static const uint LED0 = 15;
static const uint LED1 = 14;
static const uint LED2 = 13;

bool is_blinking_fast = false;
uint offset = pio_add_program(glb_pio, &blink_program); // Finds the location where there is enough space for our program. 

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq);
uint32_t get_freq(PIO pio, uint sm, uint freq);
void gpio_callback(uint gpio, uint32_t events);
void pull_user_input(PIO pio, uint sm, uint32_t freq);

int main() {

    stdio_init_all();
    gpio_init(BTN1);
    gpio_set_dir(BTN1, GPIO_IN);
    gpio_pull_down(BTN1);
    gpio_set_irq_enabled_with_callback(BTN1, GPIO_IRQ_EDGE_RISE, true, &gpio_callback); // Syntax: void gpio_set_irq_enabled_with_callback(uint gpio, uint32_t event_mask, bool enabled, gpio_irq_callback_t callback)

    blink_pin_forever(glb_pio, SM0, offset, LED0, BASE_FREQ);
    blink_pin_forever(glb_pio, SM1, offset, LED1, BASE_FREQ << 1); // * 2
    blink_pin_forever(glb_pio, SM2, offset, LED2, BASE_FREQ << 2); // * 4

    while (true);
}

// Initializing our PIO
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);    // Calling the helper function from blink.pio
    pio_sm_set_enabled(pio, sm, true);  // Start running PIO program in the state machine
    pio_sm_put_blocking(pio, sm, get_freq(pio, sm, freq)); // Write a word of data to a state machine's TX FIFO, blocking if the FIFO is full
}

uint32_t get_freq(PIO pio, uint sm, uint freq) {
    return (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BTN1 && events == GPIO_IRQ_EDGE_RISE) {
        if (is_blinking_fast) {         // set to blinker to faster frequency
            pull_user_input(glb_pio, SM0, BASE_FREQ);
            pull_user_input(glb_pio, SM1, BASE_FREQ << 1);
            pull_user_input(glb_pio, SM2, BASE_FREQ << 2);
            is_blinking_fast = false;
        } else {
            pull_user_input(glb_pio, SM0, BASE_FREQ << 1);
            pull_user_input(glb_pio, SM1, BASE_FREQ << 2);
            pull_user_input(glb_pio, SM2, BASE_FREQ << 3);
            is_blinking_fast = true;   // set the blinker to slower freqency

        }
    }
}

void pull_user_input(PIO pio, uint sm, uint32_t freq) {
    pio_sm_set_enabled(pio, sm, false); // Enable or disable a PIO state machine
    pio_sm_put_blocking(pio, sm, get_freq(pio, sm, freq)); // Write a word of data to a state machine's TX FIFO, blocking if the FIFO is full
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));  // Immediately execute an instruction on a state machine; Encode a PULL instruction.
    pio_sm_exec(pio, sm, pio_encode_out(pio_y, 32));
    pio_sm_set_enabled(pio, sm, true);
}
