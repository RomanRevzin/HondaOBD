// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 HondaOBD project
//
// Phase 1 demo: bring up the K-line, slow-init the engine ECU over ISO 9141-2,
// and read which OBD PIDs it supports (mode 01 PID 00). Prints over USB serial.
//
// STATUS: first draft. Needs the Pico SDK and the hal_pico.c TODOs filled in.

#include <stdio.h>
#include "pico/stdlib.h"
#include "iso9141.h"

int main(void)
{
    stdio_init_all();

    while (true)
    {
        printf("HondaOBD: attempting ISO 9141-2 slow init...\n");

        Iso9141Session session;
        if (iso9141_slow_init(&session))
        {
            printf("Init OK: key bytes %02X %02X\n", session.keyByte1, session.keyByte2);

            uint8_t supportedPidsRequest[2] = { 0x01u, 0x00u };
            uint8_t response[16];
            int dataLength = iso9141_request(&session,
                                             supportedPidsRequest,
                                             sizeof(supportedPidsRequest),
                                             response,
                                             sizeof(response));
            if (dataLength > 0)
            {
                printf("Supported-PID response:");
                for (int index = 0; index < dataLength; index++)
                {
                    printf(" %02X", response[index]);
                }
                printf("\n");
            }
            else
            {
                printf("Request failed (%d)\n", dataLength);
            }
        }
        else
        {
            printf("Init failed; retrying.\n");
        }

        sleep_ms(3000);
    }

    return 0;
}
