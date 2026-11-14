# HondaOBD — Build Plan

A device that plugs into the 16-pin OBD2 connector of a 2002 Honda Civic and reads/writes
as many diagnostic codes and functions as the car physically allows. This is a right-to-repair
/ owner-diagnostics project on your own vehicle.

Status: **research complete, plan drafted, nothing built yet.** Detailed research notes with
sources and confidence tags are in [`docs/research/`](research/) (nine files, ~475 KB).

---

## 1. The headline reality

A "2002 Civic" is **not one car** — it's at least a dozen across markets (US D17 sedans/coupes,
UK-built EP3 Si with K20A3, European 1.4/1.6 petrol and 1.7 CTDi diesel, JDM Ferio, Type R…).
The electronics differ enough that **the single most important first step is pinning down your exact
car** (full VIN + the ECM part number `37820-XXX-XXX` on the box behind the glove box). Everything
below assumes the common **US-market D17 car** unless noted, because that's the best-documented and
the most likely; it gets re-checked against your actual car in Phase 0.

The good news, confirmed by multiple independent sources (Honda's own factory service manual via
charm.li, SAE standards, Honda's 2017 dealer job aid):

- **The whole 2001–2005 Civic range is pre-CAN.** All diagnostics run over Honda's single-wire
  **K-line** family, not CAN bus. CAN doesn't arrive on the Civic until 2006. This makes the car
  *simpler* electrically but means **no off-the-shelf modern OBD dongle reaches anything but
  emissions data** on it.
- There are **two separate diagnostic wires** on the connector, and this is the crux of the whole
  design (see §3).

What "read/write every possible code" splits into, by how reachable it is:

| Tier | Systems | Reachable from the 16-pin connector? |
|---|---|---|
| **A — fully reachable** | Engine/PGM-FI (ECM/PCM), Automatic trans (inside the PCM), ABS, SRS airbag, Immobilizer, EPS (Si only) | **Yes** |
| **B — on-car self-test only** | Body/multiplex unit, gauges, HVAC/climate, keyless | **No** — need separate in-dash service connectors + blink codes |
| **C — stretch goal** | ECM/PCM firmware read-out and reflash | Partly — dealer-only write path; see §4 |

So a connector-based device can genuinely cover **Tier A completely** (that is "every code" for the
engine, transmission, brakes, airbags, immobilizer and power steering). Tier B is physically
impossible from that connector on this car — worth saying plainly up front. Tier C is a real research
project, not a weekend build.

---

## 2. Scope definition — three concrete goals

1. **Diagnostics (read):** every stored DTC + freeze frame + live data + Honda's fast "snapshot"
   data for all Tier-A units; plus the legislated generic OBD-II data.
2. **Service functions (write):** clear DTCs, ECM/PCM reset, idle learn, ABS/SRS/EPS code clear,
   actuator/function tests — i.e. what the Honda dealer tool (HDS) does day to day.
3. **Reflash (stretch):** read out and reprogram the ECM/PCM calibration.

Goals 1 and the safe parts of 2 are the primary deliverable. Goal 3 and the security-gated parts of
2 (immobilizer key registration, airbag writes) are a separate, carefully-scoped later phase —
they're sensitive, they need Honda-issued per-car credentials even for the dealer, and they carry
real safety/theft considerations. We treat them as research, gated behind your explicit go-ahead.

---

## 3. The finding that shapes the hardware: two diagnostic lines

From Honda's factory connector views and splice diagrams (HIGH confidence, corroborated by three of
the nine research threads), the 2002 US Civic connector has exactly **seven populated pins**:

| Pin | Wire | Signal | Goes to | Notes |
|---|---|---|---|---|
| 4 | BLK | Chassis ground | Body | — |
| 5 | BRN/YEL | Signal ground | ECM/PCM E3 | — |
| **7** | LT BLU | **K-line** (~12 V idle) | ECM/PCM E23 **+ ABS unit** | ISO 9141-2 legislated OBD *and* Honda mode |
| **9** | BRN | **SCS** service line (~5 V idle) | ECM, SRS, ABS | Ground it → units blink stored codes |
| **12** | RED/WHT | **WEN** write-enable | ECM/PCM E30 | Dealer tool reportedly puts +5 V here to reflash |
| **14** | GRY | **DIAG-H** (~5 V idle) | **SRS + Immobilizer (+ EPS on Si)** | Honda's *second* serial line; NOT the K-line |
| 16 | WHT/RED | Battery +12 V (perm.) | via BACK UP fuse | ~4 A max, always live |

