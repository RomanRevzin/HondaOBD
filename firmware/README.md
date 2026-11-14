# firmware/

Device firmware for the Raspberry Pi Pico 2 W (RP2350), C + the GNU Arm toolchain.
Architecture and rationale: [`../docs/PLAN.md`](../docs/PLAN.md) §6.

> **Status: Phase 1 scaffold, first draft.** The protocol logic is written; the Pico hardware binding
> (`hal_pico.c`) is a stub of TODOs to fill once parts arrive. Nothing here has been compiled with the
> Pico SDK yet (no SDK/hardware present when it was written). The frame codec *is* verified — see Tests.

## Layout

| File | What it is | State |
|---|---|---|
| `src/honda_frame.{h,c}` | Honda proprietary frame codec `[cmd,len,payload,checksum]` (sum-to-0) | **verified** (Tests) |
| `src/iso9141.{h,c}` | ISO 9141-2 engine: 5-baud slow init + request/response (Phase 1) | draft, untested on HW |
| `src/kline_hal.h` | Hardware abstraction for the K-line (pin 7) — the only HW dependency | interface |
| `src/hal_pico.c` | Pico 2 W implementation of the HAL | **stub / TODO** |
| `src/main.c` | Demo: init the ECU, read supported PIDs (mode 01 PID 00), print over USB | draft |
| `tests/test_honda_frame.c` | Host unit test for the codec (no hardware needed) | **passing** |
| `CMakeLists.txt` | Pico SDK build | draft |

The design keeps all protocol logic (frame codec, ISO 9141-2, later the Honda mode) behind the thin
`kline_hal.h` interface, so it can be unit-tested on a PC and only `hal_pico.c` touches the hardware.

## Tests (run now, no hardware)

The frame codec is checked against the real captured frames from the research notes
(`FE 04 72 8C`, `72 05 71 00 18`, `20 05 00 02 D9`). With any host C compiler:

```bash
cd firmware && cc -I src tests/test_honda_frame.c src/honda_frame.c -o test_honda_frame && ./test_honda_frame
```

(On Windows without gcc/clang, use an MSVC "x64 Native Tools" prompt: `cl /I src tests\test_honda_frame.c src\honda_frame.c`.) The same three vectors and the sum-to-0 rule were also verified independently — all pass.

## Building the firmware (when parts arrive)

1. Install the Pico SDK; set `PICO_SDK_PATH`; copy `pico_sdk_import.cmake` from the SDK into `firmware/`.
2. Fill in the TODOs in `src/hal_pico.c` (pin numbers finalized after the Phase 0 measurement).
3. `set(PICO_BOARD pico2_w)`, then `mkdir build && cd build && cmake .. && make` → `hondaobd.uf2`.

## Next steps

- Phase 0 on the car decides the real pin assignments and confirms the K-line idle level.
- Then implement `hal_pico.c` (UART0 for byte mode; GPIO override for the 5-baud init; a clean RX tap).
- Phase 3+: add the Honda-mode engine (reuse `honda_frame`) and a passive-sniff mode.

## Reuse / references

`iwanders/OBD9141` (MIT) for ISO 9141-2 patterns; `huhwhatbruh/HondaDiagnosticSuite` (GPL-3.0) for the
Honda protocol + DTC/signal data; `HondaECU/eculib` (GPL-3.0) and `kerpz/ArduinoHondaOBD` as references.
Project license is GPL-3.0-or-later (SPDX headers in each file; the canonical `COPYING` text is added
when the repo is first tagged).
