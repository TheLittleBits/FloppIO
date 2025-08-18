#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "program.pio.h"
#include "hardware/clocks.h"

#define BAUD_RATE 1000000

void init_uart() {
    // Init UART
    uart_init(uart0, BAUD_RATE);
    gpio_set_function(1, GPIO_FUNC_UART);
}

void fdd_program_init(PIO pio, uint sm, uint pin) {
    // Init one fdd program
    uint offset = pio_add_program(pio, &fdd_program);
    pio_gpio_init(pio, pin);
    pio_gpio_init(pio, pin + 1);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, true);
    pio_sm_config config = fdd_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 2);
    float div = (float)clock_get_hz(clk_sys) / 1e6f; // The clock speed has to be 1 MHz for frequency calculations
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void init_pio() {
    // Init the PIO programs so they can enabled later
    fdd_program_init(pio0, 0, 2);
    fdd_program_init(pio0, 1, 4);
    fdd_program_init(pio0, 2, 6);
    fdd_program_init(pio0, 3, 8);
    fdd_program_init(pio1, 0, 10);
    fdd_program_init(pio1, 1, 12);
    fdd_program_init(pio1, 2, 14);
    fdd_program_init(pio1, 3, 16);
}

void enable_pio(bool value) {
    /* If 'value' is true, all the PIOs will be enabled. This
    function's purpose is to save processing power. */

    if (value == true) {
        pio_sm_set_enabled(pio0, 0, 1);
        pio_sm_set_enabled(pio0, 1, 1);
        pio_sm_set_enabled(pio0, 2, 1);
        pio_sm_set_enabled(pio0, 3, 1);
        pio_sm_set_enabled(pio1, 0, 1);
        pio_sm_set_enabled(pio1, 1, 1);
        pio_sm_set_enabled(pio1, 2, 1);
        pio_sm_set_enabled(pio1, 3, 1);
    } else if (value == false) {
        pio_sm_set_enabled(pio0, 0, 0);
        pio_sm_set_enabled(pio0, 1, 0);
        pio_sm_set_enabled(pio0, 2, 0);
        pio_sm_set_enabled(pio0, 3, 0);
        pio_sm_set_enabled(pio1, 0, 0);
        pio_sm_set_enabled(pio1, 1, 0);
        pio_sm_set_enabled(pio1, 2, 0);
        pio_sm_set_enabled(pio1, 3, 0);
    }
}

void init_sio() {
    // Init all ENABLE pins of the FDDs
    gpio_init(18); gpio_set_dir(18, true);
    gpio_init(19); gpio_set_dir(19, true);
    gpio_init(20); gpio_set_dir(20, true);
    gpio_init(21); gpio_set_dir(21, true);
    gpio_init(22); gpio_set_dir(22, true);
    gpio_init(26); gpio_set_dir(26, true);
    gpio_init(27); gpio_set_dir(27, true);
    gpio_init(28); gpio_set_dir(28, true);
    // Init and turn on the onboard led
    gpio_init(25); gpio_set_dir(25, true);
    gpio_put(25, 1);
}

void stop_playing(int index) {
    // Turn off the according FDD
    switch (index) {
        case 0: gpio_put(18, 0); break;
        case 1: gpio_put(19, 0); break;
        case 2: gpio_put(20, 0); break;
        case 3: gpio_put(21, 0); break;
        case 4: gpio_put(22, 0); break;
        case 5: gpio_put(26, 0); break;
        case 6: gpio_put(27, 0); break;
        case 7: gpio_put(28, 0); break;
    }
}

void start_playing(int index) {
    // Turn on the according FDD
    switch (index) {
        case 0: gpio_put(18, 1); break;
        case 1: gpio_put(19, 1); break;
        case 2: gpio_put(20, 1); break;
        case 3: gpio_put(21, 1); break;
        case 4: gpio_put(22, 1); break;
        case 5: gpio_put(26, 1); break;
        case 6: gpio_put(27, 1); break;
        case 7: gpio_put(28, 1); break;
    }
}

