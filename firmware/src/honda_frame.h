// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// Honda proprietary K-line frame codec.
//
// Frame layout (confirmed against real captures and the HDS decompile — see
// docs/research/ep3-03 and ep3-04):
//
//     [ command | length | payload... | checksum ]
//
//   - "length" counts EVERY byte of the whole frame (command, length, payload,
//     checksum), not just the payload.
//   - "checksum" is the two's complement of the 8-bit sum of all preceding
//     bytes, so the whole frame sums to zero (mod 256).
//
// Worked examples that this codec must reproduce exactly:
//     FE 04 72 8C            (wake:  cmd FE, len 4, payload 72, cksum 8C)
//     72 05 71 pp cc         (table read: cmd 72, len 5, payload 71 pp)
//     20 05 00 02 D9         (legacy read: cmd 20, len 5, payload 00 02)

#ifndef HONDA_FRAME_H
#define HONDA_FRAME_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// command + length + checksum, i.e. every byte that is not payload.
#define HONDA_FRAME_OVERHEAD_BYTES 3u
#define HONDA_FRAME_MAX_TOTAL_BYTES 255u

// Two's-complement checksum over the first byteCountBeforeChecksum bytes.
uint8_t honda_frame_checksum(const uint8_t *frameBytes, size_t byteCountBeforeChecksum);

// Build a complete frame into destinationBuffer.
// Returns the total frame length in bytes, or 0 if it would not fit
// (payload too long for the length byte, or destination too small).
size_t honda_frame_build(uint8_t commandByte,
                         const uint8_t *payloadBytes,
                         size_t payloadLength,
                         uint8_t *destinationBuffer,
                         size_t destinationCapacity);

// True when receivedLength matches the frame's own length byte and the whole
// frame sums to zero (mod 256).
bool honda_frame_is_valid(const uint8_t *frameBytes, size_t receivedLength);

#endif // HONDA_FRAME_H
