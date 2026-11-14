# EP3-01 - The US-market 2002 Civic Si (EP3 / K20A3) electrical layout, control units, and how it differs from the US D17 sedan plan

> Research notes, EP3 refinement round. Date: 2026-11-14. Scope: the US EP3 hatchback's own DLC wiring, ECM connector E terminals, the set of diagnosable control units and which bus each is on, the immobilizer architecture (architecture only), and the EP3-specific service functions (CKP pattern, idle learn, VIN, EPS torque-sensor learn). Builds on round-1 notes `01-vehicle-and-control-units.md`, `02-dlc-pinout-and-legislated-obd2.md`, and `04-srs-abs-eps-immobilizer-body.md`; corrects them where the EP3 differs.
>
> Confidence tags: **HIGH** = primary source or two independent sources agree; **MEDIUM** = one credible source (or a sister-platform factory manual applied to the EP3); **LOW** = single forum post, inference, or conflicting reports.
>
> Method caveat: the EP3 factory manual is mirrored at charm.li and manualslib (`manualslib.com/manual/804994/Honda-Ep3`, the 1183-page 2002-2003 EP3 book) and `manualslib.com/manual/813630/Honda-Civic` (7th-gen Civic). **Both block the fetch tool, and charm.li's terminal tables are page images.** Almost every factory fact below was read through search-engine result snippets of those pages (tagged "(manual, via snippet)") and corroborated against a text-readable sister manual (the GD Fit/Jazz HTML at hondziarze.duckdns.org) or the rusEFI 125-pin transcription. No raw byte/scope capture of an EP3 was obtained; all protocol timing is inherited from the other dimensions.

## Summary

1. The US EP3 Si is a **single-K-line car exactly like the D17**: Honda's own job aid lists "2001-05 Civic - ALL (includes GX and Si)" as "K-Line only." One diagnostic serial bus reaches the DLC - the K-line on pin 7, ECM terminal **E23** - and it is confirmed EP3-specific (the EP3 manual's PGM-FI DLC test checks continuity DLC-7 to ECM E23). HIGH.
2. The round-1 D17 pin table (**4,5 grounds; 7 K-line; 9 SCS; 12 WEN; 14 gray/immobilizer; 16 +B; 6/15 empty**) holds for the EP3. The ECM is connector **E (31P)**, not B. E23=K-line(pin7), E24=multiplex SEFMJ (NOT a diagnostic line), E27=immobilizer comm, E29=SCS(pin9), E30=WEN(pin12). HIGH/MEDIUM.
3. **Major correction to the round-1 premise.** SRS, ABS and EPS are NOT on a separate "DIAG-H" pin-14 line - they share the **K-line on pin 7** with the ECM. The 7th-gen Civic manual literally says to "repair short in the wire between the DLC (K-line) and the ABS modulator-control unit, the SRS unit." Pin 14 (gray, ~5 V) is **immobilizer-related only**, and the "DIAG-H" name is unverified for this car. There is likely **no second diagnostic serial bus** on the EP3. MEDIUM-HIGH.
4. Control units the US EP3 Si has: **K20A3 ECM** (manual, no separate TCM - US Si is 5MT only, ECM P/N **37820-PNF-A05**); **ABS modulator-control unit** (present on the US Si, modulator 57110-S5T-A50); **SRS unit** with side airbags + OPDS/seat-occupant sensing; **EPS control unit** (column EPS - the Si is the trim that HAS it); **immobilizer control unit-receiver**; and the **multiplex (MICU) unit** (body electrical, reached by its own connector, not the DLC). HIGH/MEDIUM.
5. Immobilizer architecture (US EP3): a dedicated **immobilizer control unit-receiver** (7P) reads the transponder; its 7P#2 goes to **ECM E27** (RED/BLU); the **ECM holds the key pairing** and authorizes start - same model as the US D17, NOT the MICU/93C46 story some attach to UK/Euro hatchbacks. Re-registering a key or a replacement ECM legitimately needs a Honda PGM Tester with the engine-immobilizer program card plus a control key + recording key, and (modern access) NASTF/VSP credentials. HIGH.
6. EP3-specific service functions: the K20A3 **adds a crank/CKP pattern clear + learn** on top of idle learn (the D17 1.7 had idle learn only) - a real difference. The **02-04 ECM does NOT store the VIN** (unlike the 04-05 D17 1.7). EPS has a **torque-sensor neutral-position learn** (needed after gearbox/EPS-motor/EPS-unit replacement; ambient >20 C/68 F). HIGH/MEDIUM.
7. Build-plan consequences: a single 12 V K-line transceiver on pin 7 reaches every HDS-visible unit (ECM, ABS, SRS, EPS, gauge); the "configurable DIAG-H channel" for SRS/EPS can probably be dropped or demoted to a pin-14 immobilizer sniffer; the immobilizer is ECM-mediated, so key/ECM registration stays a legitimate Honda-code operation and is out of scope to defeat.

