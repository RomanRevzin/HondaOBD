# firmware/

Device firmware for the Raspberry Pi Pico 2 W (RP2350), written in C with the GNU Arm Embedded
toolchain (already installed on the dev PC). Nothing here yet — scaffolding comes once Phase 0
confirms the car's wiring. Architecture and rationale: [`../docs/PLAN.md`](../docs/PLAN.md) §6.

## Responsibility split

Firmware owns everything that must be timed precisely; the host ([`../host/`](../host/)) owns protocol
logic. Planned layers:

- **transport** — USB CDC / BLE / Wi-Fi to the host (binary framing)
- **service** — J2534-shaped primitives (init, send/recv, timing params) + a raw pulse primitive +
  passive sniff mode (timestamped byte logger)
- **protocol engines** — (a) ISO 9141-2 legislated OBD, (b) Honda K-line mode, (c) DIAG-H
- **physical drivers** — K-line, DIAG-H, SCS, WEN (PIO-timed)

## Reuse / references

- `iwanders/OBD9141` (MIT) — ISO 9141-2 5-baud + fast init, echo-compare, DTCs, an ECU simulator.
- `HondaECU/eculib` (GPL-3.0) — K-series-relevant Honda K-line framing/flash reference.
- `kerpz/ArduinoHondaOBD` (no license) — older Honda 9600-baud protocol (reference only; facts, not code).

Project license is GPL-3.0, so GPL references may be reused directly.
