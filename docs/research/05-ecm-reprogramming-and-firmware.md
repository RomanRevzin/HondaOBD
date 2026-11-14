# 05 - Writing to the ECM/PCM: reprogramming, firmware read-out and persistent writes (2001-2005 Civic)

Dimension owner notes. Scope: can the owner's device read and (especially) *write* the
engine/powertrain control module on a 7th-gen Civic - clear codes, run dealer service
writes, and at the stretch end dump and reflash the ECM/PCM firmware. Covers US D17
(PLM/PLR/PMP-family part numbers), US K20A3 Civic Si (PNF), and European/Japanese
variants, noting where they differ.

Confidence tags: HIGH = primary source or two independent sources; MEDIUM = one credible
source; LOW = single forum post, inference, or conflicting reports.

## Summary

1. Honda itself CAN field-reprogram ("reflash"/"update") these ECM/PCMs through the
   16-pin DLC. The period tool is the hand-held Honda Interface Module (HIM, EQS05A35570),
   later the HDS + MVCI, and for independents a SAE J2534 pass-thru running Honda's "Pass
   Thru"/J2534 Rewrite app. Communication is over K-line (DLC pin 7) - these cars are
   pre-CAN. [HIGH]
2. The ECM is flash-reprogrammable but write-guarded: "You cannot update an ECM/PCM with
   the program it already has. It will only accept a new program," and "Do not turn the
   ignition switch OFF while updating... the ECM/PCM can be damaged." [HIGH] A real
   2003-05 Civic Hybrid PGM-FI update (SB 11-043) proves field reflash with program IDs
   and 37805-xxx software part numbers. [HIGH]
3. Part-number structure matters: 37820-xxx-xxx is the hardware assembly; 37805-xxx-xxx
   is the software/program load; a "Program ID" (e.g. ZA3270) names a calibration. [HIGH]
4. The ECM microcontroller is the OKI nX-8/500S family: K20A3 (PND/PRB) confirmed to need
   the "66Q592" processor [HIGH]; D17 (PLR-Q71) listed with "66q591" [MEDIUM]. The "Q" is
   the flash-EEPROM die; non-Q parts are mask ROM. No public Ghidra/IDA module exists for
   nX-8/500S (asm662 only covers the older nX-8/200). This is the single biggest REA
   tooling gap. [HIGH]
5. No aftermarket tool reflashes the stock D17 or 02-04 K20A3 ECU over the DLC. Hondata
   K-Pro solders a USB daughterboard into a donor K-series ECU; KTuner uses a swap/modified
   ECU; the old Hondata "reflash" was mail-in (ECU + key + immobilizer). The factory D17
   ECU "cannot be converted." That universal work-around is itself strong evidence the
   factory flash path is locked to the dealer protocol. [HIGH]
6. The best public pre-CAN Honda K-line flash reference is the Honda *motorcycle* Keihin
   protocol (open source: HondaECU + eculib). Exact framing, wake-up, erase/write/recover
   command bytes are known. Whether the car ECM answers the same command set is UNVERIFIED
   and is the key thing to sniff from a dealer tool. [HIGH for moto bytes / LOW for car]
7. Non-reflash writes the dealer tool does - clear DTC, ECM reset/idle learn, immobilizer
   code rewrite on ECM swap - are well documented and within reach of a device that speaks
   the Honda K-line diagnostic protocol. Immobilizer coupling (key+transponder) is the main
   practical trap for ECM swaps and bench work. [HIGH]
8. "DLC pin 12 = write-enable" is UNCONFIRMED. WEN (WRITE ENABLE SIGNAL) is a genuine Honda
   ECM terminal (E30 on the Fit/Jazz family), but no source found places it on DLC pin 12.
   [ECM terminal HIGH / pin-12 mapping LOW]

## Findings

### Q1. Are the ECM/PCMs flash-reprogrammable through the DLC by Honda's procedure?

Yes - Honda reprograms them in the field, over the DLC, with its own tools. The question is
not *whether* the hardware can be rewritten but *who holds the protocol*.

Tool timeline (all drive the same DLC / K-line):

