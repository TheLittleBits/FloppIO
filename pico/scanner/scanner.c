#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "program.pio.h"
#include "hardware/clocks.h"

#define BAUD_RATE 1000000
#define HDD_CLICK_DELAY 10000

void init_uart() {
    // Init UART
    uart_init(uart0, BAUD_RATE);
    gpio_set_function(1, GPIO_FUNC_UART);
}

void init_sio() {
    // Init the 2 SLP pins of the DRV8825
    gpio_init(8); gpio_set_dir(8, true);
    gpio_init(9); gpio_set_dir(9, true);
    // Init and turn on the onboard led
    gpio_init(25); gpio_set_dir(25, true);
    gpio_put(25, 1);
}

void move_program_init(PIO pio, uint sm, uint pin) {
    // Init the scanner program
    uint offset = pio_add_program(pio, &move_program);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config config = move_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 1);
    float div = (float)clock_get_hz(clk_sys) / 1e6f;
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void endstop1_program_init(PIO pio, uint sm, uint pin) {
    // Init the scanner's endstops program
    uint offset = pio_add_program(pio, &endstop1_program);
    pio_gpio_init(pio, pin);
    gpio_pull_down(6);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config config = endstop1_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 1);
    float div = 65535.0f;
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void endstop2_program_init(PIO pio, uint sm, uint pin) {
    // Init the scanner's endstops program
    uint offset = pio_add_program(pio, &endstop2_program);
    pio_gpio_init(pio, pin);
    gpio_pull_down(7);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config config = endstop2_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 1);
    float div = 65535.0f;
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void hdd_program_init(PIO pio, uint sm, uint pin) {
    // Init one HDD program
    uint offset = pio_add_program(pio, &hdd_program);
    pio_gpio_init(pio, pin);
    pio_gpio_init(pio, pin + 1);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, true);
    pio_sm_config config = hdd_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 2);
    float div = (float)clock_get_hz(clk_sys) / 1e6f;
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void init_pio() {
    // Init the scanner programs
    move_program_init(pio0, 0, 2);
    endstop1_program_init(pio0, 1, 3);
    move_program_init(pio0, 2, 4);
    endstop2_program_init(pio0, 3, 5);
    // Enable the scanner programs
    pio_sm_set_enabled(pio0, 0, true);
    pio_sm_set_enabled(pio0, 1, true);
    pio_sm_set_enabled(pio0, 2, true);
    pio_sm_set_enabled(pio0, 3, true);
}

void enable_hdds(bool value) {
    /* If 'value' is true, the HDDs will be enabled. This
    function's purpose is to power. */

    if (value == true) {
        // Load the HDD program into the state machine
        hdd_program_init(pio1, 0, 14);
        hdd_program_init(pio1, 1, 16);
        hdd_program_init(pio1, 2, 18);
        hdd_program_init(pio1, 3, 20);
        // Load the HDD click delay into the OSR
        pio_sm_put_blocking(pio1, 0, HDD_CLICK_DELAY);
        pio_sm_put_blocking(pio1, 1, HDD_CLICK_DELAY);
        pio_sm_put_blocking(pio1, 2, HDD_CLICK_DELAY);
        pio_sm_put_blocking(pio1, 3, HDD_CLICK_DELAY);
        // Enable the HDDs
        pio_sm_set_enabled(pio1, 0, true);
        pio_sm_set_enabled(pio1, 1, true);
        pio_sm_set_enabled(pio1, 2, true);
        pio_sm_set_enabled(pio1, 3, true);
    } else if (value == false) {
        // Disable the HDDs
        pio_sm_set_enabled(pio1, 0, false);
        pio_sm_set_enabled(pio1, 1, false);
        pio_sm_set_enabled(pio1, 2, false);
        pio_sm_set_enabled(pio1, 3, false);
        // Set all HDD pins low to save power
        gpio_init(14); gpio_set_dir(14, true); gpio_put(14, 0);
        gpio_init(15); gpio_set_dir(15, true); gpio_put(15, 0);
        gpio_init(16); gpio_set_dir(16, true); gpio_put(16, 0);
        gpio_init(17); gpio_set_dir(17, true); gpio_put(17, 0);
        gpio_init(18); gpio_set_dir(18, true); gpio_put(18, 0);
        gpio_init(19); gpio_set_dir(19, true); gpio_put(19, 0);
        gpio_init(20); gpio_set_dir(20, true); gpio_put(20, 0);
        gpio_init(21); gpio_set_dir(21, true); gpio_put(21, 0);
    }
}

void stop_playing(int index) {
    // Turn off the according DRV8825
    switch (index) {
        case 0: gpio_put(8, false); break;
        case 1: gpio_put(9, false); break;
    }
}

void start_playing(int index) {
    // Turn on the according DRV8825
    switch (index) {
        case 0: gpio_put(8, true); break;
        case 1: gpio_put(9, true); break;
    }
}

void set_frequency(int index, int freq) {
    // Load the delay value into the according TX FIFO
    switch (index) {
        case 0: pio_sm_put_blocking(pio0, 0, 1000000/freq/2); break;
        case 1: pio_sm_put_blocking(pio0, 2, 1000000/freq/2); break;
    }
}

void hdd_click(int index) {
    if (index >= 0 && index <= 3) {
        pio_sm_put_blocking(pio1, index, HDD_CLICK_DELAY); // Deblock the according pio program so it toggles the H-bridge.
        // You can't put a fixed value in a PIO register, so I have to send the delay value every click
    }
}

void run_action(int action, int value) {
    // Each pico has it's own action table. We use 10 and 11 for the scanners and 126 for the HDDs. 0 - 9 are reserved for parameters and auxiliary actions
    switch (action) {
        case 0: break; // Reset the FDDs
        case 1: enable_hdds(value); break; // Enable the HDDs
        case 2: break;
        case 3: break;
        case 4: break;
        case 5: break;
        case 6: break;
        case 7: break;
        case 8: break;
        case 9: break;
        case 10: if (value == 0) {stop_playing(action - 10);} else {set_frequency(action - 10, value); start_playing(action - 10);} break;
        case 11: if (value == 0) {stop_playing(action - 10);} else {set_frequency(action - 10, value); start_playing(action - 10);} break;
        case 126: hdd_click(value); break;
    }
}

int main() {

    // Call all startup functions
    stdio_usb_init(); // Only because the ability of updating software without entering BOOTSEL mode manually
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
                    run_action(action, value1*127+value2); // Run the action received on UART
                    printf("%d, %d\n", action, value1*127+value2);
                }
            }
        }
    }
}