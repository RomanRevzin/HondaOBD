// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// STATUS: first draft, written without hardware present. Validate on the bench.

#include "iso9141.h"
#include "kline_hal.h"

// --- ISO 9141-2 timing (milliseconds) --------------------------------------
#define ISO9141_FIVE_BAUD_BIT_MS 200u    // 5 bit/s => 200 ms per bit
#define ISO9141_IDLE_BEFORE_INIT_MS 300u // W5: bus idle before the init address
#define ISO9141_W1_MAX_MS 300u           // init address -> 0x55 sync
#define ISO9141_KEYBYTE_TIMEOUT_MS 50u   // between sync/key bytes (W2, W3)
#define ISO9141_W4_MS 30u                // before echoing inverted key byte 2
#define ISO9141_ECHO_TIMEOUT_MS 60u      // time to hear our own transmitted byte
#define ISO9141_INTERBYTE_MS 6u          // P4: spacing between request bytes
#define ISO9141_RESPONSE_GAP_MS 60u      // P2: a gap longer than this ends the reply
#define ISO9141_KEEP_ALIVE_AFTER_MS 3000u // send keep-alive before the 5 s P3 limit

#define ISO9141_MAX_MESSAGE_BYTES 32u

static uint8_t iso9141_sum_checksum(const uint8_t *bytes, size_t count)
{
    uint32_t runningSum = 0u;
    for (size_t index = 0u; index < count; index++)
    {
        runningSum += bytes[index];
    }
    return (uint8_t)(runningSum & 0xFFu);
}

// On the single-wire bus we hear our own byte; send it, then read and check the echo.
static bool iso9141_write_byte_and_consume_echo(uint8_t value)
{
    kline_hal_uart_write_byte(value);

    uint8_t echo = 0u;
    if (!kline_hal_uart_read_byte(&echo, ISO9141_ECHO_TIMEOUT_MS))
    {
        return false;
    }
    return echo == value;
}

bool iso9141_slow_init(Iso9141Session *session)
{
    session->initialised = false;

    // Hold the line idle (recessive) for W5 so the ECU recognises a fresh init.
    kline_hal_uart_end();
    kline_hal_set_tx_high();
    kline_hal_delay_milliseconds(ISO9141_IDLE_BEFORE_INIT_MS);

    // Transmit the init address 0x33 at 5 baud, LSB first: start bit (low),
    // eight data bits, stop bit (high).
    kline_hal_set_tx_low();
    kline_hal_delay_milliseconds(ISO9141_FIVE_BAUD_BIT_MS);

    uint8_t address = ISO9141_INIT_ADDRESS;
    for (uint8_t bitIndex = 0u; bitIndex < 8u; bitIndex++)
    {
        bool bitIsOne = ((address >> bitIndex) & 0x01u) != 0u;
        if (bitIsOne)
        {
            kline_hal_set_tx_high();
        }
        else
        {
            kline_hal_set_tx_low();
        }
        kline_hal_delay_milliseconds(ISO9141_FIVE_BAUD_BIT_MS);
    }

    kline_hal_set_tx_high();
    kline_hal_delay_milliseconds(ISO9141_FIVE_BAUD_BIT_MS);

    // Switch to UART mode and read the ECU's 0x55 sync and two key bytes.
    kline_hal_uart_begin(ISO9141_BAUD_RATE);

    uint8_t syncByte = 0u;
    if (!kline_hal_uart_read_byte(&syncByte, ISO9141_W1_MAX_MS))
    {
        return false;
    }
    if (syncByte != 0x55u)
    {
        return false;
    }
    if (!kline_hal_uart_read_byte(&session->keyByte1, ISO9141_KEYBYTE_TIMEOUT_MS))
    {
        return false;
    }
    if (!kline_hal_uart_read_byte(&session->keyByte2, ISO9141_KEYBYTE_TIMEOUT_MS))
    {
        return false;
    }

    // Wait W4, then echo back the inverted key byte 2.
    kline_hal_delay_milliseconds(ISO9141_W4_MS);
    uint8_t invertedKey2 = (uint8_t)(~session->keyByte2);
    if (!iso9141_write_byte_and_consume_echo(invertedKey2))
    {
        return false;
    }

    // The ECU answers with the inverted init address (0xCC = ~0x33).
    uint8_t invertedAddress = 0u;
    if (!kline_hal_uart_read_byte(&invertedAddress, ISO9141_KEYBYTE_TIMEOUT_MS))
    {
        return false;
    }
    if (invertedAddress != (uint8_t)(~ISO9141_INIT_ADDRESS))
    {
        return false;
    }

    session->initialised = true;
    session->lastActivityMilliseconds = kline_hal_milliseconds_now();
    return true;
}