| Era | Tool | Role | Source conf. |
|-----|------|------|------|
| ~2001-2006 | Honda Interface Module (HIM), P/N EQS05A35570 | hand-held "update the ECM/PCM" device; rental | Jazz SM [HIGH] |
| ~2001-2006 | Honda PGM Tester (GNA600 era) | diagnostics, immobilizer rewrite, DTC/reset | Jazz SM [HIGH] |
| 2006+ | HDS laptop + MVCI (OTCH5528542) | "CM Update Mode" standalone reflash | SB 01-026 [HIGH] |
| 2004+ | J2534 pass-thru + Honda "Pass Thru"/J2534 Rewrite | independent-shop reflash via techinfo.honda.com | DG guide [HIGH] |

Honda service-manual language (Fit/Jazz GD mirror, same corporate procedure as Civic):
"ECM/PCM Updating and Substitution for Testing," Special Tools Required: "Honda Interface
Module ((HIM) EQS05A35570)"; "To ensure the latest program is installed, do an ECM/PCM
update whenever the ECM/PCM is substituted or replaced"; "You cannot update an ECM/PCM
with the program it already has. It will only accept a new program"; "Do not turn the
ignition switch OFF while updating the ECM/PCM. If you turn the ignition switch OFF, the
ECM/PCM can be damaged."
(https://hondziarze.duckdns.org/jazz/A00/HTML/EW/SAA3EEWK72100000000BBAT00.HTML) [HIGH]

General reprogramming procedure bulletin = Honda **01-023 "Updating Control Units/Modules"**
(Acura twin **01-026**). The Acura 01-026 (2011 rev, read in full as PDF) states: "Most 2002
and later Acuras have reprogrammable control units/modules," and walks the MVCI "CM Update
Mode -> Reprogramming" flow, selecting PGM-FI / AT (CVT) / IMA Motor / IMA Battery.
(https://static.nhtsa.gov/odi/tsbs/2013/SB-10039532-9101.pdf) [HIGH]

Proof a 7th-gen Civic PGM-FI load is field-reflashable: **SB 11-043 / SB-10042649-3282**,
"2003-05 Civic Hybrid: ECM Update to Improve DTC P1420 and P2000 Detection Threshold."
Corrective action "Update the PGM-FI software"; HDS 3.001.009+, CM Update DB 27-JUL-2011+;
table of Program IDs / Program P/Ns (ZA3270 / 37805-PZA-327 ... ZAA660 / 37805-PZA-A66);
failed part 37820-PZA-317. (https://static.nhtsa.gov/odi/tsbs/2012/SB-10042649-3282.pdf) [HIGH]

Civic (non-hybrid) software-update bulletins exist across 2001-2005 (NHTSA summaries, exact
text not posted):
- 02-011 (2002-02-26, NHTSA 629521) "ECM/PCM software update"
- 02-044 (2002-08-06, NHTSA 633576) "ECM/PCM software"
- #1203 (2003-12-16) and ServiceNews 01-01-2004 "updating REPLACEMENT ECM/PCM software"
- SB-02-011 re-issued 2011-10-22 "software update for ECM/PCM with HDS"
(https://www.carcomplaints.com/Honda/Civic/2002/tsbs/) [MEDIUM]

Note the "REPLACEMENT ECM/PCM software" wording: several bulletins are about loading current
software into a *new service part*, consistent with the SM rule that a module will only
accept a *different* program than it already holds.

DLC / connector facts [HIGH unless noted]:
- Comms line is K-line = DLC pin 7 (ISO 9141-2 / ISO 14230 KWP2000). Pre-CAN; no pin-6/14
  CAN on these cars.
- ECM K-LINE terminal = connector E23 (RED/WHT) on the Fit/Jazz-generation layout, which
  Hondata's K-Pro docs confirm is shared by the K-series ECU (SCS = E29 on both). [HIGH]
- J2534 reflash uses the standard J1962/DLC and warns to parallel a charger/second battery
  "to stabilize the voltage" (DG Technologies Honda Reprogramming User Guide, 2011). [HIGH]
- "DLC pin 12 = write-enable (WEN)": NOT confirmed. WEN is real but sits at ECM terminal E30
  (see Q7/pinout). No source maps it to DLC pin 12. [LOW]

Market differences: the reprogramming framework is corporate-wide (Honda/Acura, all markets)
but the *software catalog* is market/trim specific (KA=49-state, KL=California, European and
JDM each have their own 37805/calibration IDs). European 1.7 CTDi diesel (Bosch) is a wholly
different ECU family and protocol, out of this dimension's D17/K20 scope. [MEDIUM]

### Q2. The reprogramming protocol (pre-CAN K-line) and file formats

Honda's own car K-line reflash command set for the D17/K20 era is **not publicly reverse
engineered**. The closest public, documented pre-CAN Honda flash protocol is the Honda
**motorcycle** Keihin K-line implementation, plus a community car-ECU *diagnostic* K-line
capture. These are the templates REA would start from.

(a) Honda motorcycle Keihin K-line *flash* protocol - open source, read from
`eculib/honda.py` and `HondaECU .../kline.py`. Frame and bytes are HIGH confidence (read
directly from source); applicability to the *car* ECM is LOW (untested).

| Element | Value (verbatim from source) |
|---|---|
| Frame | `[type] [len] [data...] [checksum]`, `msgsize = 0x02 + ml + dl` |
| Checksum | `checksum8bitHonda = ((sum(bytes) ^ 0xFF) + 1) & 0xFF` (two's complement of sum) |
| Wake-up | FTDI bitbang: write 0x00, sleep 70 ms, write 0x01, sleep 130 ms |
| Init (write mode) | `0x7d 01 01 01`, `...02`, `...03`, then `0x7d 01 02 "PGM"`, `0x7d 01 03 "-FI"` |
| Erase | `0x7e 01 02`, `01 03 00 00`, `01 0b 00 00 00 ff ff ff`, `01 0e 01 90`, `01 01 01`, `01 04 ff` |
| Erase status poll | `0x7e 01 05` |
| Write block | `0x7e 01 06` + 2-byte addr + data + next-addr + two checksums (c1=checksum8bit, c2=checksum8bitHonda); block 128 bytes, falls back to 64 |
| Finalize | `0x7e 01 09`, `01 0a`, `01 0c`, `01 0d` (ok when `info[2][1]==0x0f`) |
| Recover (bootloader) | `do_init_recover`: `0x7b 00 01 01/02/03`, `0x7b 00 02 76 03 17`, `0x7b 00 03 75 05 13` |
| Security | `0x27 0xe0` + 7-byte password chunks |
| Read flash | `pgmfi_read_flash_bytes(location, 12)` (K-line ROM read EXISTS on moto) |
| State detect | id `0x72 71 00`; recover-old `0x7d 01 01 00`; recover-new `0x7b 00 01 01` |

Sources: https://raw.githubusercontent.com/Pongmi04/eculib/master/eculib/honda.py ;
https://raw.githubusercontent.com/Adeen117/HondaECU/master/src/threads/kline.py [HIGH for bytes]

(b) Car Keihin K-line *diagnostic* protocol (AutotronicCommunity, example ECU "K25"):
baud 10400; wake-up `FE 04 72 8C` -> expect `0E 04 72 7C`; init `72 05 00 F0 99` ->
`02 04 00 FA`; frame `72 [len] [table-query 71] [table#] [cksum]`; checksum = 0x100 - (sum
& 0xFF); must complete within 2 s or the ECU re-sleeps. This is a *read/table* protocol
(the 0x72/0x71 family), matching the motorcycle "get tables" path - useful for live data
and DTCs, not shown to flash.
(https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol) [HIGH for bytes]

Observation worth flagging to the protocol dimension: the motorcycle `detect_ecu_state`
uses `0x72 71 00` (ECM ID) and the car diagnostic protocol uses `72 .. 71 ..` tables - the
0x72 request header and 0x71 table-query appear in both, so the Keihin K-line *diagnostic*
layer is very likely shared between bikes and these cars. The *flash* layer (0x7d/0x7e/0x7b)
is what is unproven on the car. [MEDIUM inference]

(c) Later CAN-era `.rwd` update files (2006+, for contrast; what the D17/K20 is NOT):
- `.rwd`/`.gz` are "only partial and encrypted ROM file" - they omit the bootloader region.
- Header indicator `5a 0d 0a` (0x5A format); a `0x31` format exists but is disabled in the
  public tool. CAN module address built as `18DA<nn>F1`.
- Cipher = 256-entry byte substitution table computed from 3 key bytes and 3 operators
  (^, &, |, +, -, *, /); "Keys Bytes can be found directly in the ecu ROM.bin by searching
  for them." Trailing 4-byte file checksum (not the bin checksum).
- Known cipher example in comments: `KEIHIN_SH7058_1Mb: (((i ^ k2) + k1) - k0) & 0xFF`.
Sources: https://github.com/bouletmarc/HondaReflashTool/blob/main/Class_RWD.cs ;
https://github.com/bouletmarc/HondaReflashTool ; cross-ref gregjhogan/rwd-xray. [HIGH]
These confirm the modern Honda flash is CAN + Renesas SH + encrypted-delta - a different
world from the pre-CAN OKI K-line cars, so rwd tooling does NOT transfer to the D17/K20.

Where calibration/update files come from: Honda's techinfo.honda.com (Service Express / iN)
serves the CM Update database the HDS/MVCI/J2534 app downloads; files are keyed to Program
IDs. No open repository of D17/K20A3 factory loads was found. [MEDIUM]

### Q3. ECU hardware internals (D17 and K20A3) - the REA/Ghidra setup

| Unit | Example P/N | MCU (marking) | MCU family | ROM type | Ghidra/IDA | Conf. |
|---|---|---|---|---|---|---|
| K20A3 Si (US) | 37820-PNF | OKI "66Q592" (L66Q592) | OKI nX-8/500S (ML66Q592) | flash EEPROM ("Q") | none public | HIGH (Hondata) |
| K20 RSX base/CRV donor | 37820-PND, PPA | OKI 66Q592 / 66592 | OKI nX-8/500S | flash (Q) / mask | none public | HIGH |
| D17 Civic (non-Si) | 37820-PLR, PLM | OKI "66q591" (PLR-Q71 row) | OKI nX-8/500(S) ML66591/592 | flash or mask | none public | MEDIUM (one source) |
| Pre-2001 OBD2A | 37820-P.. | OKI 66507 | OKI nX-8/500 | mask ROM | 66507 DASM exists | HIGH |
| OBD1 | P28/P72 | OKI 66207 | OKI nX-8/200 | ext EPROM/mask | asm662 | HIGH |

Key hardware facts:
- Hondata states a K-Pro donor ECU "must be PRB A01-A12" and, explicitly, "If it does not
  have the 66Q592 processor then it can not be used"; the 2005-06 PRB-A13..A16 "has a
  different microprocessor and cannot have a K-Pro installed." So the 02-04 K20 uses OKI
  66Q592; the 05-06 switched MCU. (https://www.hondata.mobi/forum/viewtopic.php?p=92466 ;
  https://hondata.com/kpro) [HIGH]
- OKI MSM66591 / ML66592 datasheet facts: 16-bit, "OKI original CPU core nX-8/500S," ~35.7-
  41.7 ns/state (14/12 MHz). "Q" variants (MSM66Q591 / ML66Q592) are the flash-EEPROM
  versions; the flash die exposes a TEST/Vpp high-voltage programming pin and uses the EA
  pin to enable a RAM-monitor mode. Internal RAM in the 8 KB region. Exact internal flash
  size for the Honda part not pinned down from public pages (datasheet mirrors were
  403/anti-bot). (datasheetspdf.com OKI ML66592 / MSM66591 pages) [MEDIUM]
- pgmfi.org EcuDefinitionCodes maps Oz-DM OBD2-K units: PLR-Q71 Civic "66q591," PND-Q51
  Integra "66592," PPA-Q53 CRV-2002-K24 "66Q592," PAA-A06 CRV "66Q592." Confirms the whole
  2001-2005 Civic/RSX/CRV set is OKI 591/592-class. (https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/EcuDefinitionCodes.html) [MEDIUM]
- EEPROM for immobilizer / learned data: the ECM carries the immobilizer pairing; community
  reports of swapping a small 8-pin serial EEPROM (93C56-class) between ECMs to move
  immobilizer data. Not confirmed on an actual D17 teardown here. [LOW]

REA/Ghidra implication (important): the nX-8/500S core has **no** Ghidra SLEIGH module and
**no** IDA processor module in public hands. Existing open tools (asm662 = nX-8/200; 66507
DASM = one older OBD2A part) do not cover 591/592. A firmware-RE effort must first build a
disassembler for nX-8/500S - the instruction manual ("66k Assembler Docs"/OKI nX-8/500S
users manual) plus asm662's nX-8/200 tables are the starting references. This is the single
highest-value REA task in this dimension. [HIGH]

### Q4. Reading the firmware out (dumping ROM)

1. Dealer-protocol memory read over K-line: EXISTS on Honda motorcycles
   (`pgmfi_read_flash_bytes`, 12-byte reads). For the *car* D17/K20, no public K-line ROM
   read; RomRaider contributors note K-line read was never advertised for Renesas-based
   Honda ECUs (and these OKI parts are even less documented). Realistically unknown until a
   dealer tool is sniffed or the command tried. [MEDIUM]
2. Bench / silicon routes:
   - OKI "EA-pin not latched at reset" trick: pgmfi's MCU readers (66207, 66507) wire an
     I/O pin to EA, jump above the mask-ROM window, flip EA, and stream ROM out the serial
     port. Documented for the older parts; "present on all MCUs we have seen to date,"
     so plausibly adaptable to 591/592 but unproven. (https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/McuReaders.html) [MEDIUM]
   - Flash-version serial/parallel programming mode: the ML66Q592 flash die has a TEST/Vpp
     programming interface - a bench Vpp programmer could read/write the internal flash
     directly. This is the OKI analogue of the Renesas boot-mode route. No turnkey adapter
     found for this exact part. [MEDIUM]
   - There is no BDM/JTAG on this MCU family; chip-off + external reader is the fallback for
     a mask-ROM (non-Q) part. [MEDIUM]
3. Public ROM dumps / definitions: pgmfi hosts older bins (P13, 37820-P11-G00) but **no**
   public stock D17/K20A3 car dumps, and no TunerPro XDF / RomRaider defs for these cars
   were found. (The motorcycle repo has many 38770-xxx .bin + .xdf - useful only as format
   examples.) Checksum algorithm for the car ROM is unknown (motorcycle uses the two
   8-bit sums above). [MEDIUM]

### Q5. Third-party tuning/flashing products - what their approach reveals

| Product | Platform | How it gets in | What it tells us |
|---|---|---|---|
| Hondata K-Pro (v1-v4) | 02-04 K20 (RSX/EP3/CRV donor); D17 Civic via donor K-ECU | USB daughterboard soldered into the ECU; "not reversible"; onboard datalog RAM | stock K20 ECU is NOT OBD-reflashable by aftermarket; they bypass the factory flash entirely |
| Hondata "reflash" (older) | PNF Civic Si, RSX | MAIL-IN: send ECU + key + immobilizer; Hondata reprograms, returns; "one-time," not tunable after | even Hondata could not do it in-car over OBD; needed bench access + immobilizer |
| KTuner | 05-06 RSX ECU "physically modified by a dealer"; 02-04 need board/swap; D17 = "specially developed ECU" replacement ($699) | in-ECU board or replacement ECU + jumper harness + custom cam gear | same story: no stock-ECU OBD reflash for this gen; 05-06 MCU change needed |
| HTS (BouletMarc) | OBD0/OBD1 only (v2.20 "purely for OBD1") | ext EPROM / socket | does not touch D17/K20 at all |
| HondaReflashTool (BouletMarc) | 2006-2010+ CAN, Renesas SH7055/58 | J2534 CAN, encrypted .rwd | CAN-era only; confirms the pre-CAN OKI cars are a separate problem |

"The stock D17 ECU cannot be converted to a programmable ECU" (Hondata). Every commercial
path replaces or piggybacks the factory ECU. Convergent evidence that the factory D17/K20A3
flash entry is closed to third parties over the DLC - either locked behind the dealer
protocol/seed or simply never reverse-engineered. (https://hondata.com/kpro ;
https://ktuner.com/rsx/ ; https://www.pencomputing.com/rsx/tuning_21st_century_style.html) [HIGH]

One concrete tell from a teardown-salvage blog: the K-Pro is a daughterboard over the ECU
motherboard, and moving it between boards requires desoldering the main connector - i.e.
the stock processor stays, the daughterboard intercepts it. (98luder.wordpress.com) [MEDIUM]

### Q6. Non-reflash persistent writes the dealer tool performs

All within reach of a device that speaks the Honda K-line diagnostic protocol (the 0x72/
0x71 family), no flash required:

| Write | Procedure (2001-2005 class) | Conf. |
|---|---|---|
| Clear DTCs + freeze data | scan tool/PGM Tester "clear DTC," or pull No. 20 (15A) under-dash fuse 60 s | HIGH |
| ECM/PCM reset (learned values) | same as clear; then idle-learn | HIGH |
| Idle learn | warm to fans-cycle-twice, all accessories off, idle 10 min | HIGH |
| Immobilizer code rewrite on ECM replace | HDS: IMMOBI -> Immobilizer Setup -> Replace ECM/PCM; enter 4-digit code from iN Immobilizer Code Inquiry; a programmed key must be present | HIGH (SB 00-098) |
| ECM substitution for testing | swap known-good ECM, rewrite immobilizer with PGM Tester, restore original + rewrite again | HIGH |
| CKP/crank pattern clear & learn | not confirmed as a distinct service on this gen (more a later-model routine) | LOW |
| VIN / odometer storage | VIN+odometer entered into HDS/iN during immobilizer session; ECM stores immobilizer pairing, not a user-writable VIN | MEDIUM |

Sources: Honda SB 00-098 (2014 rev) replace-ECM/PCM procedure (immobilizer rewrite with HDS,
idle learn) - read as PDF (https://static.nhtsa.gov/odi/tsbs/2014/MC-10129307-9999.pdf);
Fit/Jazz "How to Troubleshoot" (reset, substitution, HIM update)
(https://hondziarze.duckdns.org/jazz/A00/HTML/EW/SAA3EEWK72100000000BBAT00.HTML). [HIGH]

### Q7. Risk assessment

- Bricking during flash: interrupting a HIM/MVCI update damages the ECM ("Do not turn the
  ignition switch OFF while updating"). Recovery per SB 01-026: keep ignition ON (II), add
  a jumper battery (not a charger), disconnect/reboot the tool, reconnect, retry. The
  motorcycle protocol has an explicit recover/bootloader mode (0x7b), implying the OKI
  bootloader survives a bad app-flash; the car's recovery likewise relies on re-entering the
  tool's update. Low battery (<10 V) corrupts the write. [HIGH for the caution / MEDIUM for
  bootloader survivability on the car]
- Immobilizer coupling: the ECM is part of the immobilizer system; a swapped or
  bench-swapped ECM has a different immobilizer code and "the engine will not start" until
  re-registered with HDS using a programmed key + the car's immobilizer unit. This is why
  Hondata's mail-in reflash required the ECU **and** the key **and** the immobilizer. For
  the owner's own device, clearing/registering immobilizer codes needs the iN code or a
  valid seed - a likely hard wall. [HIGH]
- Spare junkyard ECM for bench work: practical and cheap. 2001 D17 service ECMs are
  discontinued but used units are common (37820-PLM-xxx; KA=49-state A-suffix, KL=California
  L-suffix). For *reading firmware* you only need to power the board, not start a car, so
  the immobilizer is irrelevant on the bench. Bench power-up pinout (Fit/Jazz-family, D17
  likely similar but verify against the Civic SM):
  - Power: IGP1 = A3, IGP2 = A2 (YEL/BLK) to +12 V
  - Grounds: PG1 = A5, PG2 = A4 (BLK/BRN), LG1 = A24, LG2 = A23, LG3 = E3
  - Main relay drive: MRLY = E7; IMO FPR = E1
  - K-line (comms) = E23 (RED/WHT) -> DLC pin 7
  - SCS = E29; WEN (write enable) = E30 (RED/BLU, ~0 V ign ON); IMOCD = E27
  (https://hondziarze.duckdns.org/jazz/A00/HTML/S6/SAA5ES6A20300000000CAAT11.HTML) [HIGH for
  Jazz family / MEDIUM that D17 connector matches]

## Conflicts and uncertainties

- "DLC pin 12 = WEN" (coordinator lead): PARTLY WRONG as stated. WEN is a real ECM terminal
  (E30), not shown on DLC pin 12 anywhere I found. The write-enable concept is genuine; its
  DLC routing is unconfirmed. [resolve with the Civic SM DLC wiring diagram]
- "K-series ECUs cannot be reflashed over the DLC and need a daughterboard" (lead):
  CONFIRMED for aftermarket (Hondata/KTuner all bypass the stock flash). But Honda's own
  dealer tool DOES reflash the same ECUs over the DLC (SB 11-043 Hybrid; 01-023 framework).
  So "cannot be reflashed over the DLC" is true only for *non-dealer* tools lacking the
  protocol/seed. [resolved]
- "D17 ECUs are not tunable" (lead): CONFIRMED as a practical statement (no aftermarket
  reflash; KTuner/K-Pro use replacement ECUs). Does not mean the silicon can't be rewritten
  - Honda rewrites it. [resolved]
- Exact D17 MCU: only one source (pgmfi, Oz-market PLR-Q71 = "66q591"). US PLM/PLR not
  directly teardown-confirmed here. K20A3 = 66Q592 is solid. [MEDIUM]
- Whether the car ECM accepts the motorcycle 0x7d/0x7e flash commands: UNKNOWN. The 0x72/
  0x71 diagnostic layer looks shared; the flash layer is untested on cars. [open]
- Internal flash size of ML66Q592 in the Honda part: not pinned (datasheet mirrors blocked).
  PCMflash lists the Fit/Jazz OKI unit as "ML66Q592 / 192KB," a plausible figure for the
  Civic-class part too. [LOW]

## Open questions (and how to close them)

1. Does the D17/K20A3 ECM answer a K-line ROM-read or flash command over DLC pin 7? ->
   Sniff a dealer HDS/HIM/J2534 session against a junkyard ECM on the bench (bus capture =
   TShark/logic analyzer; a prime REA target). Compare captured frames to the motorcycle
   0x7d/0x7e set.
2. Exact factory flash command set, seed/key, block layout, checksum for these OKI cars ->
   reverse-engineer a dumped ROM (needs the nX-8/500S disassembler) and/or sniff the tool.
3. Which DLC pin (if any) carries WEN / a programming-enable on the 7th-gen Civic -> read
   the Civic factory SM DLC + ECM wiring diagram (the specific service-manual page).
4. Can the ML66Q592 internal flash be read via its TEST/Vpp serial mode on the bench, and
   what is its true size/map -> obtain the full OKI MSM66591/ML66592 users manual
   (FEUL66591-66592-01) and try a Vpp programmer on a scrap ECM.
5. Is there any public stock D17/K20A3 ROM dump or definition anywhere (non-English forums,
   drive2.ru, Chinese tuning sites) -> targeted non-English search (web-search budget was
   exhausted this round; carry to next round).
6. Build the Ghidra SLEIGH module for OKI nX-8/500S -> the enabling tool for all firmware RE
   here; seed from the nX-8/500S instruction manual + asm662 nX-8/200 opcode tables.

## Sources

- https://static.nhtsa.gov/odi/tsbs/2014/MC-10129307-9999.pdf - Honda SB 00-098 (2014 rev), 2001 Civic PCM product update; replace + immobilizer rewrite w/HDS, idle learn, PLM part numbers. PRIMARY, read as PDF.
- https://static.nhtsa.gov/odi/tsbs/2013/SB-10039532-9101.pdf - Acura SB 01-026 "Updating Control Units/Modules" (twin of Honda 01-023); MVCI CM Update Mode flow, failure recovery. PRIMARY, read as PDF.
- https://static.nhtsa.gov/odi/tsbs/2012/SB-10042649-3282.pdf - Honda SB 11-043, 2003-05 Civic Hybrid PGM-FI reflash; Program IDs + 37805 software part numbers. PRIMARY, read as PDF.
- https://www.dgtech.com/wp-content/uploads/2016/05/DG-Honda-VSI-2534-Reprogramming-User-Guide.pdf - DG Technologies Honda J2534 reprogramming walkthrough; HIM-emulating "Pass Thru" app, PGM-FI/A-T select, DLC/battery cautions. PRIMARY, read as PDF.
- https://hondziarze.duckdns.org/jazz/A00/HTML/EW/SAA3EEWK72100000000BBAT00.HTML - Honda SM "How to Troubleshoot Fuel and Emissions": ECM/PCM Updating (HIM EQS05A35570), reset, substitution, immobilizer rewrite. PRIMARY mirror.
- https://hondziarze.duckdns.org/jazz/A00/HTML/S6/SAA5ES6A20300000000CAAT11.HTML - Honda SM ECM/PCM terminal tables (connectors A/E): K-LINE E23, SCS E29, WEN E30, IMOCD E27, power/grounds. PRIMARY mirror.
- https://hondata.com/kpro - Hondata K-Pro D17 page: "stock D17 ECU cannot be converted," donor list, USB, cam wheel. VENDOR.
- https://www.hondata.mobi/forum/viewtopic.php?p=92466 - Hondata staff: donor "must have 66Q592 processor"; 05-06 PRB "different microprocessor." FORUM (vendor staff).
- https://www.hondata.mobi/help/kmanager/ecuconnectors.htm - K-Pro ECU connector reference (SCS E29 etc.). VENDOR.
- https://ktuner.com/rsx/ - KTuner RSX/EP3: supported 05-06 ECU part numbers, dealer-modified ECU, immobilizer/VIN caveat. VENDOR.
- https://tim-tran-uyxl.squarespace.com/.../ktuner-revision-1-01-05-honda-civic-d17a-ecu-package - KTuner D17 = "specially developed ECU" replacement, $699, jumper harness + cam gear. VENDOR.
- https://www.pencomputing.com/rsx/tuning_21st_century_style.html - K-Pro = USB daughterboard soldered into stock ECU; older reflash mailed in with key+immobilizer. ARTICLE.
- https://raw.githubusercontent.com/Pongmi04/eculib/master/eculib/honda.py - Honda moto K-line lib: frame, checksum8bitHonda, wake-up, 0x7d init, 0x7e erase, 0x7b recover, 0x27 password. SOURCE, read raw.
- https://raw.githubusercontent.com/Adeen117/HondaECU/master/src/threads/kline.py - HondaECU flash/read worker: 0x7e 01 06 write block, read_flash, do_erase, do_password. SOURCE, read raw.
- https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol - car Keihin K-line diag: baud 10400, wake-up FE 04 72 8C, init 72 05 00 F0 99, frame/checksum. SOURCE.
- https://github.com/bouletmarc/HondaReflashTool/blob/main/Class_RWD.cs - CAN-era .rwd parser: 5A header, substitution-table cipher from 3 keys/3 ops, trailing checksum. SOURCE.
- https://github.com/bouletmarc/HondaReflashTool - README: .rwd "partial and encrypted," 0x27 01/41 modes, SH7055/58 Keihin, 2006-2010 Civic coverage. SOURCE.
- https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/EcuDefinitionCodes.html - pgmfi ECU codes: PLR-Q71 "66q591," PND/PPA "66Q592/66592." COMMUNITY REF.
- https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/McuReaders.html - OKI EA-pin ROM-read trick for 66207/66507. COMMUNITY REF.
- https://github.com/a1k0n/asm662 - OKI 66201/66207/66301 (nX-8/200) assembler/disassembler, pre-1996 Honda ECUs; no 591/592. SOURCE.
- https://ecutools.eu/chip-tuning/pcmflash/ - PCMflash module 13 "Honda/Acura K-Line": Fit/Jazz "Keihin OKI ML66Q592/192KB," Accord SH7055, virtual reading. VENDOR list.
- https://www.carcomplaints.com/Honda/Civic/2002/tsbs/ - NHTSA TSB index: Civic 02-011/02-044/1203/01012004 ECM-PCM software bulletins. AGGREGATOR.
- https://datasheetspdf.com/pdf/767252/OKI/ML66592/1 - OKI ML66592 (nX-8/500S) identity, flash-version/Vpp notes. DATASHEET (mirror, partial).
