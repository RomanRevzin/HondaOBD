# HondaOBD — Build Plan

A device that plugs into the 16-pin OBD2 connector of a **2002 Honda Civic Si (EP3, K20A3)** and
reads/writes as many diagnostic codes and functions as the car physically allows — up to and
including ECM reflash. A right-to-repair / owner-diagnostics / enthusiast-tuning project on your
own vehicle.

Status: **research complete, plan drafted, nothing built yet.** A focused EP3/K20A3-specific research
pass is in progress and will refine the Si-specific sections. Detailed research notes with sources and
confidence tags are in [`docs/research/`](research/).

---

## 0. Decisions locked (2026-11-14)

| # | Decision | Choice |
|---|---|---|
| Car | Exact vehicle | **2002 Civic Si — US EP3 hatchback, K20A3, 5-speed manual, ECU `37820-PNF-A05`.** (UK-built; immobilizer confirmed ECM-held, same as the D17 — see §12.) |
| Scope | How far to go | **Option C — "all the way":** diagnostics + safe service writes + gated immobilizer/SRS + **ECM reflash**. Gated functions still proceed with care and your explicit go-ahead. |
| Budget | Spending | **Zero.** No HDS day-pass, no SAE spec purchases. Rely on free/open resources (see §7). Any unavoidable paid shortcut gets flagged for you to decide, not assumed. |
| License | Repo license | **GPL-3.0** — lets us reuse the best K-series/Honda references (HondaECU, freediag, both GPL-3.0). *My call per your "do what is needed"; say the word if you'd rather go permissive (MIT).* |
| REA | When to install | **Deferred to the reverse-engineering phase (Phase 3+).** It needs a Node upgrade (22.16 → ≥22.23) and writes to `~/.claude.json`; not needed at Phase 0/1. |

**Why the Si is the right car for option C:** the K20A3 is the K-series tuner platform, with a large
open ecosystem (Hondata, KTuner, RomRaider-style tooling). Reflash is far more tractable here than on
the 1.7 sedans — though the *stock* ECU's connector-reflash entry still appears dealer-locked (Hondata
K-Pro adds a daughterboard rather than cracking it), and the ECU's OKI nX-8/500S microcontroller has
**no off-the-shelf disassembler** — though the EP3 deep-dive since found a concrete free path to build
one (free ISA docs + a template to fork; see §12.E). Both points are now confirmed; §12 carries the
detail and several other refinements from that pass.

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
- On the Si, **every diagnosable unit sits on the single K-line (pin 7)** — see §3, revised by the
  EP3-specific research (§12.A). One transceiver reaches them all; simpler hardware than first thought.

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

## 3. The finding that shapes the hardware: one K-line reaches every unit

From Honda's factory connector views and splice diagrams (HIGH confidence, corroborated by three of
the nine research threads), the 2002 US Civic connector has exactly **seven populated pins**:

| Pin | Wire | Signal | Goes to | Notes |
|---|---|---|---|---|
| 4 | BLK | Chassis ground | Body | — |
| 5 | BRN/YEL | Signal ground | ECM/PCM E3 | — |
| **7** | LT BLU | **K-line** (~12 V idle) | ECM E23 — **and ABS, SRS, EPS** | ISO 9141-2 OBD *and* the Honda protocol for every diagnosable unit |
| **9** | BRN | **SCS** service line (~5 V idle) | ECM, SRS, ABS | Ground it → units blink stored codes |
| **12** | RED/WHT | **WEN** write-enable | ECM/PCM E30 | Dealer tool reportedly puts +5 V here to reflash |
| 14 | GRY | ~5 V immobilizer line | immobilizer unit (role unproven) | *Not* a confirmed serial bus; passive-sniff point only (see §12.A) |
| 16 | WHT/RED | Battery +12 V (perm.) | via BACK UP fuse | ~4 A max, always live |

**The key consequence (revised by the EP3-specific research — see §12.A):** on the Si, the engine,
**ABS, airbag (SRS) and EPS all share the single K-line on pin 7.** The EP3 service manual's own DLC
continuity test ties pin 7 to the ECM, ABS, SRS and EPS wiring, and Honda's dealer job aid lists the
2001–05 Civic (Si included) as K-line only. So **one 12 V K-line transceiver on pin 7 reaches every
unit the dealer tool can see** — you still can't do it with a generic dongle (that only does the
*legislated* subset), but you don't need a second serial channel. Pin 14 is a separate ~5 V
immobilizer-related line whose exact role isn't proven and is *not* a confirmed diagnostic bus. (The
earlier "two serial lines" picture came from D17 wiring and from conflating Honda's *protocol* name
"DIAG-H" with the physical pin-14 wire — §12.A untangles this.) To cover Tier A you drive the pin-7
K-line, switch the SCS line (pin 9) for blink codes, and keep the WEN line (pin 12) for the reflash
phase.