**The key consequence:** the engine/transmission and ABS live on the 12-volt K-line (pin 7), but the
airbag, immobilizer and electric-steering units live on a *different* ~5-volt single-wire line called
**DIAG-H** (pin 14). A normal K-line OBD adapter can't talk to them at all — which is exactly why cheap
scanners "can't see" airbag/SRS codes on these Hondas. To cover Tier A you must drive **both** lines,
plus switch the SCS line (pin 9) and, for reflash, the WEN line (pin 12).

⚠️ One hard safety rule falls straight out of this: **a 12 V K-line transceiver must never be wired to
pin 14.** DIAG-H is a ~5 V bus feeding the airbag and immobilizer units; 12 V there risks damaging
them. The design handles this with isolated, independently-configured channels (§5).

**Protocols, by confidence:**

- **Legislated OBD-II = ISO 9141-2** on pin 7 (5-baud init, address 0x33, 10.4 kbit/s). HIGH. This is
  the safe, fully-documented first milestone — standard, non-proprietary, same as thousands of other
  pre-2004 cars.
- **Honda "mode"** (the dealer protocol) also runs on pin 7 for ECM/ABS, and on pin 14 (DIAG-H) for
  SRS/immobilizer/EPS. The *frame format* is known (header/command, length byte, data, two's-complement
  checksum — consistent across Honda motorcycle tools, the 1992–2000 3-pin hobby protocol, and an
  HDS-derived database someone published). But **the actual request→reply byte sequences for a
  2001–2007 Honda car are not public.** No one has published a captured dealer-tool session on one of
  these cars. MEDIUM on format, LOW/unknown on the specific commands.

That gap is the central reverse-engineering problem of the project, and it's genuinely solvable (§7),
but it means: **the proprietary command sets will come from capturing a real Honda-capable tool
talking to your actual car, not from a datasheet.**

---

## 4. Feasibility by goal — honest assessment

| Goal | Verdict | Confidence | Basis |
|---|---|---|---|
| Generic OBD-II DTCs + live data (engine) | **Straightforward** | HIGH | Standard ISO 9141-2; open-source libs exist (iwanders/OBD9141, MIT) |
| Honda-mode engine DTCs + Honda "snapshot" fast data | **Doable, needs RE** | MEDIUM | Frame format known; commands via capture |
| ABS / SRS / EPS / immobilizer DTC read + clear | **Doable, needs RE + both lines** | MEDIUM | DIAG-H line + Honda framing; commands via capture |
| Blink-code fallback (SCS) for all units, no protocol at all | **Easy, independent path** | HIGH | Factory procedure: ground pin 9, count indicator flashes |
| ECM reset / idle learn / actuator tests | **Doable, needs RE** | MEDIUM | Dealer functions; command bytes via capture |
| **Immobilizer key registration** | **Gated** | — | Needs a per-car code issued by Honda even to dealers; sensitive — separate phase, your call |
| **Airbag (SRS) memory writes** | **Gated** | — | Safety-critical; mishandling can deploy/disable airbags — separate phase |
| **ECM/PCM firmware read-out** | **Hard, bench route** | MEDIUM | MCU is OKI nX-8/500S; likely needs bench access to a spare ECM, not the connector |
| **ECM/PCM reflash** | **Hard, dealer-locked** | MEDIUM/LOW | Factory *can* reflash over the connector; no aftermarket tool cracks the stock D17/K20A3 entry — Hondata/KTuner physically modify the ECU instead. This is why it's a stretch goal. |

A useful safety net: **the SCS blink-code method needs no protocol at all.** Grounding pin 9 makes the
MIL, ABS, SRS and EPS indicators flash their stored codes, by factory design. So even before any
protocol is cracked, the device can read codes from every Tier-A unit on day one by timing indicator
blinks — a guaranteed-to-work baseline we build in first.

---

## 5. Hardware design

