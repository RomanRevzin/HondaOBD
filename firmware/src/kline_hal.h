// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// Hardware abstraction layer for the single-wire K-line (DLC pin 7) driven
// through an ST L9637D transceiver.
//
// The protocol engines (iso9141, and later the Honda mode) talk ONLY to this
// interface, so the same protocol code runs against:
//   - hal_pico.c    — the real Raspberry Pi Pico 2 W hardware, and
//   - a host/simulator implementation used for off-car unit tests.
//
// The K-line is half-duplex on one wire: every byte transmitted is also heard
// back as an echo, which the protocol layer reads and discards.

#ifndef KLINE_HAL_H
#define KLINE_HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// --- Bit-level control, used only for the 5-baud slow-init address ---------
void kline_hal_set_tx_high(void);   // release the line to its recessive (idle-high) state
void kline_hal_set_tx_low(void);    // pull the line to its dominant (low) state
bool kline_hal_read_rx(void);       // sample the line; true = high

// --- UART-mode byte transfer -----------------------------------------------
// baudRate is 10400 for legislated OBD and (typically) 9600 for Honda mode.
void kline_hal_uart_begin(uint32_t baudRate);
void kline_hal_uart_end(void);
void kline_hal_uart_write_byte(uint8_t value);

// Read one byte, waiting at most timeoutMilliseconds.
// Returns true and stores the byte in *value on success; false on timeout.
bool kline_hal_uart_read_byte(uint8_t *value, uint32_t timeoutMilliseconds);

// --- Timing -----------------------------------------------------------------
void kline_hal_delay_milliseconds(uint32_t milliseconds);
void kline_hal_delay_microseconds(uint32_t microseconds);
uint32_t kline_hal_milliseconds_now(void);

#endif // KLINE_HAL_H