⚠️ Caution that still holds: **don't drive 12 V onto pin 14.** It's a ~5 V line tied to the immobilizer,
so the device treats pin 14 as a *passive, high-impedance sniff point only* (never driven) until its
role is confirmed by probing the actual car in Phase 0.

**Protocols, by confidence:**

- **Legislated OBD-II = ISO 9141-2** on pin 7 (5-baud init, address 0x33, 10.4 kbit/s). HIGH. This is
  the safe, fully-documented first milestone — standard, non-proprietary, same as thousands of other
  pre-2004 cars.
- **Honda "mode"** (the dealer protocol) runs on the same pin-7 K-line for every diagnosable unit. The
  *frame format* is now **confirmed on real cars**: `[command, length, data…, checksum]` where the whole
  frame sums to 0 mod 256, with command codes readout `0xA0`, clear `0xA1`, inforead `0xA2`, wake `0xFE`
  (from the HDS decompile, §12.B), corroborated by actual captures — a K-engine ECU (`FE 04 72 8C` wake,
  `72 05 71 pp cc` table read, reply header `0x02`) and a CR-V airbag unit (`60 05 70 02 29`). The
  Honda-mode baud is contested (9600 vs 10400) so firmware makes it selectable. What's still unverified
  on *this* car is the per-unit reply semantics and the exact request bytes for some functions. MEDIUM.

So the proprietary command knowledge now comes largely from an **open GPL decompile of HDS** (§12.B),
verified against your own car with the device's sniffer — not purely from scratch as first feared.

---

## 4. Feasibility by goal — honest assessment

| Goal | Verdict | Confidence | Basis |
|---|---|---|---|
| Generic OBD-II DTCs + live data (engine) | **Straightforward** | HIGH | Standard ISO 9141-2; open-source libs exist (iwanders/OBD9141, MIT) |
| Honda-mode engine DTCs + Honda "snapshot" fast data | **Doable, needs RE** | MEDIUM | Frame format known; commands via capture |
| ABS / SRS / EPS / immobilizer DTC read + clear | **Doable** | MEDIUM | All on the pin-7 K-line; Honda framing + the GPL HDS tables (§12.B); SCS blink codes as a $0 fallback |
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
- **Pin-14 channel: a passive, high-impedance ~5 V sniff input only** (never driven). The EP3 research
  shows the diagnosable units are all on the pin-7 K-line, so the originally-planned driven "DIAG-H
  channel" is cut down to a listen-only tap to observe immobilizer-related activity; its role is
  confirmed on the car in Phase 0 before anything more is built for it.
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
ground and a clean passive-sniff tap — so this is genuinely a custom design, though we borrow proven
sub-circuits (the L9637D K-line driver, and the `jared52005/Monitor` FT232+MC33660 passive tap as a
sniffer precedent; §12.I).

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
    ├─ protocol engines: (a) ISO 9141-2 legislated OBD   (b) Honda K-line mode (all units)
    ├─ passive sniff mode (timestamped byte logger)
    └─ physical drivers: K-line (pin 7), SCS (pin 9), WEN (pin 12), pin-14 sniff  (PIO-timed)
