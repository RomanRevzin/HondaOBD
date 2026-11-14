// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// Host unit test for the Honda frame codec — runs on a PC, NO hardware needed.
// This verifies the codec against the real captured frames in the research notes.
//
// Build & run (gcc/clang):
//   cc -I ../src test_honda_frame.c ../src/honda_frame.c -o test_honda_frame && ./test_honda_frame
// Build & run (MSVC "x64 Native Tools" prompt):
//   cl /I ..\src test_honda_frame.c ..\src\honda_frame.c && test_honda_frame.exe

#include <stdio.h>
#include <string.h>
#include "honda_frame.h"

static int failureCount = 0;

static void expect_build(uint8_t commandByte,
                         const uint8_t *payload,
                         size_t payloadLength,
                         const uint8_t *expected,
                         size_t expectedLength,
                         const char *testName)
{
    uint8_t built[16];
    size_t builtLength = honda_frame_build(commandByte, payload, payloadLength,
                                           built, sizeof(built));

    bool lengthMatches = (builtLength == expectedLength);
    bool bytesMatch = lengthMatches && (memcmp(built, expected, expectedLength) == 0);

    if (!bytesMatch)
    {
        printf("FAIL %s: got", testName);
        for (size_t index = 0; index < builtLength; index++)
        {
            printf(" %02X", built[index]);
        }
        printf(", expected");
        for (size_t index = 0; index < expectedLength; index++)
        {
            printf(" %02X", expected[index]);
        }
        printf("\n");
        failureCount++;
        return;
    }

    if (!honda_frame_is_valid(built, builtLength))
    {
        printf("FAIL %s: built frame does not validate\n", testName);
        failureCount++;
        return;
    }

    printf("ok   %s\n", testName);
}

int main(void)
{
    // Wake frame: FE 04 72 8C
    const uint8_t wakePayload[] = { 0x72 };
    const uint8_t wakeExpected[] = { 0xFE, 0x04, 0x72, 0x8C };
    expect_build(0xFE, wakePayload, sizeof(wakePayload),
                 wakeExpected, sizeof(wakeExpected), "wake FE 04 72 8C");

    // Table read: 72 05 71 00 18  (0x72+0x05+0x71+0x00 = 0xE8; checksum 0x18)
    const uint8_t tableReadPayload[] = { 0x71, 0x00 };
    const uint8_t tableReadExpected[] = { 0x72, 0x05, 0x71, 0x00, 0x18 };
    expect_build(0x72, tableReadPayload, sizeof(tableReadPayload),
                 tableReadExpected, sizeof(tableReadExpected), "table read 72 05 71 00 18");

    // Legacy read: 20 05 00 02 D9
    const uint8_t legacyPayload[] = { 0x00, 0x02 };
    const uint8_t legacyExpected[] = { 0x20, 0x05, 0x00, 0x02, 0xD9 };
    expect_build(0x20, legacyPayload, sizeof(legacyPayload),
                 legacyExpected, sizeof(legacyExpected), "legacy 20 05 00 02 D9");

    if (failureCount == 0)
    {
        printf("\nAll honda_frame tests passed.\n");
        return 0;
    }

    printf("\n%d test(s) failed.\n", failureCount);
    return 1;
}
