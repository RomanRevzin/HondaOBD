# hardware/

Interface hardware for the HondaOBD device (schematics, PCB, BOM, bench notes). Nothing fabricated
yet — this holds the design as it firms up. Rationale and part choices live in
[`../docs/PLAN.md`](../docs/PLAN.md) §5.

## Target architecture (draft)

- **MCU:** Raspberry Pi Pico 2 W (RP2350) — PIO blocks give exact bit-timing for 5-baud init, Honda
  wake-up pulses, and timestamped sniffing.
- **K-line channel (DLC pin 7, ~12 V):** ST **L9637D** transceiver (no TXD dominant timeout). Switchable
  510 Ω pull-up (tester mode) / high-Z (sniffer mode).
- **DIAG-H channel (DLC pin 14, ~5 V):** a separate, independently-configured single-wire channel
  (selectable pull-up off/5 V/12 V, programmable threshold, ADC read-back). ⚠️ Never put 12 V on pin 14.
- **SCS switch (DLC pin 9):** protected open-drain low-side switch with read-back (for blink codes).
- **WEN driver (DLC pin 12):** switchable, current-limited 5 V driver (reflash phase only).
- **Power (DLC pin 16, always live):** reverse-polarity + load-dump/transient protection, wide buck,
  sub-mA sleep, physical power switch.
- **Sniffer tap + logic-analyzer test points** on pins 7/9/12/14.

## DLC pinout — 2002 Civic (to be re-confirmed on the actual EP3 Si in Phase 0)

| Pin | Wire | Signal | On which units |
|---|---|---|---|
| 4 | BLK | Chassis ground | — |
| 5 | BRN/YEL | Signal ground | ECM E3 |
| 7 | LT BLU | K-line (~12 V) | ECM E23 + ABS |
| 9 | BRN | SCS (~5 V) | ECM, SRS, ABS |
| 12 | RED/WHT | WEN (write-enable) | ECM E30 |
| 14 | GRY | DIAG-H (~5 V) | SRS, immobilizer, EPS |
| 16 | WHT/RED | Battery +12 V (perm.) | via BACK UP fuse |
