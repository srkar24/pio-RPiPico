#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pwm.pio.h"
#include <iostream>

static const uint LED0 = 15;

// Write `period` to the input shift register
void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

// Write `level` to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
    pio_sm_put_blocking(pio, sm, level);
}

int main() {
    stdio_init_all();

    // todo get free sm
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &pwm_program);
    pwm_program_init(pio, sm, offset, LED0);
    pio_pwm_set_period(pio, sm, (1u << 16) - 1);

    int level = 0;
    while (true) {
        std::cout << "Level = " << level << std::endl;
        pio_pwm_set_level(pio, sm, level * level); // influences the LED's brightness
        level = (level + 1) % 256; // cycles from 0 to 255 and then resets to 0, increments and keeps the level within the rannge of 0 - 255
        sleep_ms(10);
    }
}