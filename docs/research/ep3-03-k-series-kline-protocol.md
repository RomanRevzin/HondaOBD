# EP3 K-Series K-Line Diagnostic & Datalogging Protocol — What Can Be Learned For Free

Research dimension 3 of 5. Target car: US-market 2002 Honda Civic Si (EP3, K20A3, PNF
ECU, pre-CAN). Scope: everything about talking to a K-series ECU (and the other
control units) over the K-line family, using only free/open resources. Date: 2026-11-14.

## Summary

- **Generic OBD-II works and is proven on a K-series engine ECU.** A real capture of a
  2007 Accord **K20A6** (a close relative of the K20A3) shows the engine ECU answering
  plain **ISO 9141-2** (ELM327 protocol 3, 10.4 kbaud, 5-baud init to `0x33`), reply
  header `48 6B 11` (ECU address `0x11`), `0100` supported-PID bitmap `41 00 BE 3E A8 11`.
  Only the engine ECU answers generic OBD. This is the cheapest, most certain path for
  engine DTCs + live data on the EP3. (HIGH)
- **The single most valuable free asset is `huhwhatbruh/HondaDiagnosticSuite`** (GPL-3.0),
  a from-scratch reimplementation built by *decompiling the HDS bytecode* (139 modules,
  10,021 functions). It ships `hondascan.db` (36,000 DTCs, 25,882 signal definitions with
  Honda's own scaling formulas, 182 systems) and Rust source for three protocol families:
  generic ISO 9141-2, KWP2000 (ISO 14230, "VKWP"), and Honda's own older serial protocol
  (`diagh.rs`). Byte-level command tables, service IDs, timing and the DTC bitmap scheme
  are all readable in-source. (HIGH)
- **Honda's proprietary K-line protocol** (for enhanced FI data and the non-engine
  modules) uses a frame `[cmd, len, data…, checksum]` with a **two's-complement checksum
  (whole frame sums to 0 mod 256)** — the same family across cars and motorcycles. Command
  dialects differ: `0x20` table-reads (kerpz, pre-2002 3-pin cars), `0x72` (motorcycle +
  HUD ECU Hacker), and `A0/A1/A2` readout/clear/inforead (HDS `diagh.rs`). (HIGH on
  format, MEDIUM on which dialect the EP3 uses.)
- **Baud rate is contested:** generic OBD-II is firmly 10400; the Honda-proprietary
  mode is documented as **9600** by the HDS-derived suite and by kerpz (cars), but as
  **10400** by motorcycle-oriented sources. The firmware must make this selectable. (MEDIUM)
- **Fast K-series datalogging is a Hondata/KTuner-daughterboard feature, not a stock-ECU
  feature.** Hondata's documented serial stream is a one-way broadcast on ECU pin **E24**
  from a Hondata-flashed (PRB) calibration, not reachable on a stock EP3. (HIGH)
- **ABS, SRS, EPS, immobilizer** are Honda-protocol (or blink-code) systems. The
  guaranteed-free read for all of them is the **SCS blink-code** method (ground pin 9).
  Serial reads need the Honda protocol, whose exact framing for these modules is not
  public; the SAE J2809 ABS/VSA spec is paywalled. (HIGH on blink codes, LOW on serial.)
- **Security is a hard wall for free work:** the HDS-derived suite deliberately omits the
  SecurityAccess (`0x27`) seed→key algorithm. Immobilizer key registration / airbag
  writes cannot be done from free sources and need a Honda-issued per-car code. (HIGH)

## Findings

### Q1 — Generic OBD-II / ISO 9141-2 on the K20

**Confirmed: the K-series engine ECU speaks ISO 9141-2 (not CAN, not KWP) for legislated
OBD-II.** The best evidence is a real community capture.

Real capture — 2007 Accord VII 2.0 i-VTEC, engine **K20A6** (EU), from the WiCAN firmware
issue tracker (HIGH, real logs):

| Item | Value |
|---|---|
| Protocol | ISO 9141-2 (K-line); ELM `ATSP3`, `ATDPN` = `A3`, `ecu_protocol 3` |
| Init | `ATSP3;ATH1;` (ELM327 handles 5-baud init) |
| Modules answering generic OBD | "Only the engine ECU answers generic OBD" |
| Reply header (3-byte K-line) | `48 6B 11` → **ECU address `0x11`** |
| `0100` supported-PID bitmap | `41 00 BE 3E A8 11` |
| `0105` (coolant) | `48 6B 11 41 05 7D 87` |
| `0104` (load) | `48 6B 11 41 04 5F 68` |
| `0106` (STFT) | `48 6B 11 41 06 7B 86` |
| `0111` (TPS) | `48 6B 11 41 11 28 3E` |
| `010C` (RPM) | `48 6B 11 41 0C 0B 44 60` |