**Philosophy:** we don't yet know every electrical detail of DIAG-H, WEN, and the exact pulse timings
Honda uses. So instead of hard-wiring assumptions, build **flexible, protected channels** and measure
the car in Phase 0. Concretely:

- **Microcontroller: Raspberry Pi Pico 2 W (RP2350).** ~US$7, you already have the GNU Arm toolchain
  installed. Decisive feature: its **PIO** blocks generate and timestamp bit-level signals exactly —
  essential for the 5-baud init, Honda's ~70 ms wake-up pulses, and for passively sniffing a dealer
  tool with microsecond timestamps. 2 UARTs + USB + Wi-Fi + Bluetooth. (Alternative: ESP32-S3, but
  separate toolchain and no PIO.)
- **K-line channel (pin 7): ST L9637D transceiver.** Chosen specifically because it has **no "TXD
  dominant timeout"** — many LIN/K-line chips force the line high after tens of ms, which would break
  5-baud init and long wake-up pulses. Active part, ~US$1, 3.3 V-logic-friendly. Second choice: TI
  TLIN1027-Q1.
- **DIAG-H channel (pin 14): a separate, independently-powered single-wire channel** with selectable
  pull-up (off / 5 V / 12 V), programmable receive threshold, and ADC read-back — because its exact
  levels are the biggest electrical unknown. Built so it can be mapped to pin 7 *or* pin 14, giving
  two interchangeable serial channels.
- **SCS switch (pin 9):** a protected open-drain low-side switch with current limit and voltage
  read-back — grounds the line on command, reads it back for the blink-code reader.
- **WEN (pin 12):** a switchable, current-limited 5 V driver, populated but used only in the reflash
  research phase.
- **Power (pin 16):** pin 16 is *permanent* battery, so the device must not flatten the battery.
  Reverse-polarity protection, a 33 V-class TVS + fuse for load-dump/transients (design to ISO 16750-2),
  a wide-input buck, and **sub-milliamp sleep** with a hard power switch.
- **Sniffer mode:** a high-impedance tap so the device can sit *between* a Honda-capable tool and the
  car and log every byte with timestamps — the core RE instrument (§7). Must **not** add a second 510 Ω
  pull-up (two in parallel corrupt the bus), so the pull-up is switchable.
- **Logic-analyzer test points** on pins 7/9/12/14 brought to a header, so captures can be saved in a
  format the REA toolkit ingests.

**Development path before any custom PCB exists:** start on a breadboard — Pico 2 W + L9637D + a full
16-pin connector cable — plus a cheap logic analyzer and an FTDI "KKL" cable for first K-line
experiments. No board spin needed to begin protocol work. (BOM ~US$75–130 for the breadboard rig;
~US$25–35/board in parts for a custom PCB later.)

**No existing open board covers Honda's pins.** The closest, Macchina M2 and OpenDiag, miss the SCS
ground and the pin-14 serial channel — so this is genuinely a custom design, though we borrow proven
sub-circuits (ELM327 reference K-line driver, etc.).

---

## 6. Software / firmware architecture

Layered, with a clean split between what *must* be timed precisely (firmware) and protocol logic
(host):

```
  PC / phone host (C#/.NET — your language)
    ├─ ELM327-compatible personality  → lets Torque/OBD Fusion work for the generic OBD part
    ├─ Honda service layer            → DTC/live-data/function-test logic, built from captured tables
    └─ one channel interface, two back-ends: our device, OR a commercial J2534 interface
         │  (binary framing over USB CDC / BLE / Wi-Fi)
         ▼
  Pico 2 W firmware (C, GNU Arm)
    ├─ transport (USB/BLE/Wi-Fi)
    ├─ service: J2534-shaped primitives (init, send/recv, timing params) + a raw pulse primitive
    ├─ protocol engines: (a) ISO 9141-2 legislated OBD   (b) Honda K-line mode   (c) DIAG-H
    ├─ passive sniff mode (timestamped byte logger)
    └─ physical drivers: K-line, DIAG-H, SCS, WEN  (PIO-timed)
```