## Findings

### Q1 - The EP3 Si 16-pin DLC and the K20A3 ECM connector

The 7th-gen generic-OBD DLC is a J1962 type-A 16-pin connector. Round-1's D17 cavity set is confirmed for the EP3; the EP3-specific corroboration is below.

| DLC pin | Wire | Honda name / function | Other end | EP3 confidence | Source |
|---|---|---|---|---|---|
| 4 | BLK | Body ground | Body ground | HIGH (fn) | EP3 PGM-FI DLC test jumps 4+9 for SCS ([manual p.335, via snippet](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=335)); Jazz SCS method "DLC terminals No.4 and No.9" ([Jazz EPS](https://hondziarze.duckdns.org/jazz/A00/HTML/21/SAA2E21K72200000000BBAT00.HTML)) |
| 5 | BRN/YEL | LG3 logic ground | ECM E3 | HIGH | round-1 file 02; EP3 test "voltage between DLC No.5 and No.7" (manual p.335, via snippet) |
| 6 | empty | no CAN | - | HIGH | [Honda job aid](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf) |
| 7 | LT BLU | **K-LINE** - the one diagnostic serial bus | **ECM E23**; also ABS, SRS, EPS, gauge | HIGH | EP3 manual: continuity "DLC terminal No.7 ... ECM terminal E23 ... K-LINE (LT BLU)" (manual p.335, via snippet); rusEFI 125-var "E23 K-LINE ... DLC pin 7" ([rusEFI](https://wiki.rusefi.com/Honda-125-variations/)) |
| 9 | BRN | **SCS** service check signal (ground it for blink codes) | ECM E29 (+ SRS/ABS/EPS SCS inputs) | HIGH | HR K-swap harness "E29 BRN ... controls the SCS DATA LINK CONNECTOR" ([guide](https://device.report/manual/9957255)); Jazz 4+9 jumper method |
| 12 | RED/WHT | **WEN** write-enable | ECM E30 | MEDIUM | K-series rywire pinout; Fit manual E30 "WEN (WRITE ENABLE SIGNAL)" (round-1 file 02) |
| 14 | GRY | ~5 V, **immobilizer-related** (exact function unconfirmed; "DIAG-H" name unverified) | immobilizer circuit | MEDIUM (fn), LOW (exact dest) | round-1 file 02 + 04; see Q2/Q3. NOT SRS/EPS |
| 15 | empty | no L-line | - | MEDIUM | round-1 file 02 |
| 16 | WHT/RED | +B unswitched battery | No.9 BACK UP fuse | HIGH | EP3 manual DLC terminal No.16 WHT/RED (round-1 file 02) |

ECM connector E (31P) - the K20A3 ECM uses the 125-pin A/B/C/D/E family; connector **E** carries the diagnostic and immobilizer terminals. "Connector B" (from one 2005-Civic source in round 1) is wrong for the K20A3 - the K-line is on E23.

| ECM term | Signal | Goes to | Confidence | Source |
|---|---|---|---|---|
| E23 | K-LINE (LT BLU) | DLC pin 7 | HIGH | EP3 manual p.335 (via snippet); rusEFI |
| E24 | Multiplex **SEFMJ** comm line (NOT K-line, does NOT carry scan-tool traffic) | Multiplex (MICU) unit | HIGH | rusEFI 125-var "E24 Multiplex SEFMJ ... Civic Si 2.0L" ([rusEFI](https://wiki.rusefi.com/Honda-125-variations/)); rusEFI forum "E24 SEFMJ ... did not pass SEFMJ traffic" |
| E27 | Immobilizer communication (RED/BLU) | immobilizer control unit-receiver 7P#2 | HIGH | EP3 immobilizer test "continuity between the immobilizer control unit-receiver 7P connector No.2 terminal and ECM terminal E27 ... open in the RED/BLU wire" ([manual p.1037, via snippet](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=1037)); RSX 05/06 immobilizer diagram lists E27 |
| E29 | SCS (BRN) | DLC pin 9 | HIGH | HR harness note (above) |
| E30 | WEN write-enable | DLC pin 12 | MEDIUM | K-series/Fit (above) |
| E3 | LG3 logic ground | DLC pin 5 | HIGH | round-1 file 02 |

### Q2 - Which control units the EP3 Si has, and which bus each is on

**Headline:** every unit the Honda PGM Tester / HDS can reach on this car hangs off the single **K-line (pin 7)**. Honda's job aid says all HDS communication on the 2001-05 Civic is K-line, and the 7th-gen Civic manual's DLC-short troubleshooting names the units on that one wire together.

| Unit | Present on US EP3 Si? | Bus to the tester | DTC format | Confidence | Source |
|---|---|---|---|---|---|
| PGM-FI **K20A3 ECM** (manual; **no separate TCM**, US Si is 5MT-only) | Yes | **K-line pin 7 (E23)** | P-codes + 2-digit MIL blink via SCS | HIGH | ECM 37820-PNF-A05 for "2002-2005 Civic Si 5-speed K20A3 EP3" ([partrequest](https://www.partrequest.com/auto-parts-detail/2002-2005-honda-civic-si-5-speed-k20a3-ep3-ecu-ecm-326017750763)); round-1 file 01 (PNF = US Si) |
| **ABS modulator-control unit** | Yes (standard on US Si) | **K-line pin 7** | 2-digit (31-38 solenoid, 51, 53...) | MEDIUM-HIGH | modulator 57110-S5T-A50 listed for "2002-2003 Civic 3-Door SI, KA 5MT" ([hondapartsnow](https://www.hondapartsnow.com/genuine/honda~modulator~assy~57110-s5t-a50.html)); EP3 ABS pages ([manual p.677/679](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=677)) |
| **SRS unit** (+ side airbags, OPDS / seat-occupant classification) | Yes | **K-line pin 7** | 2-digit blink or PGM Tester SRS menu | MEDIUM-HIGH | 7th-gen Civic manual: "repair short in the wire between the DLC (K-line) and the ABS modulator-control unit, **the SRS unit**" ([manual p.643, via snippet](https://www.manualslib.com/manual/813630/Honda-Civic.html?page=643)); charm.li Si lists Seat Occupant Classification Module + impact/safing sensors |
| **EPS control unit** (column electric power steering - the Si is the US trim that HAS it) | Yes | **K-line pin 7** (plus its own SCS line + EPS indicator for blink codes) | 2-digit (12,17,18,32,51,71...) | MEDIUM | EP3 DLC test checks "DLC terminal 7 to the ECM **and EPS** wiring" (manual p.335, via snippet); EPS read via 16P DLC with PGM Tester ([Jazz EPS](https://hondziarze.duckdns.org/jazz/A00/HTML/21/SAA2E21K72200000000BBAT00.HTML)) |
| **Immobilizer control unit-receiver** | Yes | ECM-mediated (receiver 7P#2 -> ECM E27; tester reaches ECM on pin 7). Pin 14 may be a direct immobilizer line. | n/a | HIGH (arch), LOW (pin 14) | manual p.1037 (above); see Q3 |
| **Multiplex control unit (MICU)** | Yes | **Not on the DLC** - own inspection connector; talks to ECM on E24 SEFMJ | n/a | HIGH | rusEFI (E24 SEFMJ); round-1 file 02 (MICU own connector) |
| Gauge/meter assembly | Yes | K-line pin 7 (Honda job aid "continuity between the PCM and gauge control module") | - | MEDIUM | [Honda job aid](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf) |

EPS connector C (20P) partial pinout gathered (for the sniffer/tap plan): **C3 & C11** torque-sensor signal pair (to torque sensor 6P); **C8** SCS (BRN) to the DLC; **C17/C19** engine-speed (NEP)-type signal shared with the ECM (~6 V at idle, "33 Hz per 1,000 rpm"); C1 ground. No K-line terminal was seen in snippets, so EPS-on-pin-7 is inferred from the DLC-circuit text, not read off the EPS table. ([manual p.516 title, via snippet](https://www.manualslib.com/manual/813630/Honda-Civic.html?page=516); [s2ki OBD wiring](https://www.s2ki.com/forums/uk-ireland-s2000-community-25/obd-wiring-connections-1187064/))

### Q3 - EP3 immobilizer architecture (architecture + legitimate re-registration only)

Confirmed flow (EP3/RSX manual + general Honda bulletin): the **transponder key** is energised by a ring antenna in the **immobilizer control unit-receiver**; the receiver sends the key's ID to the **ECM/PCM over E27**; the ECM compares it against **key codes stored in ECM memory** and only then enables fuel + ignition. This is the same "ECM holds the pairing" model as the US D17 sedans, with a dedicated receiver unit at the ignition.

| Element | Fact | Confidence | Source |
|---|---|---|---|
| Transponder reader | immobilizer control unit-receiver, 7P connector | HIGH | [manual p.1037 (via snippet)](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=1037) |
| Receiver 7P pinout (from Fit sister manual) | #1 ground (BRN/YEL), **#2 -> ECM/PCM E27 (RED/BLU)**, #6 IG battery (YEL/BLK, via PGM-FI main relay 1), #7 constant battery (WHT/RED) | MEDIUM (Fit, applied to EP3; #2->E27 confirmed EP3) | [Jazz immobilizer](https://hondziarze.duckdns.org/jazz/A00/HTML/K0/SAA2EK0J46636200000FAAT00.HTML) |
| Where pairing lives | **ECM/PCM memory** (not the MICU). ECM stores up to ~4-5 key codes | HIGH (ECM), LOW (count) | [techinfo Control Module Replacement](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_TOOLS_CONTROL_MODULE_REPLACEMENT.htm); Honda Civic bulletin "stores up to 5 key codes" |
| ECM participation | start authorization line is **E27**; the MICU/multiplex (E24 SEFMJ) is body electrical, not engine-start auth, on this car | HIGH / MEDIUM | manual p.1037; rusEFI |
| Legitimate re-registration | after replacing the ECM or the receiver you must "rewrite the unit with a Honda PGM Tester"; dealer needs the car + a **control key + recording key + PGM Tester with the engine-immobilizer program card**; modern access needs a Security-Professional subscription and **NASTF/VSP** registration and a per-car code | HIGH | manual p.1037; [techinfo](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_TOOLS_CONTROL_MODULE_REPLACEMENT.htm) |

On the "UK hatchback = MICU + 93C46 EEPROM holds start authorization" hypothesis: **not supported for the US EP3.** The US EP3 service manual documents the ECM-stored pairing with a dedicated receiver (E27). The multiplex/MICU exists but is reached on E24 SEFMJ and governs body electrical, not immobilizer start-enable. (Any 93C46-in-MICU behaviour, if real, would be a Euro-5-door/market-specific variant - unverified here and flagged as an open question. The EP3 is Swindon-built for all markets, so the hardware is likely common and the difference, if any, is calibration.) Research deliberately stopped at architecture; no bypass method was sought or recorded.

### Q4 - EP3-specific DTCs and service functions

| Function | EP3 answer | Confidence | Source |
|---|---|---|---|
| **Crank / CKP pattern clear + learn** | **Yes - the K20 adds it** (the D17 1.7 had idle learn only). HDS/PGM: ADJUSTMENT menu -> CRANK PATTERN -> CRANK PATTERN LEARNING. A PCM reset erases everything **except** the CKP pattern; a generic clear wipes it, so after a full clear the order is **idle learn -> CKP pattern learn -> test drive**. Manual learn: warm engine, decelerate throttle-closed in gear (e.g. 2,500->1,000 and 5,000->3,000 rpm) several times. Incomplete CKP learn can throw misfire codes (P0302/P0303); check the "CKP PULSER F/B LEARN" data-list value. | MEDIUM-HIGH | [Honda CKP/idle sequence (via snippet)](https://www.manualslib.com/manual/813630/Honda-Civic.html?page=346); K20 crank wheel "12 teeth + 1 sync, Hall" ([Haltech](https://support.haltech.com/portal/en/kb/articles/thomas-te)) |
| **Idle learn** | Yes. Needed after battery disconnect, ECM replace/disconnect, or ECM reset; **not** needed after merely erasing DTCs. Loads off, reset ECM (or battery off ~15 min), confirm "idle learn complete" on the data list. EP3 ECM reset = pull **No.6 ECU(ECM) 15 A fuse** (under-hood box) for 10 s. | HIGH | manual p.346 (via snippet); EP3 fuse method (honda-tech, via snippet) |
| **ECM stores a VIN?** | **No - the 02-04 K20 ECM does not store the VIN** (unlike 04-05 D17 1.7). During HDS "replace ECU" a VIN is entered for the immobilizer step only; Hondata: pre-05 ECUs don't record it. | MEDIUM-HIGH | [Hondata forum](https://www.hondata.mobi/forum/viewtopic.php?p=12317); Scribd immobilizer-reprogram note ("VIN not stored in the 02-04 ECU") |
| **EPS torque-sensor neutral learn** | Yes. PGM Tester / HDS EPS menu -> Miscellaneous Test -> Torque Sensor Learn. Required after the steering gearbox, EPS motor, or EPS control unit is replaced; **ambient must be >20 C / 68 F**; clearing DTCs does not change the stored neutral position. | MEDIUM (EP3 has DTC 12 T/SIG; the 20 C spec is from a later-gen bulletin) | EP3 "DTC 12: Torque Sensor T/SIG" ([manual p.551](https://manualslib.com/manual/804994/Honda-Ep3.html?page=551)); [Honda EPS bulletin SB-10039384](https://static.nhtsa.gov/odi/tsbs/2014/SB-10039384-3097.pdf) |
| EP3 EPS DTC set (2-digit) | 12 torque-sensor T/SIG; 17/18 torque-sensor Vcc1/Vcc2; 32 current sensor; 51-xx torque-sensor circuit; 71-xx motor-angle-sensor neutral-position learning | MEDIUM | manual p.551; Civic Si EPS code charts (via snippet) |
| Powertrain DTCs vs D17 | K20A3 i-VTEC adds cam/VTC-related and dual-cam-sensor codes absent on the SOHC D17 (e.g. VTC, both CMP). Full EP3 P-code table not transcribed this round (images). | LOW (not transcribed) | - |

### Q5 - Differences from the D17 plan the build plan should absorb

1. **Same single K-line, confirmed on the actual car.** Pin 7 -> ECM E23 is now EP3-verified, not inferred. No CAN, no second legislated bus.
2. **Drop/demote the "configurable DIAG-H channel for SRS/EPS."** SRS, ABS and EPS are on the K-line (pin 7), not pin 14. The hardware needs **one** 12 V K-line transceiver to reach ECM + ABS + SRS + EPS + gauge. Keep pin 14 only as a **passive ~5 V immobilizer sniffer** (its exact role is unconfirmed), not a driven general-diagnostic channel.
3. **ECM is connector E (31P).** Target E23/E27/E29/E30 - not "connector B."
4. **E24 SEFMJ is a trap:** it is a 5 V multiplex line to the MICU, physically near the ECM, that does NOT carry scan-tool traffic. Don't mistake it for a second K-line when probing.
5. **Immobilizer is ECM-held (E27) with a dedicated receiver**, like the D17 - so engine-start authorization decoding/registration is a legitimate Honda-code operation and stays out of scope; reading immobilizer DTCs/status is in scope.
6. **Add CKP (crank) pattern clear + learn to the service-function list** (ECM, EPS). The D17 1.7 only needed idle learn; the K20A3 needs idle learn *and* CKP pattern learn, in that order, after a full clear.
7. **Do not implement "read/write VIN from ECM" for this car** - the 02-04 K20 ECM doesn't store one. (The 04-05 D17 1.7 does; keep that model-year-gated.)
8. **EPS gets a torque-sensor neutral-position learn** (temperature-gated >20 C) as a service function, plus its own SCS/indicator blink-code path.
9. ECM P/N to hard-code for this car: **37820-PNF-A05**, 5MT, K20A3 (PNF = US Si; RSX is PND/PNE - different calibration, do not cross-flash).

## Conflicts and uncertainties

- **Pin 14 exact destination/function (LOW).** rywire calls it "+5 V from immobilizer control unit pin 3 (probably for rewriting immobilizer codes)" - note "probably." honda-tech measured 5 V. The Fit immobilizer-receiver 7P table I read did not expose a DLC-14 line. 8th-gen Civic puts pin 14 on ECM A37 as "immobilizer communication." So pin 14 is immobilizer-ish on the EP3 but its wire-level endpoint is unproven. Treat as a passive line to characterise on the bench.
- **EPS on the K-line: inferred, not read off the EPS table (MEDIUM).** The EP3 DLC-circuit text ties DLC-7 to "the ECM and EPS wiring," and all HDS links are K-line, but I never saw a K-line terminal in the EPS connector-C snippet. Possible alternative: early EPS is PGM-Tester-readable only via a path I didn't capture. Verify by probing EPS connector C for continuity to DLC pin 7.
- **EPS SCS DLC endpoint conflict (LOW).** One s2ki thread cites EPS SCS (BRN, EPS pin 8) going to **DLC pin 1**, another to **DLC pin 9**; a 2003 manual is cited for pin 1. DLC pin 1 is otherwise not listed as populated on the EP3. Likely a mis-read of pin 9, but flagged.
- **K-line idle voltage (LOW).** One EP3 snippet said "voltage between DLC No.5 and No.7 ... 0 V (ign on)," whereas round-1's CR-V/Fit manual said the K-line idles at battery ("8.5 V or more"). The "0 V" reading is probably a short-detection step or a snippet garble; the K-line should idle high. Re-read on the car.
- **Key-code storage count (LOW).** "Up to 4" (one Honda bike manual) vs "up to 5" (Honda Civic bulletin). Minor.
- **CKP pattern learn is confirmed from 2006-2009 Civic Si text + K20 trigger geometry (MEDIUM for the exact EP3 menu).** The 02-03 EP3 book's CRANK PATTERN page wasn't opened (image); the menu wording may differ on the PGM Tester vs HDS.
- **UK/Euro MICU + 93C46 immobilizer claim: unverified (LOW).** No EP3 source supports MICU-held start authorization; the US EP3 manual is ECM-held. Left as an open question rather than denied outright, since the Euro 5-door (EU-series) is a different body.

## Open questions (and the cheapest FREE way to close each)

1. **Pin 14's real endpoint and signal.** Cheapest: on the owner's car, back-probe DLC-14 with a DMM/scope (idle voltage, any serial activity with key on) and buzz continuity DLC-14 to the immobilizer receiver 7P and to ECM E-connector. Zero cost, definitive.
2. **Is EPS actually on K-line pin 7?** Cheapest: continuity from EPS connector C to DLC pin 7; or watch for EPS frames while sniffing pin 7 with the passive channel. Alternatively read the EP3 manual EPS connector-C table image on charm.li by eye (free, just not machine-readable here).
3. **Full EP3 connector-E table + EP3 P-code/EPS/ABS/SRS DTC lists.** Cheapest: open the charm.li 2002 Civic Si pages (ECM "Diagrams > Connector Views", EPS "Pinout Values", ABS/SRS DTC pages) and the manualslib EP3 pages (247, 516, 551, 677, 1037) as images in a browser - they are free, just blocked to this fetch tool. rusEFI's `OEM-Docs/Honda/2003-2005-Honda-Civic-Si-2.0L-ECM-pinout` and `2002-2006-Acura-RSX-ECM-pinouts` folders hold transcribed pin-list images too.
4. **Confirm CKP pattern menu wording on the EP3-era PGM Tester** (vs HDS "CRANK PATTERN LEARNING"). Cheapest: the published HDS-derived protocol database (GPL) and the EP3 manual PGM-FI "Adjustment" image.
5. **Whether any immobilizer traffic appears on pin 14 vs pin 7 during a key cycle.** Cheapest: passive sniff both pins through one key-on/crank cycle once the device exists.
6. **Euro-5-door immobilizer variance (MICU/93C46).** Cheapest: charm.li has no Euro book; check the free Euro Civic (EU) wiring on rusEFI/forums only if the owner's car ever turns out to be an import - not relevant to the US EP3.

## Sources

- [rusEFI "Honda 125-pin variations" transcription catalogue](https://wiki.rusefi.com/Honda-125-variations/) - E23 K-LINE->DLC pin 7, E24 Multiplex SEFMJ on Civic Si 2.0L; distinguishes 02-04 RSX/Civic-Si from 05-06 RSX.
- [rusEFI "Vault of Honda OEM Information"](https://wiki.rusefi.com/Vault-Of-Honda-OEM) and [Hellen-125-Honda-K](https://wiki.rusefi.com/Hellen-125-Honda-K/) - points to the RSX and Civic-Si 2.0L ECM pin-list image folders.
- [charm.li 2002 Civic Si service manual root](https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/) - exact factory section tree (DLC, ECM, Information Bus/SEFMJ, Steering Control Module, Air Bag, EBCM/ABS, Keyless/Communications); terminal tables are page images.
- [manualslib EP3 service manual (2002-2003, 1183 pp.)](https://www.manualslib.com/manual/804994/Honda-Ep3.html) - p.247 ECM connector E (31P), p.335 PGM-FI DLC circuit (DLC-7<->E23 K-LINE LT BLU), p.516/551/557 EPS, p.677/679 ABS, p.1037/1038 immobilizer (7P#2<->ECM E27 RED/BLU). Read via search snippets.
- [manualslib 7th-gen "Honda Civic" manual](https://www.manualslib.com/manual/813630/Honda-Civic.html) - p.346 ECM idle learn / CKP sequence, p.512 EPS SCS method, p.516 EPS connector C (20P), p.643 ABS (DLC K-line short also hits "the SRS unit"). Via snippets.
- [hondziarze Fit/Jazz HTML manual - EPS troubleshooting](https://hondziarze.duckdns.org/jazz/A00/HTML/21/SAA2E21K72200000000BBAT00.HTML), [SRS troubleshooting](https://hondziarze.duckdns.org/jazz/A00/HTML/00/SAA2E00K79100000000BBAT00.HTML), [immobilizer troubleshooting](https://hondziarze.duckdns.org/jazz/A00/HTML/K0/SAA2EK0J46636200000FAAT00.HTML) - text-readable sister platform; SCS 4+9 method, receiver 7P#2->ECM E27, PGM-Tester rewrite.
- [American Honda job aid "DLC Circuit Troubleshooting" (2017)](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf) - 2001-05 Civic ALL (incl. Si) is K-Line only; K-line links PCM and gauge.
- [techinfo.honda.com "Control Module Replacement"](https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_TOOLS_CONTROL_MODULE_REPLACEMENT.htm) - immobilizer must be registered to a new PCM; VSP/NASTF + Security-Professional subscription.
- [Honda EPS torque-sensor bulletin SB-10039384](https://static.nhtsa.gov/odi/tsbs/2014/SB-10039384-3097.pdf) - torque sensor neutral learn, >20 C/68 F, relearn after gearbox/motor/unit replacement.
- [Haltech "Honda K Series Engine"](https://support.haltech.com/portal/en/kb/articles/thomas-te) - K20 crank trigger 12+1 Hall; K20A3 i-VTEC differs from K20A2.
- [Hondata forum - immobilizer/VIN threads](https://www.hondata.mobi/forum/viewtopic.php?p=12317) - 02-04 K20 ECU does not store the VIN; use HDS "replace ECU" with any 02-04 RSX VIN for the immobilizer step.
- [partrequest ECM 37820-PNF-A05](https://www.partrequest.com/auto-parts-detail/2002-2005-honda-civic-si-5-speed-k20a3-ep3-ecu-ecm-326017750763) and [hondapartsnow ABS modulator 57110-S5T-A50](https://www.hondapartsnow.com/genuine/honda~modulator~assy~57110-s5t-a50.html) - US Si 5MT K20A3 ECM P/N; KA 5MT Si ABS modulator (ABS present).
- [Hybrid Racing K-swap wiring guide](https://device.report/manual/9957255) - E29 BRN "controls the SCS DATA LINK CONNECTOR"; E23 BLU/YEL "controls the DATA LINK CONNECTOR."
- [tegiwa K-Tuned Immobilizer/Multiplexor Bypass (EP3)](https://tegiwa.com/products/k-tuned-immobilizer-multiplexor-bypass-honda-civic-ep3-ecu) - community confirmation the EP3 ECU expects both immobilizer and multiplex signals (architecture context only).
- [s2ki "OBD wiring connections"](https://www.s2ki.com/forums/uk-ireland-s2000-community-25/obd-wiring-connections-1187064/) - EPS SCS (BRN, EPS pin 8) to DLC (pin 1 vs 9 conflict).
- [pencomputing RSX ECU connectors (Helms-sourced)](https://pencomputing.com/rsx) - A31/B24/C22/D17/E31 connector layout; ECU must be matched to immobilizer+key on replacement.
