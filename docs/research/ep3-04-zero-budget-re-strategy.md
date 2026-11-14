# EP3 Zero-Budget Reverse-Engineering Strategy: Learning the Honda Protocol and Reflash Path with No Money and No Dealer Tool

Research dimension 04 for the 2002 US-market Honda Civic Si (EP3, K20A3, PNF ECU) owner-diagnostics / right-to-repair build. Date: 2026-11-14.

Scope: how to turn the unknowns (exact Honda-mode commands for ECM/ABS/SRS/EPS/immobilizer, the reflash protocol) into knowns, spending **zero money**, with no HDS subscription and no SAE standards purchased, and with no dealer tool available to sniff.

Confidence tags: **HIGH** = primary source or two independent sources agree; **MEDIUM** = one credible source; **LOW** = single forum post / inference / conflicting.

---

## Summary

The single most important finding is that **the hard reverse-engineering has largely already been done and published under GPL-3.0**. A project called **HondaDiagnosticSuite** (GitHub `huhwhatbruh`) decompiled HDS's internal diagnostic bytecode "without installing Windows" and extracted Honda's own database: **36,000 DTCs with Honda descriptions and repair text, 25,882 signal/parameter definitions each with Honda's conversion formula, 182 systems, 3,607 unit converters**, validated against Honda's own interpreter on 127,313 inputs with 0 mismatches. The database ships in the source tree. This means the owner does **not** need to buy or re-RE HDS to obtain the DTC / parameter / system metadata — the cheapest-effort free source is to reuse this project (HIGH).

The Honda K-line frame format is now corroborated on **real cars**, not just motorcycles: a published capture of a Honda K-engine ECM (`Honda_Keihin_KLine_Protocol`) shows wake-up `FE 04 72 8C` and table reads `72 05 71 pp cc`, and a Russian capture of a Honda CR-V SRS shows `60 05 70 02 29` — both obeying the two's-complement "all bytes sum to 0 mod 256" checksum. The HDS-derived `diagh.rs` in HondaDiagnosticSuite independently encodes the same frame and the DIAG-H command set (READOUT `0xA0`, CLEAR `0xA1`, INFOREAD `0xA2`, WAKE `0xFE`).

A fully capable **read-only** tool — every DTC in every reachable module, live data, service identification — is buildable and verifiable on the car for **free**. Progress genuinely stalls at two gates that every open source avoids: **SecurityAccess (seed→key)**, which the HDS-decompiling author deliberately withheld, and **reading/reflashing the stock PNF/OKI ECM calibration**, for which no public tool exists (free or commercial) and no CPU disassembler module exists. The tuning world sidesteps this with the Hondata K-Pro daughterboard (paid hardware), not a DLC reflash.

The only genuinely useful money would be ~$35-40 (a ~$25-30 GODIAG GD101 clone + a ~$10 one-day i-HDS subscription) to self-capture ground-truth HDS sessions with the owner's own passive sniffer. Legally, working on one's own car for diagnosis/repair/modification sits inside the DMCA §1201(f) interoperability exception and the triennial vehicle-repair exemption (37 CFR §201.40).

---

## Findings

### Q1. Obtaining HDS / i-HDS legitimately for free to ANALYSE

**The key realisation: someone already did the extraction and published it, so re-RE'ing HDS is optional.** HondaDiagnosticSuite's author states: *"I took HDS apart piece by piece, without installing Windows. Honda's diagnostic logic is bytecode. I wrote a decompiler for it: 139 modules, 10,021 functions"* and ran every in-scope function next to Honda's own interpreter on 127,313 inputs with 9,660 comparisons and 0 mismatches (HIGH — [repo README](https://github.com/huhwhatbruh/HondaDiagnosticSuite)). For the metadata the owner wants (commands, DTCs, parameters, systems), this is the cheapest-effort free source; REA/Ghidra on HDS is only needed to *verify or extend* it.

What is free vs gated in Honda's own channel:

