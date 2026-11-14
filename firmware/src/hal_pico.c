// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// Raspberry Pi Pico 2 W (RP2350) implementation of the K-line HAL.
//
// STATUS: STUB. The function bodies below are TODOs to be filled in when the
// hardware arrives and the Pico SDK is set up. The Pico SDK APIs to use are
// named in each comment. This file is excluded from the host unit-test build
// (see tests/) so the protocol modules can be tested on a PC without the SDK.
//
// Wiring assumption (revise after Phase 0 measurement):
//   - L9637D TX (its K-line driver input)  <- Pico GPIO  (UART0 TX in UART mode)
//   - L9637D RX (logic-side copy of the bus) -> Pico GPIO (UART0 RX; also the
//     clean sniffer tap, 0/3.3 V)
//   - A GPIO override is used for the 5-baud init (bit-bang), then the pin is
//     handed back to the UART.

#include "kline_hal.h"

// #include "pico/stdlib.h"
// #include "hardware/uart.h"
// #include "hardware/gpio.h"

// TODO: pick the real pins after Phase 0.
// #define KLINE_UART        uart0
// #define KLINE_TX_GPIO     0
// #define KLINE_RX_GPIO     1

void kline_hal_set_tx_high(void)
{
    // TODO: drive KLINE_TX_GPIO high as a plain GPIO output
    // (gpio_set_function GPIO_FUNC_SIO; gpio_put(KLINE_TX_GPIO, 1)).
}

void kline_hal_set_tx_low(void)
{
    // TODO: drive KLINE_TX_GPIO low (gpio_put(KLINE_TX_GPIO, 0)).
}

bool kline_hal_read_rx(void)
{
    // TODO: return gpio_get(KLINE_RX_GPIO).
    return true;
}

void kline_hal_uart_begin(uint32_t baudRate)
{
    // TODO: uart_init(KLINE_UART, baudRate); set 8N1; route the pins back to the
    // UART with gpio_set_function(..., GPIO_FUNC_UART).
    (void)baudRate;
}

void kline_hal_uart_end(void)
{
    // TODO: uart_deinit(KLINE_UART) and take the TX pin back as SIO for bit-bang.
}

void kline_hal_uart_write_byte(uint8_t value)
{
    // TODO: uart_putc_raw(KLINE_UART, value) (blocking).
    (void)value;
}

bool kline_hal_uart_read_byte(uint8_t *value, uint32_t timeoutMilliseconds)
{
    // TODO: poll uart_is_readable_within_us / a deadline; on data, *value =
    // uart_getc(KLINE_UART) and return true; on timeout return false.
    (void)value;
    (void)timeoutMilliseconds;
    return false;
}

void kline_hal_delay_milliseconds(uint32_t milliseconds)
{
    // TODO: sleep_ms(milliseconds).
    (void)milliseconds;
}

void kline_hal_delay_microseconds(uint32_t microseconds)
{
    // TODO: busy_wait_us(microseconds) for precise sub-ms timing.
    (void)microseconds;
}

uint32_t kline_hal_milliseconds_now(void)
{
    // TODO: return to_ms_since_boot(get_absolute_time()).
    return 0u;
}