int iso9141_request(Iso9141Session *session,
                    const uint8_t *serviceAndPidBytes,
                    size_t requestLength,
                    uint8_t *responseBuffer,
                    size_t responseCapacity)
{
    if (!session->initialised)
    {
        return -1;
    }
    if ((requestLength + 4u) > ISO9141_MAX_MESSAGE_BYTES)
    {
        return -1;
    }

    // Assemble 68 6A F1 <service/pid...> <sum-checksum>.
    uint8_t requestFrame[ISO9141_MAX_MESSAGE_BYTES];
    size_t frameLength = 0u;
    requestFrame[frameLength++] = ISO9141_REQUEST_HEADER_0;
    requestFrame[frameLength++] = ISO9141_REQUEST_HEADER_1;
    requestFrame[frameLength++] = ISO9141_REQUEST_HEADER_2;
    for (size_t index = 0u; index < requestLength; index++)
    {
        requestFrame[frameLength++] = serviceAndPidBytes[index];
    }
    requestFrame[frameLength] = iso9141_sum_checksum(requestFrame, frameLength);
    frameLength++;

    for (size_t index = 0u; index < frameLength; index++)
    {
        if (!iso9141_write_byte_and_consume_echo(requestFrame[index]))
        {
            return -1;
        }
        kline_hal_delay_milliseconds(ISO9141_INTERBYTE_MS);
    }

    // Collect the reply until a gap longer than P2 ends it.
    uint8_t responseFrame[ISO9141_MAX_MESSAGE_BYTES];
    size_t responseFrameLength = 0u;
    while (responseFrameLength < ISO9141_MAX_MESSAGE_BYTES)
    {
        uint8_t received = 0u;
        if (!kline_hal_uart_read_byte(&received, ISO9141_RESPONSE_GAP_MS))
        {
            break;
        }
        responseFrame[responseFrameLength++] = received;
    }

    // Minimum valid reply: 3-byte header + at least one data byte + checksum.
    if (responseFrameLength < 5u)
    {
        return -1;
    }

    uint8_t expectedChecksum = iso9141_sum_checksum(responseFrame, responseFrameLength - 1u);
    if (responseFrame[responseFrameLength - 1u] != expectedChecksum)
    {
        return -1;
    }

    // Strip the 3-byte reply header and the trailing checksum.
    size_t dataLength = responseFrameLength - 3u - 1u;
    if (dataLength > responseCapacity)
    {
        return -1;
    }
    for (size_t index = 0u; index < dataLength; index++)
    {
        responseBuffer[index] = responseFrame[3u + index];
    }

    session->lastActivityMilliseconds = kline_hal_milliseconds_now();
    return (int)dataLength;
}

void iso9141_keep_alive_if_needed(Iso9141Session *session)
{
    if (!session->initialised)
    {
        return;
    }

    uint32_t elapsed = kline_hal_milliseconds_now() - session->lastActivityMilliseconds;
    if (elapsed < ISO9141_KEEP_ALIVE_AFTER_MS)
    {
        return;
    }

    // Mode 01 PID 00 is the conventional keep-alive request.
    uint8_t keepAliveRequest[2] = { 0x01u, 0x00u };
    uint8_t discardBuffer[ISO9141_MAX_MESSAGE_BYTES];
    (void)iso9141_request(session, keepAliveRequest, sizeof(keepAliveRequest),
                          discardBuffer, sizeof(discardBuffer));
}
