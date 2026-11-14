# Bill of Materials — breadboard prototype

Enough to build the Phase 1–3 prototype and start talking to the car. Everything here is the
**minimum to begin**; the custom-PCB BOM comes later. Prices are rough 2026 ballparks (USD) and vary by
region/vendor — treat them as "order of magnitude," not quotes.

Hardware purchases are in-scope (your call, 2026-11-14); only paid *software/protocols* are off the table.

## Core — buy these first (~$30–45)

| # | Part | Qty | ~Price | Purpose / notes |
|---|---|---|---|---|
| 1 | **Raspberry Pi Pico 2 W** (RP2350) | 1–2 | $7 ea | The brain. PIO does the exact K-line bit-timing + sniffing. Buy 2 (one spare). |
| 2 | **ST L9637D** K-line transceiver (SOIC-8) | 2 | $1–2 ea | Pin-7 K-line driver/receiver; no TXD timeout. Buy 2 (fragile, cheap). |
| 3 | SOIC-8 → DIP breakout board | 2 | $1 | So the L9637D fits a breadboard. |
| 4 | **OBD-II (J1962) male plug**, all 16 pins, screw/solder terminals or flying leads | 1 | $6–10 | Plugs into the car. Must break out **all** pins (esp. 7, 9, 12, 14, 16, 4, 5) — most "OBD cables" omit 9/12/14. A bare 16-pin male connector or an OBD **breakout box** is ideal. |
| 5 | OBD-II male→female **extension cable**, full 16-pin | 1 | $6–10 | Lets the device sit inline for passive sniffing; also saves wear on the car's port. |
| 6 | Breadboard + jumper wires | 1 set | $8 | Prototyping. |
| 7 | Resistors: 510 Ω (0.5 W) ×4, 10 kΩ ×5, 2.2 kΩ ×5, assorted | 1 kit | $6 | 510 Ω = the K-line tester pull-up; others for dividers/bases. A through-hole kit covers everything. |

## Protection & power (~$10–15) — needed before leaving it plugged in

| # | Part | Qty | ~Price | Purpose |
|---|---|---|---|---|
| 8 | Automotive buck regulator 12 V→5 V (e.g. module with reverse-polarity + transient tolerance) | 1 | $4–8 | Powers the Pico from DLC pin 16. |
| 9 | TVS diode ~33 V (e.g. SMBJ series) + inline fuse holder + 1 A fuse | few | $4 | Load-dump/transient protection on the vehicle supply (pin 16 is always live). |
| 10 | Schottky diode (reverse-polarity) | 2 | $1 | Input protection. |
| 11 | Small MOSFETs / BJTs for the SCS (pin 9) low-side switch + logic level shifting | few | $3 | Open-drain SCS ground switch with read-back. |

## Test / debug (~$12–25) — strongly recommended, not strictly required day 1

| # | Part | Qty | ~Price | Purpose |
|---|---|---|---|---|
| 12 | **USB logic analyzer** (8-ch, "24 MHz" clone works with sigrok/PulseView) | 1 | $8–12 | See the real K-line waveform/timing; verify init and byte framing. Hugely helpful. |
| 13 | Cheap digital multimeter | 1 | $10 | Phase 0: probe which DLC pins are populated and their idle voltages. |
| 14 | *(Optional)* FTDI "KKL" USB↔K-line cable | 1 | $8–15 | Lets you poke at the K-line from a PC before the Pico firmware exists. |

## Later / optional (not now)

- **Spare junkyard ECU `37820-PNF-A05`** (~$30–60) — for safe **bench** reverse-engineering and
  firmware-dump work without touching the car. Only when we reach the reflash phase.
- **GODIAG GD101** J2534/ELM327 interface (~$25–30) — only if you ever want to *sniff a real HDS
  session*; it's optional hardware (not a software purchase) and not required for the read tool.
- Custom PCB + enclosure — after the breadboard proves the design.

## Notes

- **Don't** rely on a slim "OBD-II to DB9" cable — those carry only CAN/K-line/power and **omit Honda's
  pins 9/12/14**. You need all 16 conductors. A clear plastic **OBD-II breakout box** (~$15) is the
  nicest option if you'd rather not crimp a bare connector.
- You likely already have: USB cables, a soldering iron, a PC (you do). Those aren't listed.
- Rough total to **start**: **~$40–60** for core + protection; **~$60–85** with the logic analyzer and
  multimeter. Nothing here is a software/subscription cost.
