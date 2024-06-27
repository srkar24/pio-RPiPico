#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pwm.pio.h"
#include <iostream>

int steps = 6;
typedef enum {
    RH_GU_BL,
    RD_GH_BL,
    RL_GH_BU,
    RL_GD_BH,
    RU_GL_BH,
    RH_GL_BD
} CtrlLed_t;

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

    static const uint LEDR = 15;
    static const uint LEDG = 14;
    static const uint LEDB = 13;

    // todo get free sm
    PIO pio = pio0;
    uint sm0 = 0;
    uint sm1 = 1;
    uint sm2 = 2;
    uint offset = pio_add_program(pio, &pwm_program);

    pwm_program_init(pio, sm0, offset, LEDR);
    pio_pwm_set_period(pio, sm0, (1u << 16) - 1);

    pwm_program_init(pio, sm1, offset, LEDG);
    pio_pwm_set_period(pio, sm1, (1u << 16) - 1);

    pwm_program_init(pio, sm2, offset, LEDB);
    pio_pwm_set_period(pio, sm2, (1u << 16) - 1);

    int level = 0;
    int direction = 1;
    int ctrl = 0;

    while (true) {
        switch (ctrl) {
            case (RH_GU_BL): {
                direction = 1;
                // pio_pwm_set_level(pio, sm0, level * level); // influences the LED's brightness
                pio_pwm_set_level(pio, sm0, 255 * 255);
                pio_pwm_set_level(pio, sm1, level * level);
                pio_pwm_set_level(pio, sm2, 0);
                break;
            }

            case (RD_GH_BL): {
                direction = -1;
                pio_pwm_set_level(pio, sm0, level * level);
                pio_pwm_set_level(pio, sm1, 255 * 255);
                pio_pwm_set_level(pio, sm2, 0);
                break;
            }

            case (RL_GH_BU): {
                direction = 1;
                pio_pwm_set_level(pio, sm0, 0);
                pio_pwm_set_level(pio, sm1, 255 * 255);
                pio_pwm_set_level(pio, sm2, level * level);
                break;
            }

            case (RL_GD_BH): {
                direction = -1;
                pio_pwm_set_level(pio, sm0, 0);
                pio_pwm_set_level(pio, sm1, level * level);
                pio_pwm_set_level(pio, sm2, 255 * 255);
                break;
            }    

            case (RU_GL_BH): {
                direction = 1;
                pio_pwm_set_level(pio, sm0, level * level);
                pio_pwm_set_level(pio, sm1, 0);
                pio_pwm_set_level(pio, sm2, 255 * 255);
                break;
            }    

            case (RH_GL_BD): {
                direction = -1;
                pio_pwm_set_level(pio, sm0, 255 * 255);
                pio_pwm_set_level(pio, sm1, 0);
                pio_pwm_set_level(pio, sm2, level * level);
                break;
            }

            default: 
                break;                    
        }

        level = (level + (direction * 1)) % 256; // cycles from 0 to 255 and then resets to 0, increments and keeps the level within the rannge of 0 - 255
        
        if (level == 0 || level == 255) { // Reset 
            ctrl = (ctrl + 1) % steps;
        }
        sleep_ms(10);
    }
}