void set_frequency(int index, int freq) {
    /* Load the delay value into the according TX FIFO.
    The PIO delay code part has a "SET" instruction and a
    "JMP" instruction, so one delay loop cycle takes
    two microsecond. That's why the delay value is split in
    half. */

    switch (index) {
        case 0: pio_sm_put_blocking(pio0, 0, 1000000/freq/2); break;
        case 1: pio_sm_put_blocking(pio0, 1, 1000000/freq/2); break;
        case 2: pio_sm_put_blocking(pio0, 2, 1000000/freq/2); break;
        case 3: pio_sm_put_blocking(pio0, 3, 1000000/freq/2); break;
        case 4: pio_sm_put_blocking(pio1, 0, 1000000/freq/2); break;
        case 5: pio_sm_put_blocking(pio1, 1, 1000000/freq/2); break;
        case 6: pio_sm_put_blocking(pio1, 2, 1000000/freq/2); break;
        case 7: pio_sm_put_blocking(pio1, 3, 1000000/freq/2); break;
    }
}

int reset() {
    #define DIRECTION_MASK 174760    // Binary: 0b00000000000101010101010101000
    #define STEP_MASK      87380     // Binary: 0b00000000000010101010101010100
    #define LED_MASK       477888512 // Binary: 0b11100011111000000000000000000
    #define ALL_MASK       478150652 // Binary: 0b11100011111111111111111111100

    // Init all used pins
    gpio_init_mask(ALL_MASK);
    gpio_set_dir_out_masked(ALL_MASK);

    // Activate all FDDs
    gpio_set_mask(LED_MASK);

    // Move the head to max position
    gpio_clr_mask(DIRECTION_MASK);

    for (int i = 0; i < 85; i++) {
        gpio_set_mask(STEP_MASK);
        sleep_ms(10);
        gpio_clr_mask(STEP_MASK);
        sleep_ms(10);
    }

    // Change direction and move to the centre
    gpio_set_mask(DIRECTION_MASK);

    for (int i = 0; i < 42; i++) {
        gpio_set_mask(STEP_MASK);
        sleep_ms(10);
        gpio_clr_mask(STEP_MASK);
        sleep_ms(10);
    }

    gpio_clr_mask(ALL_MASK);
    gpio_init_mask(ALL_MASK); // "Deinit" all the used pins
}

void run_action(uint action, uint value) {
    /* Each pico has it's own action table. Here are the
    8 FDDs starting at 12. 0 - 9 are reserved for
    parameters and auxiliary actions */

    switch (action) {
        case 0: reset(); init_pio(); init_sio(); break;
        case 1: enable_pio(value); break;
        case 2: break;
        case 3: break;
        case 4: break;
        case 5: break;
        case 6: break;
        case 7: break;
        case 8: break;
        case 9: break;
        case 10: break; // Scanner #1
        case 11: break; // Scanner #2
        case 12: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 13: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 14: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 15: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 16: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 17: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 18: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
        case 19: if (value == 0) {stop_playing(action - 12);} else {set_frequency(action - 12, value); start_playing(action - 12);} break;
    }
}

int main() {

    // Call all startup functions
    stdio_usb_init(); // Only because the ability of updating software without entering BOOTSEL mode manually
    reset();
    init_uart();
    init_pio();
    init_sio();
    
    uint8_t action; // Here is the action stored
    uint8_t value1; // Here is the first data byte stored
    uint8_t value2; // Here the second

    for (;;) {
        action = uart_getc(uart0);
        if (action >= 128) { // Check if "action" is really an action byte and has MSB 1
            action -= 128;
            value1 = uart_getc(uart0);
            if (value1 <= 127) { // Then check if value1 is a data byte and has MSB 0
                value2 = uart_getc(uart0);
                if (value2 <= 127) { // Value2 also has to be a data byte
                    run_action(action, value1*127+value2);  // Run the action received on UART
                }
            }
        }
    }
}