```

- **Model the link layer on the SAE J2534 primitives** (configurable baud, parity, checksum-off,
  5-baud and fast init, arbitrary wake-up pulses). J2534 is a published, complete model for exactly
  this, and designing to it means our firmware can later *also* present a J2534 interface that Honda's
  own HDS software could drive — which doubles as the best capture mechanism (§7).
- **Reusable prior art:** `iwanders/OBD9141` (MIT) for the ISO 9141-2 layer; and the big one —
  **`huhwhatbruh/HondaDiagnosticSuite` (GPL-3.0)**, a decompile of Honda's HDS that ships ~36,000 DTCs,
  ~25,882 live-signal definitions *with Honda's scaling formulas*, 182 systems, and working Honda-protocol
  code. Our GPL-3.0 license (§0) lets the host build directly on it (§12.B). `HondaECU/eculib` (GPL-3.0)
  and `kerpz/ArduinoHondaOBD` are further protocol references.
- **Honda-specific DTC/PID tables now have an open source** — HondaDiagnosticSuite's database (above).
  Captures + the factory manual verify it on the car and fill the few request bytes it marks unconfirmed.
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

**The capture strategy (zero-budget first).** In order of cost:

1. **Reuse `huhwhatbruh/HondaDiagnosticSuite` (GPL-3.0) — $0.** It already decompiled HDS; its database
   and Honda-protocol code give most of the DTC/PID/command knowledge up front (§12.B). This replaces
   the earlier "buy SAE docs / rent HDS" idea for the *read* path.
2. **Verify on your car with the device's own sniffer + generic OBD + SCS blink — $0.** Confirms the GPL
   tables against your actual ECU/ABS/SRS/EPS and nails down the request bytes the open sources mark
   "unconfirmed."
3. **Only if you later want immobilizer/airbag writes or the reflash sequence — one flagged spend
   (~$35–40):** a ~$25–30 GODIAG GD101 clone interface + a ~$10 one-day i-HDS subscription, captured with
   our own passive tap on your car. This is the sole near-free way past Honda's deliberately-withheld
   SecurityAccess key (§12.F–G). Your call if/when — everything above is done first at $0.
4. **Reverse-engineer the HDS binaries and a clone J2534 DLL with REA+Ghidra — $0**, to fill remaining
   gaps; the OKI firmware/reflash route is §12.D–E.

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

**Phase 3 — Reuse + verify (mostly $0).** Import the GPL HDS database; add passive sniff mode; confirm
the tables on the car via generic OBD + SCS blink + the device's pin-7 sniffer. The optional ~$35–40
HDS-capture (§7 step 3, §12.G) slots in here only if/when you want the write/reflash sequences.

**Phase 4 — Honda mode, read.** Implement the Honda K-line engine from the tables: full
DTCs/freeze/live-data/snapshot for ECM, ABS, SRS, EPS and immobilizer status, with Honda's scaling.

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
  disable occupant protection. The SRS unit shares the pin-7 K-line and the pin-9 SCS line, so those
  channels are 5 V-safe and current-limited, and pin 14 is listen-only — the device never drives a line
  into the SRS unit.
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

## 11. Decisions — resolved

All five opening decisions are made (see **§0**): car = 2002 Civic Si EP3/K20A3; scope = C (all the
way, incl. reflash); budget = zero; license = GPL-3.0; REA install deferred to the RE phase. The
zero-budget choice reshapes §7 — see the free capture strategy there; the EP3 research pass is working
out exactly how far "free" gets us before any spend would be needed, and that limit will be stated
plainly.

**The one thing still outstanding** is physical, not a decision — **Phase 0 data from the actual car**,
whenever you're next at it:

- the full **VIN** and the **ECM part number** (sticker on the ECU box behind the glovebox — expect
  `37820-PNF-…`);
- a quick probe of the OBD connector: which pins have wires, and the idle voltage on pins 7, 9, 12, 14
  (expect ~12 V on 7, ~5 V on 9 and 14, ~0 V on 12) with the ignition on.

None of this blocks the design, firmware scaffolding, or reverse-engineering prep — I'll keep moving on
all of those. It just confirms the wiring before any hardware is finalized.
---

## 12. Update from the EP3/K20A3 deep-dive (2026-11-14)

A focused five-agent pass on the Si refined the plan. Full notes:
[`ep3-01`](research/ep3-01-si-electrical-and-units.md) … [`ep3-05`](research/ep3-05-rea-ghidra-workflow.md).
Headlines, highest-impact first:

**A. One K-line, not two (hardware simplification).** On the Si the engine, ABS, airbag and EPS are all
on the pin-7 K-line; the EP3 manual's own DLC continuity test proves it. Pin 14 is a ~5 V
immobilizer-related line, *not* a confirmed diagnostic bus — the round-1 "two serial lines" idea came
from D17 wiring and from the fact that Honda's *protocol family* is itself named "DIAG-H" (a name, not
the pin). Net: the driven second channel is cut; one K-line transceiver + the SCS switch + a listen-only
pin-14 tap covers everything. This supersedes the original framing in §3/§5.

**B. Most of the HDS reverse-engineering is already done — and it's GPL-3.0.** `huhwhatbruh/Honda​
DiagnosticSuite` decompiled Honda's HDS diagnostic bytecode and ships Honda's own database: ~36,000 DTCs
(with descriptions/repair text), ~25,882 live-signal definitions **with Honda's exact scaling formulas**,
182 systems, 3,607 unit converters, plus readable code for ISO 9141-2, KWP2000 and Honda's proprietary
serial protocol. GPL-3.0 matches our license, so the host builds on it rather than hand-authoring Honda's
tables. This removes most of the risk from the "read" goals. Caveats: it was never run on a real car
(reply framing needs on-car verification), and the SecurityAccess seed→key is deliberately omitted.

**C. Honda's car protocol frame is confirmed on real cars.** `[command, length, data…, checksum]`,
whole frame sums to 0 mod 256; commands readout `0xA0` / clear `0xA1` / inforead `0xA2` / wake `0xFE`.
Corroborated by an actual K-engine capture (`FE 04 72 8C`; `72 05 71 pp cc`; reply header `0x02`) and a
CR-V airbag capture (`60 05 70 02 29`). Generic OBD is proven on a K20 (ISO 9141-2, 10400 baud, ECU
address `0x11`). Honda-mode baud is contested (9600 vs 10400) → firmware makes it selectable, 9600 first.

**D. Reflash path, hardened (still the hard part).** Honda itself can reflash the EP3 over the K-line
(write-guarded). *No* tool at any price reflashes the **stock** EP3 ECU over the connector — even paid
PCMflash only "virtual-reads"/writes other OKI cars and doesn't list the EP3; the tuner world uses the
Hondata K-Pro daughterboard. The ECU is an OKI **ML66Q592** (nX-8/500S core, ~192 KB flash, 128-pin QFP,
with an EA pin usable for a bench ROM dump). Free reflash-research route: bench-dump a junkyard
`37820-PNF`, build a disassembler, capture a Honda write session.

**E. The disassembler blocker is now cracked open.** No off-the-shelf processor module exists for the
OKI nX-8/500S, **but** the ISA and flash manuals are *free* on the pgmfi wiki (`nx_800.500s_core.pdf`,
`msm66591_ml66592_um.pdf`, `msm66q591_um.pdf`), and `VIRUXE/ghidra-oki66207` (public-domain) is a
validated Ghidra SLEIGH module for the older nX-8/200 core to fork. So building an nX-8/500S Ghidra
module is a concrete (if labor-heavy) **free** task, not a dead end. This is where REA+Ghidra earn their
place; note stock Ghidra, not REA, loads the ECU image.

**F. What $0 actually buys — and the one wall.** A fully capable **read** tool is buildable and verified
on the car for zero money: every DTC in every reachable unit (generic OBD on the engine + SCS blink for
ABS/SRS/EPS + Honda-mode reads), live data with correct names/formulas (GPL DB), freeze frames, service
identification. Clearing codes and the service learns are very likely reachable too, but a few request
bytes are "unconfirmed" in the open sources, so they get verified on the car. Progress only truly stalls
at two gates, both behind Honda's withheld SecurityAccess key: (1) immobilizer key registration and
airbag writes; (2) stock-ECU read/reflash.

**G. The single flagged spend (your call).** The only near-free way through those two gates is to capture
a genuine HDS session on your own car: ~$25–30 GODIAG GD101 clone + ~$10 one-day i-HDS subscription,
sniffed with the device's own passive tap (~$35–40 total). Per your "no money" rule this stays optional
and flagged — everything in (F) is done first at $0; this is only if/when you want the write/reflash
sequences. (Legally this sits in the DMCA interoperability + owner vehicle-repair exemption.)

**H. Si-specific service functions.** Implement idle learn **and** crank/CKP pattern clear+learn (order
after a full clear: idle learn → CKP learn → test drive), plus EPS torque-sensor neutral learn (needs
ambient > 20 °C). Do **not** implement ECM VIN read/write — the 2002–2004 Si ECU doesn't store a VIN.
ECU is `37820-PNF-A05`; never cross-flash RSX (PND/PNE) calibrations.

**I. Capture hardware precedent.** `jared52005/Monitor` (MIT; FT232 + MC33660 passive K-line tap →
Wireshark) is a direct precedent for the sniffer. Clean capture point = the L9637D's logic-side RX pin
(a 0/3.3 V copy of the bus) into a spare Pico UART + a logic-analyzer header. REA's packet/signal lanes
are Linux-only (WSL2) with no K-line decoder, so captures are converted with a small dissector
(`text2pcap` + a USER link type, or Monitor's dummy-IP wrapper).

**Environment checklist for the RE phase (all free):** upgrade Node 22.16 → 22.23.5 → install WSL2 +
Ubuntu (tshark, sigrok-cli) → JDK 21 → Ghidra 12.1.4 → fork `ghidra-oki66207` + fetch the free OKI PDFs
→ `rea setup --dry-run`, then setup. RE artifacts (evidence bundles, Ghidra projects, HDS data, dumps)
stay out of this public repo — the `.gitignore` already enforces that; a separate meta-repo versions them.

---

*Research sources and full detail: see the fourteen files under [`docs/research/`](research/) — nine
from the first broad pass and five EP3/K20A3-specific (`ep3-01`…`ep3-05`).*