- **Model the link layer on the SAE J2534 primitives** (configurable baud, parity, checksum-off,
  5-baud and fast init, arbitrary wake-up pulses). J2534 is a published, complete model for exactly
  this, and designing to it means our firmware can later *also* present a J2534 interface that Honda's
  own HDS software could drive — which doubles as the best capture mechanism (§7).
- **Reusable prior art:** `iwanders/OBD9141` (MIT) for the ISO 9141-2 layer; `HondaECU/eculib` (GPL-3.0)
  and `kerpz/ArduinoHondaOBD` (no license) as *protocol references* for Honda framing. ⚠️ **License
  decision needed early** (§11): the best Honda references are GPL-3.0, so copying their code forces
  GPL on the project; using only the documented facts does not.
- **Honda-specific DTC/PID tables are not in any open database** — they get built from captures and the
  factory manual. Generic DTC text is freely available (MIT).
- **Testing without the car:** build a Honda ECM emulator from captured request/response tables so the
  firmware can be regression-tested on the bench. A junkyard ECM (~`37820-PLM-xxx`) powered on the
  bench is the highest-fidelity test target and only needs power, not a running car.

---

## 7. The reverse-engineering track (using REA + Ghidra)

This is where your chosen tool, **REA** (`github.com/morluto/rea`, npm `rea-agents`), comes in. What I
found about how it actually fits (full detail in [`09-rea-toolkit-fit.md`](research/09-rea-toolkit-fit.md)):

- REA is an MIT-licensed MCP server/CLI that drives back-ends (Ghidra, JADX, TShark, Binwalk…) behind
  an "evidence" contract. It does **not** install those back-ends for you.
- **Prerequisite: upgrade Node.** REA 6.9.0 needs Node ≥22.23.4; your PC has 22.16.0, so it refuses to
  run until upgraded (same 22.x LTS line is fine).
- **Where REA helps directly (on Windows):** decompiling Honda's **HDS dealer software** and **J2534
  DLLs** (32-bit x86 PE — supported), and Android apps that do Honda diagnostics (JADX lane). These are
  the richest targets — HDS *contains* the protocol tables and command logic we need.
- **Where REA has a gap:** the **ECU firmware**. The ECM is an **OKI nX-8/500S** microcontroller, and
  *neither* REA *nor* stock Ghidra has a processor module for it — this is the single biggest RE
  obstacle, and building an nX-8/500S disassembler is the highest-value (and hardest) RE task if we go
  after firmware. (Note: stock Ghidra *does* handle SuperH, so if your specific ECM turns out SuperH-based
  it's easier — another reason Phase 0's chip ID matters.)
- **Signal/packet lanes (TShark, sigrok, Binwalk) are Linux-only** in REA → run them in WSL2. sigrok has
  no K-line decoder, so we write a small framing parser over the UART byte stream.

**The capture strategy** — this is how the unknown Honda commands get learned, in order of cost:

1. **Query the published HDS-derived database** (an open GPL project decoded protocol metadata from HDS
   3.105.12 — per-system protocol, DIAG-H pin, DTC bit-maps). Free, immediate, needs verification on a
   real bus.
2. **Buy the two SAE documents** that specify the Honda lines: J2534-2/4 (DIAG-H) and J2809 (ABS/VSA).
   ~US$70–200 total, legitimate, authoritative on the physical/framing layer.
3. **Legitimately rent HDS** (US one-day pass ~US$60 in 2026) and **log it talking to your car** via a
   J2534 logging shim, or let our own device sit in the middle as a sniffer. This is the decisive
   step — it captures the exact byte sequences for every function, on *your* car.
4. **Reverse-engineer the HDS binaries / a clone-cable firmware** with REA+Ghidra to fill gaps and
   understand the reflash path.

⚠️ **Repo hygiene (your global rule):** REA evidence bundles, Ghidra projects, and any proprietary
HDS material must be kept **out of** the public `HondaOBD` repo. We'll store them in a separate local
location (meta-repo or just outside the tree), per your CLAUDE.md policy.

---

## 8. Phased roadmap

**Phase 0 — Identify the car & measure the connector (no build).**
Record full VIN + ECM/PCM part number; probe which connector pins are populated and their idle
voltages (expect ~12 V on 7, ~5 V on 9 and 14, ~0 V on 12); confirm US vs Euro vs JDM wiring. This
decides every branch below. Deliverable: a one-page "your exact car" spec.

