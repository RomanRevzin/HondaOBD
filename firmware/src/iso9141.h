// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// ISO 9141-2 engine for legislated OBD-II on the 2001-2005 Civic K-line.
//
// This is the Phase 1 foundation: standard, fully documented, and the safest
// first thing to get working on the car. 5-baud slow init to address 0x33,
// then 10400 baud; request header 68 6A F1, response header 48 6B <ecu>.
// (On a K20 the engine ECU answers at address 0x11 — see docs/research/ep3-03.)
//
// STATUS: first draft, written without hardware present. The timing constants
// come from ISO 9141-2 and the research notes; validate on the bench in Phase 1.

#ifndef ISO9141_H
#define ISO9141_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ISO9141_BAUD_RATE 10400u
#define ISO9141_INIT_ADDRESS 0x33u
#define ISO9141_REQUEST_HEADER_0 0x68u
#define ISO9141_REQUEST_HEADER_1 0x6Au
#define ISO9141_REQUEST_HEADER_2 0xF1u   // tester (our) address

typedef struct
{
    bool initialised;
    uint8_t keyByte1;
    uint8_t keyByte2;
    uint32_t lastActivityMilliseconds;
} Iso9141Session;

// Perform the 5-baud slow init. Returns true and fills session on success.
bool iso9141_slow_init(Iso9141Session *session);

// Send one OBD request (serviceAndPidBytes, e.g. {0x01, 0x00}) and read the
// response. The 68 6A F1 header and the trailing sum-checksum are added here.
// Returns the number of response DATA bytes written to responseBuffer
// (the bytes after the 3-byte reply header, excluding the checksum),
// or a negative value on error.
int iso9141_request(Iso9141Session *session,
                    const uint8_t *serviceAndPidBytes,
                    size_t requestLength,
                    uint8_t *responseBuffer,
                    size_t responseCapacity);

// Send a keep-alive request if the bus has been idle close to the 5 s P3 limit.
void iso9141_keep_alive_if_needed(Iso9141Session *session);

#endif // ISO9141_H