Source: [wican-fw issue #1035](https://github.com/meatpiHQ/wican-fw/issues/1035). The
K20A6 is the same K-series architecture/era as the K20A3; the EP3 is extremely likely to
behave the same (MEDIUM for EP3 specifically — not the exact same ECU).

Corroboration and the generic parameters (HIGH unless noted):

- **Physical layer:** K-line on **DLC pin 7**, 10.4 kbaud, 8N1, half-duplex single wire,
  0/12 V. 5-baud slow init sends `0x33` at 5 baud; ECU replies `0x55` sync at 10400, then
  key-word bytes. ([SparkFun OBD-II protocols](https://learn.sparkfun.com/tutorials/getting-started-with-obd-ii/obd-ii-protocols),
  [m0agx K-line part 2](https://matt2005.github.io/2019-12-28-repost-reading-obd2-data-without-elm327-part2))
- **HDS-derived suite `obd9141.rs`** (the decompiled-HDS reference implementation):
  request header `68 6A F1`, reply header `48 6B <ecu>`, 5-baud init at address `0x33`;
  modes `LIVE_DATA 0x01`, `FREEZE_FRAME 0x02`, `STORED_DTC 0x03`, `PENDING_DTC 0x07`,
  `VEHICLE_INFO 0x09`; positive response = service byte + `0x40`; a DTC frame "carries
  three code slots of two bytes; `0000` is an empty slot."
  ([obd9141.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/obd9141.rs)) (HIGH)
- **Key words / ECU address vary by model.** A 2001 Honda Acty (K-line sibling) answered
  ISO 9141-2 with key words `08 08` and ECU address `0x10`
  ([HDS-suite issue #11](https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11));
  the K20A6 Accord used ECU `0x11`. For the K20 the observed value is `0x11`. (MEDIUM)
- **EP3 factory data:** the EP3 service manual's DLC continuity check tests terminal
  **No. 7 to the ECM**; the K-series DLC pinout puts pin 7 ("K-Line, 2-way OBDII serial")
  on ECU pin E23. ([EP3 PGM-FI, ManualsLib](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=335),
  [rywire K-series DLC pinout](https://blog.rywire.com/k-series-dlc-connector-pinout/)) (HIGH)
- **Coverage note:** A commercial Honda scanner (Astemo/Hitachi HDM-9000) lists the EP3
  Civic Si (2001-2005) and DC5 Integra Type R among cars whose *engine* fault codes it
  reads over the OBD connector. ([Astemo Honda car list](https://www.aftermarket.astemo.com/japan/ja/automotive/assets/docs/1_8000_HONDA_car_list_231026.pdf)) (MEDIUM)

**A real EP3 K20A3 `0100` bitmap was not found in public sources** — the K20A6 capture is
the closest. Cheapest way to close it: run any ELM327 on the owner's own car.

### Q2 — Honda-mode / fast datalogging on K-series (Hondata, KTuner)

**Finding: high-rate K-series datalogging is a property of the Hondata/KTuner hardware,
not of the stock ECU's K-line.** There is no stock-ECU "Honda-mode" fast log over pin 7.

- **Hondata K-Pro** is a daughterboard installed *inside* the ECU (irreversible mod).
  KManager logs to a laptop over **USB** from that board; KPro4 logs at ~100 Hz with
  10-220 min of onboard memory. ([Hondata KPro](https://hondata.com/kpro),
  [Hybrid Racing listing](https://hybrid-racing.com/products/hondata-k-pro-c)) (HIGH)
- **Hondata documented serial stream ("Datalogger Output")** — this is the one
  openly specified Hondata transport, and it is a **one-way broadcast**, enabled in
  KManager under Parameters → Multiplexer (HIGH,
  [Hondata Serial Output](https://hondata.com/help/kmanager/serialoutput.htm)):

  | Parameter | Value |
  |---|---|
  | Pin | ECU **E24 (SEFMJ)**, normally dashboard comms (disables stock water-temp gauge + charge light) |
  | Levels | 12 V / 0 V (high may follow battery 10-14 V), idle 12 V |
  | Baud | 9600 or 115200 bps, 8 data, 1 stop, no parity (software selectable) |
  | Rate | ~20 Hz (9600) / ~30 Hz (115200) |
  | Packet | one type, 22 bytes; byte 0 = ID `0x69`, byte 21 = checksum; MSB-first words |
  | Payload | speed, RPM, gear, coolant, IAT, battery, TPS, MAP, injector duration, timing, output/input bit fields, cam angle, lambda, knock count |
  | Checksum | `0xFF − (byte sum)`; full packet (incl. checksum) sums to `0xFF` |
  | Requirement | **PRB calibrations only** (i.e. a Hondata-flashed ECU); PRA cannot use PRB |

  This is what **Hondash** consumes. It needs Hondata hardware, so it is **not available
  on a stock EP3**, but if the owner ever runs K-Pro it is a trivially free spec to read.
- **KTuner:** uses J2534 over OBD-II for flash-based cars; its help documents "Direct
  Serial Logging" only for supported widebands, not a stock K-line log.
  ([KTuner](https://ktuner.com/), [KTuner help](http://www.ktuner.com/KTunerHelp/tuning.htm)) (MEDIUM)
- **No open-source K-series datalogger or kmanager/KTuner traffic sniff was found.** The
  only openly documented Hondata transport is the E24 stream above. (MEDIUM — absence of
  evidence)

**Implication for the build:** stock-ECU datalogging over pin 7 is limited to (a) generic
OBD-II mode-01 polling (slow, round-trip per PID) and (b) Honda-mode block reads (see Q3).
Neither matches Hondata's 100 Hz; a Honda-mode block read (many channels per frame) is the
fast-ish option on a stock ECU.

### Q3 — The published HDS-derived protocol database (`HondaDiagnosticSuite` / `hondascan.db`)

**Project: [`huhwhatbruh/HondaDiagnosticSuite`](https://github.com/huhwhatbruh/HondaDiagnosticSuite),
GPL-3.0-or-later, "not affiliated with Honda."** Built by decompiling the HDS bytecode
("139 modules, 10,021 functions, no opcode left undecoded"). Confirmed on a 2015 Accord
(K24W1, CVT); partial scans of 2019/2021 Civic. ([README](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/README.md)) (HIGH)

**Database (`hondascan/data/hondascan.db`, ~21 MB, encrypted via `dbcrypt.rs`):** 36,000
trouble codes, 25,882 signal definitions each with Honda's own conversion formula, 3,607
unit converters, 182 systems; "113 of the 182 systems use the older K-line or Honda's own
older protocol." (HIGH) Schema (from `bin/build-db.rs`, HIGH):

| Table | Columns (verbatim where quoted) | Role |
|---|---|---|
| `systems` | `name, display, family, props, type` | `family` filled from CSV field **`vendor_protocol`** — the protocol-family selector per system |
| `ecus` | `system, id_pattern, name` | ECU id match |
| `dtc` | `system, sae, subcode, honda_code, …` | DTC text, SAE↔Honda code map |
| `diagh_dtc_map` | `system, honda_main, honda_sub, symbol, …` | "Honda DIAG-H: the module address and bit of each trouble code; all values fit a byte" |
| `can_packets` / `can_signal_parts` / `can_addresses` | DIDs, signal bytes/scaling, CAN addr | newer CAN cars; K-line (VKWP) signals also stored in `can_signal_parts` with the local id in the `did` column |

**Three protocol families in source:**

1. **Generic ISO 9141-2 / J1979** — `obd9141.rs` (see Q1).

2. **KWP2000 / ISO 14230 ("VKWP")** — `kwp.rs` (HIGH,
   [kwp.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/kwp.rs)):

   | Service | ID | | Service | ID |
   |---|---|---|---|---|
   | StartCommunication | `0x81` | | ReadDTCByStatus | `0x18` |
   | StopCommunication | `0x82` | | ReadECUID | `0x1A` |
   | StartSession (`10 81`) | `0x10` | | ReadByLID | `0x21` |
   | ReadFreezeFrame | `0x12` | | TesterPresent (`3E 01`/`3E 02`) | `0x3E` |
   | ReadDTC | `0x13` | | `TESTER` | `0xF1` |
   | ClearDTC (`14 FF 00`) | `0x14` | | `ALL_GROUPS` | `0xFF00` |

   Frame `Fmt [Tgt Src] [Len] data… Checksum`; Fmt bits 7-6 addressing (`00` none / `10`
   physical / `11` functional), bits 5-0 length (`0` ⇒ a length byte follows); checksum =
   sum of all earlier bytes mod 256. Timing `P2 = 0.2 s`, `P2* = 5.0 s`, retries 1, pause
   after NRC `0x21` = 200 ms. ELM path: fast init `ATSP5`, 5-baud `ATSP4` + `ATIIA<tgt>`;
   `PHYSICAL = 0x80`, `FUNCTIONAL = 0xC0`.
   ([elm_kline.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/transport/elm_kline.rs))

3. **Honda's own older serial protocol** — `diagh.rs` + `transport/diagh_serial.rs`.
   Module doc: covers "the 55 systems HDS lists as the HONDA protocol family"; the code
   "has never run on a car," and reply framing / byte meaning is marked unknown. (HIGH on
   the request side, LOW on replies — unverified by the author) Frame `[command, length,
   data…, checksum]`, `length` counts every byte, checksum is the two's-complement negation
   so all bytes sum to 0 mod 256. **Baud = 9600 8N1** (`pub const BAUD: u32 = 9600`).
   Commands: `READOUT 0xA0`, `CLEAR 0xA1`, `INFOREAD 0xA2`, `WAKE 0xFE`; row-specific `0x5C`
   and `0xB8/0xB9`. An **H/99 wake-up pulse is sent by the interface hardware**, not from
   the database — "the pulse timings are the interface's defaults." The `PROFILES` table
   (HIGH, [diagh.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/diagh.rs)):

   | Row | Wake-up | Keep-awake | Stop |
   |---|---|---|---|
   | 1 | `FE 04 91 6D` | `A2 07 72 00 00 01 E4` | `AD 04 F1 5E` |
   | 2 | `FE 04 91 6D` | `A0 05 70 01 EA` | `AD 04 F1 5E` |
   | 3 | `A0 05 00 01 5A` then `FE 04 91 6D` | `A0 05 70 01 EA` | `AD 04 F1 5E` |
   | 13 | `FE 04 A9 55` | `5C 06 20 19 04 61` | `5C 04 00 A0` |
   | 14 | `20 05 00 02 D9` | (none) | (none) |

   Row 17 (immobilizer) is deliberately excluded from the source. Checksum check: e.g.
   `FE+04+91+6D = 0x200 ⇒ 0x00 mod 256`. ✓

**DTC bit-map structure** — `diagh_dtc.rs` (HIGH,
[diagh_dtc.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/diagh_dtc.rs)):
the map comes from `tblDtcPidXref` joined with the PID table → `diagh_dtc_map`. Each
`DtcBit` = `{honda code (main, sub), symbol, module address (u8), mask (u8)}`. Input is a
map of module address → the single byte read from it. Logic: single-bit mask hitting a set
bit ⇒ code **present**; multi-bit mask with any bit set ⇒ **uncertain**; address with no
byte ⇒ **unread/unknown**. "How a DIAG-H reply carries those bytes is not known (no reply
has been seen)." So: **read one status byte per module address, test each DTC's bit-mask.**

**Security** — `seedkey.rs` (HIGH,
[seedkey.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/seedkey.rs)):
"SecurityAccess key calculation is not part of the public source." `calc_key(seed,type)`
returns `None`; `calc_key_pcm(seed)` returns `0`. Scan, codes and live data need no unlock;
anything gated by SecurityAccess (`0x27`) — immobilizer key registration, some actuator
tests, airbag writes — cannot be done from this free source.

### Q4 — Which command family the EP3 ECM most likely speaks on pin 7

The Honda K-line serial protocol is one family (frame `[cmd len data… 2's-comp checksum]`,
whole frame sums to zero) with several **command dialects** and a disputed baud rate:

| Dialect / source | Baud | Wake | Read cmd | Clear | Checksum | Target cars |
|---|---|---|---|---|---|---|
| **kerpz ArduinoHondaOBD** ([src](https://github.com/kerpz/ArduinoHondaOBD/blob/master/hobd_uni/hobd_uni.ino), [README](https://github.com/kerpz/ArduinoHondaOBD)) | 9600 8N1 | init string `68 6a f5 af bf b3 b2 c1 db b3 e9` + 300 ms | `0x20` table read (`20 05 <loc> <len> <cks>`) | `21 04 01 00` | `0xFF−(sum−1)` (= 2's-comp) | "Honda ECUs **before 2002**", 3-pin DLC; tested P2T OBD2, P30 OBD1 |
| **HDS `diagh.rs`** (decompiled HDS) | 9600 8N1 | `FE 04 91 6D` (+ interface H/99 pulse) | `A0` readout, `A2` inforead | `A1` | 2's-comp | the 55 HDS "HONDA protocol family" systems |
| **Motorcycle eculib / HUD ECU Hacker** ([HUD](https://netcult.ch/elmue/HUD%20ECU%20Hacker/)) | **10400** | **~70 ms** low pulse (ISO14230-fast-like) | `0x72` (`72 05 00 F0 99` → `02 04 00 FA`) | — | 2's-comp | Honda bikes; "undocumented proprietary protocol over K-Line at 10400 baud" |
| **Generic OBD-II** (J1979) | **10400** | 5-baud `0x33` | mode `01` | mode `04` | sum/8 | mandatory on every post-1996 car incl. EP3 — **proven on K20A6** |

Correlations and inferences:

- The **frame format and two's-complement checksum are common to all three Honda dialects**
  — the firmware checksum routine is shared. (HIGH)
- The **wake command `0xFE` is common to HDS cars (`FE 04 91 6D`) and bikes (`FE 04 72 8C`)**
  — same `FE len … cks` wake frame, different payload. (HIGH)
- The issue-#11 reporter tried kerpz's exact pre-2002 command (`20 05 76 0A 5B`, which is
  kerpz's "ecu id" `h_cmd4`) on a 2001 Acty and got **no reply**, but plain ISO 9141-2
  worked. This suggests the **2001+ 16-pin K-line cars dropped the old `0x20`-table dialect**
  in favour of ISO 9141-2 (generic) + the HDS Honda-family dialect for enhanced/other
  modules. (MEDIUM, single data point)
- **Best guess for the EP3 (MEDIUM/LOW):** engine generic data over ISO 9141-2 @ 10400
  (proven on K20A6); any Honda-enhanced engine data and the non-engine modules over the
  HDS Honda-family dialect (`A0/A1/A2`, 9600, H/99 pulse). The exact per-module bytes for
  the EP3 are **not public**.
- **Open baud question:** 9600 (HDS suite + kerpz, cars) vs 10400 (bikes/HUD). Make it
  configurable; try 9600 first for Honda-mode on this car.

### Q5 — Existing open / hobby tools that read Honda K-line data over a plain adapter

| Tool | What it does | Fit for stock EP3 |
|---|---|---|
| **[Skeleten80/vtec-logger](https://github.com/Skeleten80/vtec-logger)** (2026, open, read-only v0.1) | Honda K-line discovery instrument: tries ISO 9141-2 5-baud (ELM proto 3) first, then KWP2000 5-baud/fast; bus monitor; **experimental Honda mode `$22`** query, logged raw/undecoded. Example car: 2007 Accord K24A8. | **Closest existing open tool.** Confirms the ISO 9141-2 path; Honda enhanced mode still undecoded. (MEDIUM) |
| **[muki01/OBD2_KLine_Library](https://github.com/muki01/OBD2_KLine_Library)** (Arduino/ESP32) | ISO9141 / ISO14230 (KWP2000) / KW1281 / DS2 / KW82; init `Init_5Baud`/`Init_Fast`/`Init_Ping`; `setInitAddress(0x33)`; `readStoredDTCs()`, `getLiveData(pid)`, `clearDTCs()`; `68 6A F1 …` request. | Good **C reference** for the firmware's K-line/KWP layer; no Honda-enhanced support. (HIGH as a code ref) |
| **[dewasha40/OBD2_K-line_Reader](https://github.com/dewasha40/OBD2_K-line_Reader-spanish)** (ESP32/Arduino) | ISO9141 + ISO14230 reader. | Generic K-line reference. (MEDIUM) |
| **kerpz/ArduinoHondaOBD** | 9600 Honda `0x20`-table protocol. | **Pre-2002 3-pin only** — not the EP3, but documents the old dialect. (HIGH) |
| **Adeen117 / roccofz1 HondaECU (eculib)** | Honda **motorcycle** K-line read/flash (FTDI + K-line transceiver), 10400, `FE 04 72 8C`, `0x72`. | Motorcycle only; best byte-level relative for the Honda proprietary protocol. (HIGH as a ref) |
| **Torque / OBD Fusion "enhanced Honda PIDs"** ([VCMTUNER CSV](https://www.vcmtuner.com/pages/torque-pro-app-enhanced-honda-pids-for-vcmtuner-and-vcmtuner-ii)) | Custom PIDs adding ECT/IAT etc. | For **newer CAN Hondas** (tested 2022 Odyssey / 10th-gen Civic). **Not applicable to the pre-CAN EP3.** (MEDIUM) |
| **ScanGauge X-Gauge** | User-defined extra gauges. | No EP3/K20-specific X-Gauge recipe found in public sources. (LOW — not found) |
| **Hondash** ([hondash.com](https://hondash.com)) | Instrument cluster reading the Hondata E24 serial stream. | **Requires Hondata K-Pro v2/v3/v4 or S300 v3** — reads K-Pro/S300 USB, not a stock ECU. The non-Hondata Hondash variant is the OBD1 (3-pin, ≤2001) product. **Does not read a stock EP3.** (HIGH — corrects the round-1 "3-pin only" framing: current Hondash needs Hondata HW) |

Bottom line: **no open tool today reads Honda-specific (enhanced FI / ABS / SRS / EPS)
data from a stock EP3 over a plain K-line adapter.** vtec-logger is the nearest and is
explicitly a "gather real logs" instrument. ELM327/STN adapters do the generic ISO 9141-2
engine data fine; one source warns many cheap "ELM327" clones lack the genuine PIC18F25K80
and are unreliable ([CivicX PIDs thread](https://www.civicx.com/forum/threads/pids-for-torque-torque-pro.23291/)).

### Q6 — ABS (pin 7) and EPS / immobilizer protocols

**EP3 16-pin DLC pinout** — the K-series-specific source
([rywire](https://blog.rywire.com/k-series-dlc-connector-pinout/), HIGH; corroborated by a
[D-series forum](https://www.d-series.org/threads/abs-and-srs-faults.172153/page-2) and the
EP3 manual's pin-7 continuity check):

| Pin | Wire | Signal | To |
|---|---|---|---|
| 4 | Black | Chassis ground | ties to E7 |
| 5 | Brown/Yellow | LG3 logic ground | ECU E3 |
| 7 | Light Blue | **K-Line** (2-way OBD-II serial) | ECU E23 |
| 9 | Brown | **SCS** (Service Check Signal) | ECU E29 |
| 12 | Red/White | **Write-enable** (for rewriting the ECU) | ECU E30 |
| 14 | Gray | **+5 V from immobilizer control unit pin 3** ("probably for rewriting immobilizer codes") | immobilizer unit |
| 16 | White/Red | +12 V battery (BACKUP fuse) | ties to E9 |

**ABS:**
- Physically on **pin 7 K-line** (single-wire); "the scan tool does all of its
  communication over a single-wire K line" and HDS can read ABS where generic ELM327 often
  cannot. ([honda-tech 2002 Civic thread](https://honda-tech.com/forums/honda-civic-del-sol-1992-2000-1/obd-2-question-3144511/)) (MEDIUM)
- **Free guaranteed read = SCS blink codes:** ground the SCS line (pin 9) and count ABS
  lamp flashes (long blink = 10, short = 1; lamp goes off ~3.6 s when no code). Factory
  method. ([manualslib Honda Civic](https://www.manualslib.com/manual/813630/Honda-Civic.html?page=627)) (HIGH)
- **Serial read:** via the Honda protocol; exact ABS framing is not public. The relevant
  SAE spec is **J2809 "Honda Diagnostic Serial Data Link Protocol — ABS/VSA System"**,
  which is paywalled and out of scope under the zero-budget rule.
  ([BSI listing](https://knowledge.bsigroup.com/products/honda-diagnostic-serial-data-link-protocol-abs-vsa-system)) (HIGH that it exists; content not accessible)
- In the HDS-derived suite, ABS would be one of the Honda-family systems (read via `A0`
  readout + the `diagh_dtc_map` address/bitmask scheme). (LOW — inferred, not verified)

**EPS (the EP3 has electric power steering):**
- **Free read = SCS blink codes:** short the SCS circuit, ignition ON, the EPS/PS
  indicator blinks the DTC (the indicator lights ~6 s first). Factory method on the
  2005 Civic Si (EP3's last year).
  ([2005 Civic Si steering DTC, CHARM mirror](https://manuals.autorepairshq.com/Honda/2005/Civic%20Si%20L4-2.0L/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes)) (MEDIUM — page known but not fully fetched)
- **Physical line is uncertain.** Round-1 placed SRS/EPS on a "DIAG-H" second serial line
  at pin 14, but the EP3-specific pinout sources describe **pin 14 as a +5 V immobilizer
  line, not a serial bus**, and generic standard pinouts call pin 14 CAN-L (N/A on the
  pre-CAN EP3). No source confirms a dedicated serial-to-SRS/EPS pin on the EP3 DLC. (See
  Conflicts.) The dependable free read for EPS and SRS is therefore the SCS blink method.

**SRS / airbag:** read via SCS blink or Honda serial; any *writes* are SecurityAccess-gated
(key algorithm not public). Document the architecture only.

**Immobilizer:** pin 14 supplies +5 V from the immobilizer control unit ("probably for
rewriting immobilizer codes"). Key registration needs a **Honda-issued per-car code** and
SecurityAccess — not achievable from free resources, and intentionally excluded (row 17)
from the HDS-derived suite. Legitimate requirement documented; no defeat method.

## Conflicts and uncertainties

1. **Honda-proprietary baud rate: 9600 vs 10400.** The HDS-derived suite (`diagh_serial.rs`,
   `BAUD = 9600`) and kerpz (cars) say **9600**; HUD ECU Hacker and the motorcycle tools say
   **10400**. Generic OBD-II is unambiguously 10400. Likely resolution: cars = 9600,
   bikes = 10400 — but treat as unresolved and make baud selectable. (MEDIUM)
2. **Pin 14 identity.** EP3-specific sources: +5 V immobilizer line. Generic OBD-II
   standard: CAN-L (not populated on pre-CAN EP3). Round-1: "DIAG-H ~5 V serial to
   SRS+immobilizer+EPS." The *serial-bus-to-SRS/EPS* interpretation is **unconfirmed** by
   any EP3 source found here. The device's "DIAG-H channel" should be treated as
   experimental; SCS blink codes are the reliable fallback for SRS/EPS/ABS.
3. **Which Honda command dialect the EP3 enhanced/other modules use** (`0x20` vs `0x72` vs
   `A0/A1/A2`) is not established for the EP3. The pre-2002 `0x20`-table dialect probably
   does **not** apply (Acty data point); `A0/A1/A2` (HDS family) is the leading candidate.
4. **ECU address:** `0x10` (Acty) vs `0x11` (K20A6 Accord). K20 observed = `0x11`.
5. **`diagh.rs` is unverified against a car** by its own author ("never run on a car,"
   reply framing unknown). The request-side bytes are from the decompiled HDS; replies are
   not proven.
6. **The `0x100000`+ tail of HUD ECU Hacker and the full `hondascan.db` contents were not
   read** (db is 21 MB and encrypted). The EP3's exact `systems.family` value and its
   `diagh_dtc_map` addresses/masks remain unread.

## Open questions (and the cheapest FREE way to close each)

1. **Does the EP3 K20A3 answer ISO 9141-2, and what are its `0100` bitmap, ECU address and
   key words?** → Run any ELM327/clone on the owner's own car (zero cost; proven approach
   on K20A6). Or passive-sniff with the device under build.
2. **Exact EP3 Honda-mode dialect, baud and per-module request bytes (FI enhanced, ABS,
   SRS, EPS).** → Passive-sniff a borrowed genuine HDS or HDS clone session on the owner's
   car with the device's sniffer (the single highest-value free experiment), and/or probe
   `FE 04 91 6D` at 9600 after a ~70 ms low wake pulse, then `A0` readout.
3. **H/99 wake-pulse exact timing.** → Documented as ~70 ms low (HUD/motorcycle);
   confirm by sniffing HDS, since the HDS-suite leaves it to interface defaults.
4. **The EP3 rows inside `hondascan.db` (`systems.family`, `diagh_dtc_map`
   address+mask per DTC, signal scaling).** → The DB is GPL and the decrypt path
   (`dbcrypt.rs`) + `build-db.rs` are in-source; build/read it locally, inspect via the
   GUI, or ask in the repo issues. (Do not run downloaded code unsandboxed.)
5. **True function of pin 14 on the EP3** (+5 V supply vs serial bus). → EP3 wiring diagram
   on charm.li; confirm with a multimeter on the owner's car.
6. **ABS serial framing** (behind SAE J2809). → Sniff an HDS ABS session; J2809 itself is
   paywalled and excluded by the budget.

## Sources

- [huhwhatbruh/HondaDiagnosticSuite](https://github.com/huhwhatbruh/HondaDiagnosticSuite) — GPL-3.0 reimplementation of HDS from decompiled bytecode; the core free asset.
  - [README](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/README.md) — project scope, DB stats, protocol families, licence.
  - [kwp.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/kwp.rs) — KWP2000 (ISO 14230) service IDs, frame, timing.
  - [obd9141.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/obd9141.rs) — generic ISO 9141-2 / J1979 modes, headers, DTC slots.
  - [diagh.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/diagh.rs) — Honda proprietary protocol: commands A0/A1/A2/FE, PROFILES wake/keepalive/stop.
  - [diagh_dtc.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/diagh_dtc.rs) — DTC module-address + bit-mask structure.
  - [transport/diagh_serial.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/transport/diagh_serial.rs) — 9600 8N1, H/99 pulse is interface default.
  - [transport/elm_kline.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/transport/elm_kline.rs) — ELM327 ATSP4/5, ATIIA, PHYSICAL/FUNCTIONAL format bytes.
  - [bin/build-db.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/bin/build-db.rs) — DB schema; `systems.family` = `vendor_protocol`.
  - [seedkey.rs](https://github.com/huhwhatbruh/HondaDiagnosticSuite/blob/main/hondascan/src/seedkey.rs) — SecurityAccess 0x27 key algorithm deliberately omitted.
  - [issue #11](https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11) — 2001 Acty FI_ answered ISO 9141-2 (0x33, key words 08 08, ECU 0x10).
- [meatpiHQ/wican-fw issue #1035](https://github.com/meatpiHQ/wican-fw/issues/1035) — real 2007 Accord K20A6 ISO 9141-2 capture: ECU 0x11, header `48 6B 11`, `0100` = `41 00 BE 3E A8 11`.
- [kerpz/ArduinoHondaOBD](https://github.com/kerpz/ArduinoHondaOBD) ([hobd_uni.ino](https://github.com/kerpz/ArduinoHondaOBD/blob/master/hobd_uni/hobd_uni.ino)) — pre-2002 Honda 9600-baud `0x20`-table protocol, init string, checksum.
- [Skeleten80/vtec-logger](https://github.com/Skeleten80/vtec-logger) — open read-only Honda K-line discovery tool; ISO 9141-2 first, experimental mode `$22`.
- [muki01/OBD2_KLine_Library](https://github.com/muki01/OBD2_KLine_Library) — Arduino/ESP32 ISO9141/ISO14230 library; good firmware reference.
- [dewasha40/OBD2_K-line_Reader](https://github.com/dewasha40/OBD2_K-line_Reader-spanish) — ESP32/Arduino K-line reader.
- [Adeen117/HondaECU](https://github.com/Adeen117/HondaECU) — Honda motorcycle K-line read/flash (eculib relative); 10400 baud, `0x72`, `FE 04 72 8C`.
- [HUD ECU Hacker (netcult.ch/elmue)](https://netcult.ch/elmue/HUD%20ECU%20Hacker/) — Honda proprietary K-line 10400 baud, ~70 ms wake pulse, `72 05 00 F0 99` → `02 04 00 FA`; ISO9141 5-baud/0x33/0x55; L-line pin 15.
- [Hondata Serial Output (KManager help)](https://hondata.com/help/kmanager/serialoutput.htm) — E24 one-way datalogger stream spec (9600/115200, 20/30 Hz, 22-byte packet, ID 0x69, PRB only).
- [Hondata K-Pro](https://hondata.com/kpro) / [Hybrid Racing KPro4](https://hybrid-racing.com/products/hondata-k-pro-c) — daughterboard, 100 Hz USB datalogging.
- [KTuner](https://ktuner.com/) / [KTuner help](http://www.ktuner.com/KTunerHelp/tuning.htm) — J2534 over OBD2 + direct-serial wideband logging.
- [hondash.com](https://hondash.com) — Hondash requires Hondata K-Pro/S300.
- [rywire K-series DLC connector pinout](https://blog.rywire.com/k-series-dlc-connector-pinout/) — EP3/K-series DLC pin-by-pin (pin 7 K-line E23, pin 9 SCS, pin 12 WEN, pin 14 immobilizer +5 V).
- [EP3 PGM-FI service manual (ManualsLib)](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=335) — DLC pin-7-to-ECM continuity check.
- [2005 Civic Si steering DTC (CHARM/autorepairshq mirror)](https://manuals.autorepairshq.com/Honda/2005/Civic%20Si%20L4-2.0L/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes) — EPS DTC read via SCS short + indicator blink.
- [manualslib Honda Civic ABS blink codes](https://www.manualslib.com/manual/813630/Honda-Civic.html?page=627) — SCS short, long blink = 10 / short = 1.
- [honda-tech 2002 Civic OBD thread](https://honda-tech.com/forums/honda-civic-del-sol-1992-2000-1/obd-2-question-3144511/) — single-wire K-line; HDS reads modules ELM327 cannot.
- [SAE J2809 (BSI listing)](https://knowledge.bsigroup.com/products/honda-diagnostic-serial-data-link-protocol-abs-vsa-system) — Honda ABS/VSA serial protocol (paywalled; existence only).
- [Astemo HDM-9000 Honda car list](https://www.aftermarket.astemo.com/japan/ja/automotive/assets/docs/1_8000_HONDA_car_list_231026.pdf) — EP3 engine codes readable over OBD connector (coverage data point).
- [SparkFun OBD-II protocols](https://learn.sparkfun.com/tutorials/getting-started-with-obd-ii/obd-ii-protocols) / [m0agx K-line part 2](https://matt2005.github.io/2019-12-28-repost-reading-obd2-data-without-elm327-part2) — generic ISO 9141-2 / KWP2000 init background.