**Phase 1 — Generic OBD-II, read-only.** Breadboard Pico 2 W + L9637D on pin 7. Implement ISO 9141-2,
read engine DTCs + live data. Proves the hardware and gives immediate value. (Low risk, high confidence.)

**Phase 2 — Blink-code reader (all Tier-A units).** Drive the SCS line (pin 9), read ABS/SRS/EPS/engine
codes by timing indicator blinks. No protocol needed — guaranteed coverage of every Tier-A unit's codes.

**Phase 3 — Sniffer + capture.** Add passive sniff mode; capture a Honda-capable tool (rented HDS, or a
clone cable) talking to the car on pins 7 and 14; decode with REA/WSL2. Build the command tables.

**Phase 4 — Honda mode, read.** Implement Honda K-line + DIAG-H engines from the captured tables: full
DTCs/freeze/live-data/snapshot for ECM, ABS, SRS, EPS, immobilizer status.

**Phase 5 — Honda mode, safe writes.** Clear DTCs, ECM reset, idle learn, actuator/function tests. Custom
PCB spin around here.

**Phase 6 — Gated functions (your explicit go-ahead, separately).** Immobilizer key registration, SRS
writes — sensitive, credential- and safety-gated; researched and scoped on their own.

**Phase 7 — Firmware read/reflash (stretch, research).** Bench ECM, chip ID, firmware dump, nX-8/500S
tooling, reflash path. Open-ended.

---

## 9. Risks & safety

- **Airbags (SRS):** disconnect battery negative and wait ≥3 min before any SRS work; use only a
  ≤10 mA meter on squib circuits; never probe/jumper SRS terminals. Mishandling can deploy an airbag or
  disable occupant protection. The device's pin-14 and pin-9 drivers are 5 V-safe and current-limited
  specifically because they share wires with the SRS unit.
- **Bricking the ECM on reflash:** an interrupted write damages the ECM; always on a battery
  maintainer, never during cranking. Keep a spare ECM.
- **Immobilizer coupling:** the ECM stores the immobilizer pairing; swapping or bench-running an ECM
  means the car won't start until re-registered (needs a programmed key + Honda-issued code). Plan ECM
  swaps around this.
- **Battery drain:** pin 16 is always live → strict sleep-current budget + physical power switch.
- **Electrical faults:** reverse-polarity, load-dump and transient protection on the vehicle supply.

---

## 10. Legality & ethics

This is your own car; reading/clearing codes and diagnosing it is squarely right-to-repair, and
capturing a tool you've legitimately licensed talking to your own vehicle is a standard
interoperability activity. Two areas get handled with care: **immobilizer/key functions** (anti-theft —
dealer functions need Honda-issued per-car credentials, which we respect rather than circumvent) and
**airbag writes** (safety). REA's own disclaimer draws the same line (analysis of software you're
authorized to examine; not circumventing protection measures), and we keep proprietary dealer material
out of the public repo.

---

## 11. Decisions I need from you

1. **Your exact car** — VIN + ECM part number if you have them, and market (US / Europe / Japan),
   engine, and auto/manual. This re-aims the whole plan. If you don't know yet, Phase 0 finds out.
2. **How far do you actually want to go?** (a) read-everything diagnostics + safe service functions is
   the solid, high-confidence target; (b) + immobilizer/key + airbag writes (gated, sensitive);
   (c) + ECM reflash (hard, open-ended research). This sets scope.
3. **License for the repo** — GPL-3.0 (lets us reuse HondaECU/freediag code directly) vs permissive
   MIT (facts-only from those references, more freedom later). Needs deciding before we write code.
4. **Install REA now?** It upgrades Node and writes to your `~/.claude.json` + skills. I'd do a
   `--dry-run` first and keep its artifacts out of the repo. OK to proceed when we reach Phase 3?
5. **Budget appetite** for the legitimate shortcuts: ~US$60 HDS day-pass + ~US$70–200 SAE docs would
   save enormous RE effort. Worth it to you, or purely DIY-from-scratch?
```
```

*Research sources and full detail: see the nine files under [`docs/research/`](research/).*