| Item | Status | Confidence |
|---|---|---|
| Honda SAE J2534 reprogramming software | Historically offered "FREE of charge" (EPA/CARB mandate); now folded into the i-HDS suite and described as available "with an i-HDS subscription" | MEDIUM ([techinfo J2534 page](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_J2534.htm) via search; direct fetch blocked by a TLS cert-chain error) |
| i-HDS installer / Honda Download Manager (HDM) | Downloadable; the paid subscription gates *communication/use against a car*, not necessarily the install itself | MEDIUM (conflicting: Honda pages say subscription required; a forum user says "It's still free too") |
| Full diagnostics + reprogramming use | Paid subscription: 1-day (~$10), 30-day, 365-day tiers; "Software is non-refundable" | HIGH ([store.honda.com 1-day](https://store.honda.com/1-day-i-hds-subscription)) |
| VCI hardware | Official MVCI is **$2,437**, but i-HDS runs on any J2534-1/-2 pass-thru, including ~$25-30 clones | HIGH (Honda tools page; GD101 listings) |

For the owner's **analysis** goal (get the HDS binaries + databases onto the PC so a disassembler can read the protocol logic and tables): the installer/HDM is obtainable without buying a subscription, and analysing software one has lawfully obtained, for interoperability with one's own car, is defensible (see Q6). But in practice the extracted artefacts from HondaDiagnosticSuite make a fresh HDS teardown unnecessary unless verifying a specific module.

Note: the HDS "help" content (human-readable DTC and parameter text) is also already extracted and published separately as HTML — see `alexey-bass/honda-hds-help` in Q2.

### Q2. Published HDS-derived and decoded-HDS resources

**These remove most of the "RE HDS from scratch" burden.**

#### HondaDiagnosticSuite (`huhwhatbruh`) — GPL-3.0-or-later — the primary resource (HIGH)

Extracted database (in `hondascan/data/hondascan.db`, SQLite, shipped in source so "a clean clone builds and runs"; release builds are encrypted via `release-key.txt`):

| Content | Count |
|---|---|
| Honda DTCs (with descriptions + repair-help text; stored/pending/history) | 36,000 |
| Signal/parameter definitions (each with Honda's own conversion formula) | 25,882 |
| Systems Honda knows about | 182 |
| Unit converters (checked against Honda's conversion code) | 3,607 |
| Decompiled modules / functions | 139 / 10,021 |

README notes the DB *"is derived from Honda's diagnostic data and included for interoperability"* and the GPL *"covers this project's code, not Honda's rights in the original."* Confirmed on a 2015 Accord (K24W1, CVT) over CAN; 2019/2021 Civics over ELM327 with partial results; **K-line, ELM327, slcan, SocketCAN paths are simulator-only; actuator tests and writing functions have never run on a car** (HIGH — [README](https://github.com/huhwhatbruh/HondaDiagnosticSuite)).

Protocol source files relevant to the EP3 (file reads, HIGH that the files exist and say this; the *data* within is HDS-derived and mostly car-unvalidated — see per-file notes):

| File | Purpose |
|---|---|
| `honda.rs` | Honda VCAN (CAN) diagnostics — **newer cars only**, not the EP3 K-line |
| `diagh.rs`, `diagh_dtc.rs`, `transport/diagh_serial.rs`, `probes/diagh_session.rs` | **DIAG-H (pin 14, 5V) — directly relevant to the EP3 SRS/EPS** |
| `kwp.rs`, `obd9141.rs`, `klineservice.rs`, `transport/elm_kline.rs` | K-line (pin 7): KWP2000 for "VKWP" body systems + generic OBD-II |
| `seedkey.rs` | SecurityAccess — **placeholder only, no key** |
| `probes/abs_probe.rs` | ABS probing |
| `db.rs`, `dbcrypt.rs`, `bin/build-db.rs`, `bin/encrypt-db.rs` | DB build + (release) encryption |

`diagh.rs` — "Honda's own K-line message format" (MEDIUM; header says the code was **"never run on a car or a bus"**, so these are HDS-decompiled, not verified):
- Frame: `[command, length, data..., checksum]`; `length` counts every byte; checksum makes all bytes sum to 0 mod 256.
- Covers HDS "protocol rows" 1, 2, 3, 13, 14; **row 17 (immobilizer) deliberately omitted** ("no immobilizer support").
- Command bytes: READOUT `0xA0`, CLEAR `0xA1`, INFOREAD `0xA2`, WAKE `0xFE`; row 13 uses `0x5C` (read+clear); row 14 uses `0xB8` (readout) / `0xB9` (clear).
- Timing: rows 1-3 → t2=500 ms, t5=300 ms; row 13 → t2=0, t5=0; row 14 → t2=0, t5=30 ms.
- Example frames (header): wake `FE 04 91 6D`, stop `AD 04 F1 5E`, keep-awake `A0 05 70 01 EA`; row-3 wake is two messages `A0 05 00 01 5A` then `FE 04 91 6D`. (All verified to sum to 0 mod 256.)
- **Explicitly unknown**: bus-level init, baud rate, the meaning of readout/inforead data bytes, and all ECU reply bytes.

`klineservice.rs` (MEDIUM): "kwp.rs for HDS's VKWP systems on ISO 14230, obd9141.rs for generic OBD-II." An ISO 9141-2 adapter is "generic OBD-II only"; VKWP needs ISO 14230. Read-only — **DTC clear deliberately absent because "the clear bytes are unconfirmed."** 1,023 DTCs carry placeholder "Do not diagnose…" text.

`seedkey.rs` (HIGH): *"SecurityAccess key calculation is not part of the public source… No key is available in this source tree."* `calc_key` returns None, `calc_key_pcm` returns 0. The author could likely derive it from the decompiled HDS but chose not to publish it — this is the deliberate line between "diagnostics" and "unlock."

#### alexey-bass/honda-hds-help — HDS help text as HTML (HIGH)

Extracted HDS "help" pages: `Help.html` (~8 MB) plus per-system pages (e.g. `Civic-2013/ABS-VSA.html`, `AT.html`, `EPS.html`, `PGM-FI.html`, `SRS.html`) and a large `Images/` set of DTC/parameter diagrams (named `4R_<SYSTEM>_<TOPIC>`). This is the human-readable DTC and parameter reference as shown in HDS ([repo tree](https://github.com/alexey-bass/honda-hds-help)). Vehicle folder shown is a 2013 Civic (9th gen), but `Help.html` is the general HDS help corpus.

#### AutotronicCommunity/Honda_Keihin_KLine_Protocol — a REAL car capture (HIGH)

Files: `CIRCUIT UART KLINE.png`, `README.md`, `REQUEST - RESPONSE ECU HONDA K25 901.txt`. The .txt is a real K-engine ECM K-line session ([raw file](https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol)):

| Direction | Bytes | Meaning |
|---|---|---|
| Request | `fe 04 72 8c` | Wake-up (sums to 0x200) |
| Request | `72 05 71 00 18` | Read table 0x71, page 0x00 |
| Response | `02 0f 71 00 01 01 34 0f 01 01 00 00 00 00 37` | ECU reply (header `02`) |
| Request | `72 05 74 01 14` / `72 05 73 01 15` | Read tables 0x74 / 0x73 |
| Request | `72 05 71 17 01` | → `02 18 71 17 …` (24-byte block) |

So: **tester→ECU header `0x72`, ECU→tester header `0x02`; commands 0x71/0x73/0x74 are table/location reads; wake `FE 04 72 8C`** — identical to the motorcycle eculib wake from round 1, now confirmed on a car K-engine ECU. This is the datalogging protocol family the K20A3 ECM uses on pin 7.

#### Habr article 865294 (Russian) — a REAL CR-V SRS (DIAG-H) capture (MEDIUM-HIGH)

Captured with a home-built K-line logger on an **OpenPort 2.0** interface, against a Honda **CR-V III SRS** unit ([article](https://habr.com/en/articles/865294)). Request = `[command, total-length-incl-checksum, 2 param bytes, checksum]`, example `60 05 70 02 29` (sums to 0). `60h` = read (params select which DTCs/IDs; 2nd param = number of 16-byte data bytes returned, max `0x0F`); `61h` = action, used to clear recorded crash data. No raw reply bytes in the article text (only screenshots). Checksum attributed to the KEIHIN GitHub repo. **This is the closest public analogue to the EP3's pin-14 DIAG-H / SRS channel** and corroborates both the frame format and the `0x60/0x61` read/action pattern.

#### Generic DTC databases (LOW-MEDIUM, not HDS-derived)

`github.com/OBDb` (community-documented DTCs/PIDs) and `peyo/dtc-and-vin-data` (JSON DTCs) — thin Honda coverage; useful only as a cross-check.

### Q3. Free capture paths with no purchase

| Path | Cost | What it yields | Confidence |
|---|---|---|---|
| Generic ELM327 the owner may already own | ~$0-10 | Generic OBD-II only (ISO 9141-2 / SAE J1979 modes 01/02/03/04/07/09) on pin 7. **No Honda proprietary.** | HIGH |
| Published captures (already free) | $0 | Keihin K25 ECM session + Habr CR-V SRS session (above) + HondaECU moto logs — real byte sequences to bootstrap from | HIGH |
| Borrow a dealer/shop's HDS or a clone, sniff once | $0 | Passively wiretap the owner's own K-line (pin 7) and DIAG-H (pin 14) with the device's built-in sniffer while the borrowed tool drives a full session → ground-truth for every module, including security-access and reflash | HIGH (feasibility) |
| **Flagged paid exception** (owner decides) | **~$35-40** | ~$25-30 **GODIAG GD101** (J2534 + ELM327, ISO9141 + ISO15765, HDS-compatible) + ~$10 one-day i-HDS subscription; run HDS → car, sniff with the owner's passive device. Unlocks exact request/reply bytes for ECM/ABS/SRS/EPS/immobilizer, the **SecurityAccess seed/key exchange**, and the **reflash sequence** | HIGH on price/feasibility; MEDIUM on exactly what a 1-day sub enables |

- **GODIAG GD101** pricing confirmed: Walmart $29.99, Newegg $24.99, GODIAG shop $23; does both J2534 and ELM327 modes; one listing is "compatible with Honda HDS 3.104.024" ([GD101 listing](https://www.bestkeysupply.com/products/godiag-gd101-j2534-cable-p1062)). Caveat: in ELM327 mode it "can not read ISO9141 VIN."
- No complete public HDS↔Civic session dump covering all modules was found (searched forums, Habr, drive2, GitHub). The 2004-Civic-Si K-line thread on rusEFI is an *unresolved* "guessing the protocol" discussion — confirming the full byte-level protocol is **not** public (MEDIUM).
- Tactrix **OpenPort 2.0** (~$170) does K-line, but RomRaider notes it is "not friendly with Honda's K-Line"; the GD101 is cheaper and HDS-ready, so OpenPort is not recommended for this (MEDIUM).

**Bottom line for Q3:** truly $0 capture is possible only by (a) reusing the already-published captures, or (b) borrowing a tool for a one-off passive sniff. Generating your own HDS ground-truth without borrowing costs ~$35-40 — and that spend is the single highest-leverage one in the whole project because it is the only thing that cracks the two stall gates (Q5).

### Q4. How far free open source gets WITHOUT any capture

What can be **built and verified on the car for free** before any proprietary protocol is cracked:

| Capability | How, for free | Confidence |
|---|---|---|
| Generic OBD-II over K-line | ISO 9141-2 / SAE J1979: mode 01 live data, 02 freeze frame, 03/07 DTCs, 04 clear, 09 VIN/CALID. Via `fenugrec/freediag` (open) or own firmware | HIGH |
| SCS blink codes | Ground pin 9 (SCS) → ECM blinks stored DTCs on the MIL. Documented free in the FSM at **charm.li** ("Free Car Service Manuals… No strings attached!", no login/payment) — zero protocol needed | HIGH ([charm.li](https://charm.li)) |
| DTC/parameter/system meaning | Reuse HondaDiagnosticSuite's extracted DB: 36k DTC texts, 25.8k signal formulas, 182 systems, ID rules | HIGH |
| ECM proprietary table reads | Implement the Keihin `72`-header table-read (`72 05 71/73/74 pp cc`, reply header `02`) on pin 7 from the K25 capture + `eculocate` + HondaECU; validate against live K20A3 values | MEDIUM (protocol known; not yet validated on K20A3 specifically) |
| DIAG-H command openers | Implement `diagh.rs` command set (A0/A1/A2/FE; rows 1-3/13/14) against SRS/EPS on pin 14 | MEDIUM (HDS-derived, reply format must be learned on the car) |

Tool-by-tool relevance:

- **`fenugrec/freediag`** (open): generic OBD-II on ISO9141/14230/J1850; *"does not yet support any manufacturer specific protocols"* — so Honda-proprietary is out, but it is a solid free generic baseline ([freediag](https://freediag.sourceforge.io/)).
- **`kerpz/ArduinoHondaOBD`** (working K-line datalogger): targets **pre-2002** Hondas ("Honda ECU's before 2002," tested P2T OBD2 stock / P30 OBD1) using a `21AA/22AA/24AA` read format (digit = byte count, `AA` = address) — an older format than the EP3's `72`-header. Reference, not direct for the EP3 ([repo](https://github.com/kerpz/ArduinoHondaOBD)).
- **HondaECU / eculib** (`Adeen117/HondaECU`, GPL-3.0): the motorcycle K-line reference (wake, `72` table reads, flash `0x7D/0x7E`, security `0x27`). Protocol-family reference; targets moto ECUs, not the K20A3 ([repo](https://github.com/Adeen117/HondaECU)).
- **`eculocate`** (ESP32, moto, 10400 baud): open reference for Honda table-request framing — "interrogate it to dump its tables" ([eculocate](https://gti.telent.net/dan/eculocate)).
- **Hondata KManager / KTuner**: KManager is a free download but **requires the K-Pro daughterboard**; it **cannot read a stock EP3 ECU** ([hondata.com/kpro](https://hondata.com/kpro)). Useful only as a protocol/behaviour reference. `mkotyk/kpro-datalog` RE's the KPro *USB* datalog stream (not the K-line path).
- **RomRaider / nvmtool**: RomRaider needs a per-ECU definition, and **K-series Honda defs are not available** — community still asking as of Oct 2025; "haven't seen K-line read advertised yet for a Renesas based Honda ECU" (MEDIUM; **corrects round 1's "RomRaider K-series defs"** — [RomRaider Honda dev threads](https://mailserver.romraider.com/forum/viewtopic.php?p=175786)). `crome`/ECUFlash are OBD1-era Honda, not K-series.

**Net:** a genuinely capable **read-only** diagnostic tool is achievable for free: all DTCs (generic + SCS + known Honda read commands) across reachable modules, live data with correct names/formulas, and service identification. It stalls at clearing codes (clear bytes unconfirmed in open sources), actuator tests/learns, and anything behind SecurityAccess.

### Q5. Concrete ordered zero-cost "learn the protocol" plan

**Step 0 — Reuse, don't re-RE (effort: reading, $0).** Clone HondaDiagnosticSuite (DB + protocol scaffolding, GPL), clone `honda-hds-help` (DTC/parameter text), and study the Keihin K25 `.txt` + Habr article for real byte sequences. *Yields:* the entire DTC/parameter/system corpus and a validated frame format, for free, immediately.

**Step 1 — Generic OBD-II + SCS on the real car ($0).** Bring up the L9637D K-line transceiver end-to-end: generic DTC read/clear, live data, VIN; ground pin 9 for SCS blink codes. *Yields:* proven hardware + a working baseline; high confidence.

**Step 2 — ECM proprietary table reads ($0).** Implement the `72`-header Keihin table-read on pin 7; confirm K20A3 live values against the extracted signal formulas. *Yields:* real-time ECM datalogging without any new RE.

**Step 3 — DIAG-H (pin 14, 5V) bring-up ($0).** Implement the `diagh.rs` command set against SRS/EPS and **learn, on the owner's own car, the reply format and data-byte semantics that no open source has** (read-only, passive-friendly). *Yields:* original ground-truth that extends the public knowledge; this is where the project adds genuinely new data.

**Step 4 — Clearing + service functions ($0, lower confidence).** Attempt DTC clear with the (unconfirmed) clear bytes and simple resets; verify each carefully before trusting it.

**STALL POINT — where "free" genuinely ends.** Two gates stop a zero-cost effort:
1. **SecurityAccess (seed→key).** Every open source omits it (HondaDiagnosticSuite deliberately; `calc_key` returns None). Any protected service, write, or reflash needs it. Crossing it for free means REthe key algorithm out of the HDS binaries yourself (hard; the one person who decompiled HDS chose not to publish it).
2. **Stock ECM calibration read-out / reflash.** No public tool reflashes the stock EP3 **PNF/OKI** ECU over the DLC — free or commercial. There is no Ghidra/IDA processor module for the OKI nX-8/500S. Even commercial bench-flash tools (e.g. bFlash) target only the newer Keihin/Hitachi CPUs (MPC5566, SH7254x, TC275, SH72543), not the EP3's OKI part. The K-series tuning world sidesteps the stock ECU entirely with the **Hondata K-Pro daughterboard** (paid hardware), not a DLC reflash.

The cheapest break through both gates is the flagged ~$35-40 capture (Q3): sniff an HDS security-access + reflash session on the owner's own car. Everything up to Step 3 is free; Step 4 is free-but-risky; the reflash stretch goal is the real paid/stall boundary.

(Immobilizer key registration is a separate, *legitimate* gate: it requires a Honda-issued per-car **PCM code** obtained from Honda's iN system against the VIN — see Q6. This is by design and is not a target to defeat.)

### Q6. Legality / ethics framing for a zero-budget RE effort on one's own car

- **DMCA §1201(f) interoperability exception** (MEDIUM): a lawful user may reverse engineer portions of a program "solely to achieve interoperability with other programs that were not previously readily available" — squarely the posture of building one's own tool to talk to one's own car. ([EFF vehicle-repair comment](https://kittens.eff.org/files/2015/02/09/2014_-07_eff_vehicle_repair_comment.pdf))
- **Triennial vehicle-repair exemption, 37 CFR §201.40(b)(14)** (MEDIUM): since 2015 the Copyright Office permits circumventing TPMs on vehicle software when necessary for the **owner** to **diagnose, repair, or lawfully modify** their vehicle. It excludes telematics/entertainment and does **not** extend to third parties / commercial shops. A Sept-2026 Auto Innovators filing opposes renewal (status undecided). Fair-use / §117 arguments support owner tinkering but are untested in court. ([SEMA](https://sema.org/news-media/enews/2015/44/copyright-office-grants-exemption-allow-vehicle-modification), [Copyright Office 1201](https://www.copyright.gov/1201/))
- Honda's J2534 reprogramming being mandated available (historically "free of charge") reinforces the right-to-repair posture (MEDIUM).

**What to keep out of the public repo** (follow HondaDiagnosticSuite's model exactly):
- The **SecurityAccess seed→key** algorithm and any keys. Document that a gate exists; do not publish how to open it.
- Any **immobilizer key-registration** method or PCM-code derivation. Document only the architecture (key transponder → receiver/MICU → ECM/PCM) and the legitimate requirement that adding a key needs a **Honda-issued per-car PCM code from the iN system, tied to VIN and rotating** — do not publish a bypass ([Honda control-module replacement](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_TOOLS_CONTROL_MODULE_REPLACEMENT.htm)).
- **SRS/airbag** write and crash-data-erase procedures. Document the architecture (5V DIAG-H bus, `0x60` read / `0x61` action), not a how-to.
- **Honda's copyrighted data in bulk/cleartext.** Ship only *derived* data for interoperability, state that Honda retains rights in the original, and keep the GPL scoped to the project's own code.
- Operationally: keep the tool **read-focused**, gate every write behind explicit confirmation, and never broadcast to "every ECU at once."

---

## Conflicts and uncertainties

- **Is the i-HDS installer itself free to download?** Honda's official pages tie everything to a subscription; a forum user says the reprogramming tool "is still free too." The direct techinfo pages could not be fetched (TLS cert-chain error), so this rests on search-surfaced quotes. Resolved enough in practice because the extracted artefacts make a fresh download unnecessary. (MEDIUM/conflicting)
- **EP3 ECM physical layer.** Round 1 pins the ECM to the 12V K-line (pin 7); an old InsightCentral HIM/HDS thread (single user, LOW) measured DIAG-H as a 5V bus and speculated ECM data might also appear on H-DIAG. Treat pin 7 = ECM as the working assumption; verify on the car.
- **`diagh.rs` command set is HDS-derived and never run on a car.** Frame format is corroborated by two real captures (K25, CR-V), but the specific DIAG-H command bytes, reply format, and data-byte meanings are unverified on an EP3. (MEDIUM)
- **DTC clear bytes** are "unconfirmed" even in the project that extracted everything else — expect to confirm these on the car or from a capture. (MEDIUM)
- **K20A3 CPU** reported as OKI nX-8/500S (round 1, "reportedly"); not independently re-confirmed here. If true, it explains the total absence of a stock-ECU reflash path. (LOW on the exact part; HIGH that no public reflash exists)
- **`72`-header protocol on the K20A3 specifically.** Confirmed on a K-engine ("K25") ECU and used by the whole K-series tuning ecosystem, but a byte-level K20A3/PNF capture was not found public. (MEDIUM)

## Open questions (and the cheapest FREE way to close each)

- **Exact DIAG-H reply format + data-byte semantics for the EP3 SRS/EPS.** Cheapest close: passively read the owner's own modules with the device once Step 3 is built ($0) — this is original data-gathering, not RE of a proprietary binary.
- **DTC clear and service-function request bytes (confirmed).** Cheapest close: a borrowed-tool one-off sniff ($0), else the ~$35-40 capture.
- **SecurityAccess seed→key.** No free close short of RE'ing it from HDS binaries (high effort) or capturing an HDS unlock (~$35-40). This is the first true money/effort wall.
- **Stock PNF/OKI ECM read-out + reflash protocol.** No free close exists; no public tool at any price reflashes this ECU over the DLC. Realistic options are all paid (Hondata K-Pro daughterboard) or research-grade (chip-off + writing an OKI nX-8/500S disassembler from scratch).
- **A complete public HDS↔K-line-Civic session log.** Not found; likely does not exist publicly. Close only by self-capture.

## Sources

- [huhwhatbruh/HondaDiagnosticSuite](https://github.com/huhwhatbruh/HondaDiagnosticSuite) — GPL-3.0 project that decompiled HDS bytecode and extracted Honda's DTC/parameter/system database; DIAG-H + K-line + seedkey source.
- [alexey-bass/honda-hds-help](https://github.com/alexey-bass/honda-hds-help) — HDS help content (DTC/parameter text) extracted as HTML + image set.
- [AutotronicCommunity/Honda_Keihin_KLine_Protocol](https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol) — real Honda K-engine ECM K-line request/response capture + circuit.
- [habr.com/en/articles/865294](https://habr.com/en/articles/865294) — Russian write-up of an OpenPort 2.0 capture of a Honda CR-V SRS (DIAG-H), frame format and `0x60/0x61` commands.
- [Adeen117/HondaECU](https://github.com/Adeen117/HondaECU) — GPL-3.0 Honda motorcycle K-line diagnostic/flash tool (protocol-family reference).
- [kerpz/ArduinoHondaOBD](https://github.com/kerpz/ArduinoHondaOBD) — working Honda K-line datalogger for pre-2002 Hondas (`21AA/22AA/24AA` format).
- [gti.telent.net/dan/eculocate](https://gti.telent.net/dan/eculocate) — ESP32 Honda moto K-line table dumper (10400 baud), open table-request reference.
- [freediag.sourceforge.io](https://freediag.sourceforge.io/) — open generic OBD-II scantool (ISO9141/14230/J1850); no manufacturer-specific protocols.
- [hondata.com/kpro](https://hondata.com/kpro) — K-Pro / KManager: free software, but requires the paid daughterboard; cannot read a stock EP3 ECU.
- [RomRaider Honda development threads](https://mailserver.romraider.com/forum/viewtopic.php?p=175786) — K-series defs still unavailable; K-line Renesas Honda read not advertised.
- [charm.li](https://charm.li) — free Honda/Acura factory service manuals, no login/payment (FSM, DTC lists, SCS, DLC).
- [store.honda.com 1-day i-HDS subscription](https://store.honda.com/1-day-i-hds-subscription) + [techinfo J2534 page](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_J2534.htm) — HDS/i-HDS pricing and J2534 reprogramming gating.
- [techinfo control-module replacement](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_TOOLS_CONTROL_MODULE_REPLACEMENT.htm) — immobilizer key registration needs a Honda-issued per-car PCM code from iN (VIN-tied).
- [GODIAG GD101 J2534 listing](https://www.bestkeysupply.com/products/godiag-gd101-j2534-cable-p1062) — ~$23-30 J2534+ELM327 clone, HDS-compatible (the flagged capture exception).
- [Copyright Office §1201](https://www.copyright.gov/1201/) + [SEMA vehicle-modification exemption](https://sema.org/news-media/enews/2015/44/copyright-office-grants-exemption-allow-vehicle-modification) + [EFF vehicle-repair comment](https://kittens.eff.org/files/2015/02/09/2014_-07_eff_vehicle_repair_comment.pdf) — DMCA §1201(f) + 37 CFR §201.40 right-to-repair framing.
- [colinbourassa car_stuff/diagnostic_protocols](https://colinbourassa.github.io/car_stuff/diagnostic_protocols) — general K-line protocol background.
- [commaai/openpilot Honda-Acura wiki](https://github.com/commaai/openpilot/wiki/Honda-Acura) / gregjhogan `rwd-xray` — CAN-era Honda `.rwd` reflash file format + George Hotz firmware cipher (relevant to newer cars, not the pre-CAN EP3).
