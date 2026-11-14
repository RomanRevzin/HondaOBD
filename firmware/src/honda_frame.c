// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project

#include "honda_frame.h"

uint8_t honda_frame_checksum(const uint8_t *frameBytes, size_t byteCountBeforeChecksum)
{
    uint32_t runningSum = 0u;
    for (size_t index = 0u; index < byteCountBeforeChecksum; index++)
    {
        runningSum += frameBytes[index];
    }
    // Two's complement so that (preceding bytes + checksum) == 0 (mod 256).
    uint8_t checksum = (uint8_t)((0x100u - (runningSum & 0xFFu)) & 0xFFu);
    return checksum;
}

size_t honda_frame_build(uint8_t commandByte,
                         const uint8_t *payloadBytes,
                         size_t payloadLength,
                         uint8_t *destinationBuffer,
                         size_t destinationCapacity)
{
    size_t totalLength = payloadLength + HONDA_FRAME_OVERHEAD_BYTES;
    if (totalLength > HONDA_FRAME_MAX_TOTAL_BYTES)
    {
        return 0u;
    }
    if (totalLength > destinationCapacity)
    {
        return 0u;
    }

    destinationBuffer[0] = commandByte;
    destinationBuffer[1] = (uint8_t)totalLength;
    for (size_t index = 0u; index < payloadLength; index++)
    {
        destinationBuffer[2u + index] = payloadBytes[index];
    }

    uint8_t checksum = honda_frame_checksum(destinationBuffer, totalLength - 1u);
    destinationBuffer[totalLength - 1u] = checksum;
    return totalLength;
}

bool honda_frame_is_valid(const uint8_t *frameBytes, size_t receivedLength)
{
    if (receivedLength < HONDA_FRAME_OVERHEAD_BYTES)
    {
        return false;
    }

    uint8_t declaredLength = frameBytes[1];
    if ((size_t)declaredLength != receivedLength)
    {
        return false;
    }

    uint32_t runningSum = 0u;
    for (size_t index = 0u; index < receivedLength; index++)
    {
        runningSum += frameBytes[index];
    }
    return (runningSum & 0xFFu) == 0u;
}
