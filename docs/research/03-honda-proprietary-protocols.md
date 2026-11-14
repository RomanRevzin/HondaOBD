# Honda proprietary (dealer-tool) diagnostic protocols on pre-CAN cars (2001-2005 Civic focus)

Research dimension 03 of 9. Research date: 2026-11-14. Web research only; nothing was installed or executed.

Confidence tags: **HIGH** = primary source or two independent sources agree; **MEDIUM** = one credible source; **LOW** = single forum post, inference, or conflicting reports.

Limits of this round (read before relying on anything below):

- The shared web-search quota ran out part-way through. Not searched at all: ScanGauge XGauge / Torque custom PIDs, drive2.ru, Japanese blogs, Autel/Launch protocol lists, Tactrix documentation.
- honda-tech.com, driveaccord.net, preludeonline.com and insightcentral.net redirect automated readers to a paid gateway that was not used. Statements attributed to those forums come from search-result excerpts only, never from the full thread.
- SAE standards are paywalled (only the table of contents of J2534-2 was seen). The two Honda R&D specifications on Scribd could not be read beyond the cover / contents / foreword of one of them.
- Byte sequences were read through a summarising fetcher. Every Honda-format frame quoted in this file was re-checked by hand against the checksum rule (all bytes sum to 0 modulo 256) and every one is self-consistent, which makes transcription errors unlikely but not impossible.

## Summary

1. Honda's pre-CAN dealer-tool communication has three named layers: **92Hm/2** (1992, single-wire bus called DIAG-H, chassis/body ECUs and non-OBD engine ECUs), **95Hm-CARB** (PGM-FI on the ISO 9141-2 K-line of OBD2 cars, shared by the generic scan tool and the PGM Tester) and **H/99** (extension of 95Hm-CARB with a generic mode 'C' and a Honda-tester mode 'H', which also lets other ECUs share the K-line). MEDIUM.
2. DIAG-H is a physical line separate from the K-line. SAE J2534-2 gives it its own protocol ID (`HONDA_DIAGH_PS`, 0x800B) and its own electrical specification; Honda's own interface lists K-Line and Honda Diag H as separate channels. HIGH.
3. On the 7th-generation Civic the DLC carries both lines. A 2004 Civic wiring diagram shows: pin 7 K-line to ECM/PCM and ABS modulator-control unit; pin 14 (GRY) DIAG-H to SRS unit, immobilizer control unit-receiver and EPS control unit; pin 12 (RED/WHT) WEN to the ECM/PCM; pin 9 SCS. HIGH for 2004 US cars, MEDIUM for a 2002 car until checked on the vehicle.
4. The Honda message format is `[header/command] [length = whole message] [data ...] [checksum]`, checksum = two's complement of the byte sum. DIAG-H runs at 9600 baud 8N1. HIGH for the format, HIGH for the baud rate.
5. A 2026 open-source project that decoded Honda HDS 3.105.12 publishes the dealer tool's per-protocol-row wake-up, keep-awake and stop messages and command bytes for cars (for example wake-up `FE 04 91 6D`, stop `AD 04 F1 5E`, commands A0 read-out / A1 clear / A2 info-read). It has never been run on a car. MEDIUM.
6. The 1992-2000 3-pin hobby protocol (`20 05 addr count checksum`, 9600 baud) and the Honda motorcycle K-line protocol (70 ms low / 120 ms high pulse, `FE 04 72 8C`, 10400 baud) use the same framing and checksum as the car dealer protocol; the dealer tool's own table still contains a `20 05 00 02 D9` message. They are one protocol family with different header bytes, wake-up data and baud rates. HIGH for the framing match.
7. Not publicly known for 2001-2005 cars: what the ECUs answer, the meaning of the data bytes in read requests, the "H/99 pulse" timing and the baud rate of Honda mode on the K-line, which protocol the Civic ABS unit speaks on the K-line (Honda framing or the separate SAE J2809 ABS/VSA protocol), and every service function (resets, learns, actuator tests, key registration).
8. No public capture or byte-level write-up of an HDS / PGM Tester session with a 2001-2007 Honda car was found.
9. Hondash supports only OEM 1992-2001 ECUs through the 3-pin connector; a generic ELM327 cannot speak the Honda protocol. The tools documented as speaking DIAG-H are Honda-specific interfaces (HIM, GNA600, MVCI, Mongoose Pro Honda, Blue Streak iflash).
10. Cheapest reliable ways to close the gaps, in order: query the per-system properties in the published HDS-derived database; buy SAE J2534-2 (section 13 holds the DIAG-H electrical and serial specification); obtain the two Honda R&D specs (HS-E5901, HS-E5903); passively sniff DLC pins 7 and 14 while a Honda-capable tool works on the car; only then reverse engineer HDS or the ECM ROM.
11. Hardware consequence: the device needs two independent single-wire UART channels (12 V ISO 9141 K-line on pin 7; DIAG-H on pin 14, which most of the evidence puts at about 5 V), millisecond-accurate line pulsing, 9600 and 10400 baud, plus switched SCS (pin 9) and WEN (pin 12) lines.

## Findings

### 1. Names and taxonomy

#### 1.1 Protocol names

| Name | What it is | Line and DLC pin | Physical layer | Baud | Initialisation | Control units | Confidence |
|---|---|---|---|---|---|---|---|
| 92Hm/2 | Honda diagnostic communication protocol established in 1992 for the PGM Tester. Honda R&D specification HS-E5901, Rev. 0, dated 2003-03-27, 12 pages | DIAG-H single-wire bus. 3-pin DLC: pin 1. 16-pin DLC without CAN: pin 14 | Single wire, bidirectional UART, tester plus at most seven ECUs | 9600 8N1 | Wake-up message, preceded by a line pulse on H/99-capable units; details in section 2 | Engine ECU on non-OBD models, transmission, immobilizer, SRS, ABS/VSA, EPS, EV powertrain | MEDIUM-HIGH |
| 95Hm-CARB | Protocol defined for PGM-FI on OBD2 cars to meet the CARB regulation; serves both the SAE J1978 generic scan tool and the PGM Tester | K-line, DLC pin 7 | ISO 9141-2 | Generic side 10400 (ISO 9141-2); Honda side not found | Generic side ISO 9141-2 5-baud; Honda side not found | PGM-FI only; no other ECU allowed on the K-line | MEDIUM |
| H/99 | Extension of 95Hm-CARB that implements the ISO 9141-2 communication controls completely. Honda R&D specification HS-E5903, Rev. 0, 8 pages. Mode 'C' = J1978 scan tool, single request / multiple responses. Mode 'H' = PGM Tester / HDS, master/slave, single request / single response. Has a STOPCOM command to return to the inactive state | K-line, DLC pin 7 | ISO 9141-2 | Mode 'C' 10400; mode 'H' not found (10400 on motorcycles) | Mode 'H': line pulse then wake-up message (HDS calls it the "H/99 pulse") | PGM-FI plus other ECUs that share the K-line | MEDIUM |
| DIAG-H | Name of the single-wire line itself, and of the SAE J2534-2 optional feature | See 92Hm/2 | Specified in SAE J2534-2 section 13.2.6 (paywalled) | 9600 | - | - | HIGH |
| `HONDA_DIAGH_PS` | SAE J2534-2 pin-switched protocol ID, value 0x800B | Pin chosen by the application | - | - | - | - | HIGH that it exists, MEDIUM for the value |
| SAE J2809 | "Honda Diagnostic Serial Data Link Protocol - ABS/VSA System", information report of October 2007, stabilized October 2017. Applies to the ABS or VSA ECU of some Honda vehicles, explicitly not all | Not stated publicly; implemented in J2534-2 as `UART_ECHO_BYTE_PS` (0x800A) | - | Not found | Address byte, then sync byte, then key bytes; bytes are echoed (the same J2534-2 feature also carries Keyword Protocol 1281) | ABS / VSA | MEDIUM |
| "Diag X" | Listed next to Diag H by Honda's MVCI and by the Mongoose Pro Honda | Unknown | Unknown | Unknown | Unknown | Unknown | LOW (unidentified) |
| VKWP | HDS's internal name for systems that use ISO 14230 (KWP2000) on the K-line: 58 of its 182 systems, for example the gauge unit and later body ECUs | K-line | ISO 14230 | - | 5-baud or fast init | Later body systems | MEDIUM |
| "H-Line" | No evidence of a Honda protocol with this name was found | - | - | - | - | - | LOW |

Sources for the table:

- HS-E5901 cover, contents and foreword (title, date, 1992 origin, DIAG-H single wire bus, seven-ECU limit, system list, message fields "header, message length, data bytes, checksum", time-outs named T2 and T5): [Scribd 975378747](https://www.scribd.com/document/975378747/ECP-1-01-03-0327-HS-E5901-R0-92Hm2). Authenticity cannot be proven, but the content agrees with every other source. MEDIUM-HIGH.
- HS-E5903 description (H/99 extends 95Hm-CARB, modes 'H' and 'C', STOPCOM, re-establishing communication): [Scribd 975378743](https://www.scribd.com/document/975378743/ECP-1-03-03-0327-HS-E5903-R0-H-99). Body not read. MEDIUM.
- Forum excerpts that paraphrase the same Honda text (92Hm/2 for chassis/body ECUs, 95Hm-CARB on the K-line at DLC terminal 7 for PGM-FI and for both testers, no other ECU on the K-line under 95Hm-CARB, H/99 modes and K-line sharing, 3-pin DLC pin 1 = DIAG-H, pin 2 = battery or not connected, pin 3 = ground, 3-pin and 5-pin DLCs on 1992-2000 non-OBD vehicles, only HIM / GNA600 / HDS support DIAG-H): [driveaccord.net thread 24332](https://www.driveaccord.net/threads/accord-2003-obdii-or-can-bus.24332/), [preludeonline.com post 3031499](https://www.preludeonline.com/3031499-post22.html). Seen as search excerpts only. MEDIUM.
- SAE J2534-2 (December 2020 revision) table of contents: section 12 "UART Echo Byte Protocol", section 13 "Honda DIAG-H Protocol" with 13.2.2 programmable power supply, 13.2.3 pin usage, 13.2.4 Honda diagnostic connectors, 13.2.5 serial communication interface/protocol, 13.2.6 electrical characteristics; figures for the diagnostic line configuration of the 3-pin DLC model, the 5-pin DLC model, the 16-pin DLC model without CAN and the 16-pin DLC model with CAN; figures "recommended interface circuit on tester", "load circuit", "threshold voltage"; tables for serial interface parameters and transmitter / receiver electrical and timing requirements; references list SAE J2809 and SAE J2818 (Keyword Protocol 1281): [normsplash preview PDF](https://www.normsplash.com/Samples/SAE/151155732/SAE-J-2534-2-2020-en.pdf). HIGH.
- Protocol ID values 0x800A and 0x800B: header listing quoted in [J2534-Sharp discussion 13](https://github.com/BrianHumlicek/J2534-Sharp/discussions/13). MEDIUM (one listing).
- SAE J2534-2/4_0500 (January 2022, 22 pages) defines HONDA_DIAGH for API version 05.00: [SAE Mobilus](https://saemobilus.sae.org/content/J2534-2/4_5_00/), [DIN Media](https://www.dinmedia.de/en/standard/sae-j-2534-2-4-5-00/351349577). HIGH.
- SAE J2809 scope: [SAE Mobilus J2809_200710](https://saemobilus.sae.org/content/J2809_200710). HIGH for scope. UART-echo-byte timing parameters `UEB_T0_MIN` (0x8028), `UEB_T1_MAX` (0x8029), `UEB_T2_MAX` (0x802A) mapped to J2809 t0 (idle before the address byte), t1 (stimulation to sync byte) and t2 (sync byte to key byte 1): [scandoc.org J2534 reference](https://scandoc.org/en/develop/j2534/pt_ioctl_config.html). MEDIUM.
- Honda MVCI (part OTCHON3828AF) protocol list: two pin-selectable dual-wire CAN channels, single-wire CAN, K-Line, Honda Diag H, Honda Diag X; APIs J2534-1 and J2534-2; covers all Honda OBD-equipped vehicles from model year 1992: [Honda tool catalogue](https://honda.snapon.com/HondaAcura/Catalog/Product?id=OTCHON3828AF). HIGH.
- Mongoose Pro Honda: CAN, Diag H, Diag X, ISO 9141 / KWP2000, programming voltage on pin 12: [dashlogic.com](http://www.dashlogic.com/drewtech_mongoose_pro_honda). MEDIUM.
- Blue Streak iflash VCI (2016-11-24) describes Honda Diag-H as a proprietary single-wire UART protocol among its J2534-2 protocols: [bsecorp.com](https://bsecorp.com/blue-streak-electronics-announces-2534-iflash-vcis-validation-to-hondas-online-oem-diagnostic-system-hds/). MEDIUM.
- HDS system census (182 systems; 55 in the "HONDA" protocol family, 58 VKWP, the remaining 69 by subtraction on CAN; 113 systems on K-line or the older Honda protocol): [HondaDiagnosticSuite README](https://github.com/huhwhatbruh/HondaDiagnosticSuite), [diagh.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/diagh.rs), [kwp.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/kwp.rs). MEDIUM.

#### 1.2 Which line carries which unit on the 7th-generation Civic

Read from a 2004 Honda Civic "Computer Data Lines" wiring diagram (three figures: except hatchback and hybrid; hatchback; hybrid) kept in the rusEFI documentation repository, and from rusEFI's transcriptions of the 2004 Civic 1.7 L and 2003-2005 Civic Si 2.0 L ECM pinouts. Figure 1 of the diagram numbers the DLC rows the other way round (its numbers are offset by 8); the table uses the standard SAE J1962 numbers, which figure 2 and the ECM pinout files confirm.

| DLC pin | Wire colour | Signal | Connected units | Confidence |
|---|---|---|---|---|
| 4 | BLK | Body ground | Ground point | HIGH |
| 5 | BRN/YEL | Signal ground (ECM E3, LG3) | ECM/PCM, immobilizer control unit-receiver pin 1, shields | HIGH |
| 7 | LT BLU | K-LINE (ECM E23) | ECM/PCM; ABS modulator-control unit; a connector the diagram labels "Carlink interface box"; hybrid: also MCM and TCM (hybrid figure is small, MEDIUM) | HIGH |
| 9 | BRN | SCS (ECM E29) | ECM/PCM, SRS unit (C6), ABS modulator-control unit, EPS control unit | HIGH |
| 12 | RED/WHT | WEN (ECM E30) | ECM/PCM; hybrid: also MCM and TCM (MEDIUM) | HIGH |
| 14 | GRY | DIAG-H (labelled at SRS unit terminal C2) | SRS unit, immobilizer control unit-receiver pin 3, EPS control unit (C20, hatchback and hybrid figures), the "Carlink interface box" connector | HIGH |
| 16 | WHT/RED | Battery, fuse 9 (10 A) in the under-hood box | - | HIGH |

Sources: [2004-civic-datalines.pdf](https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2004-civic-datalines.pdf), [2004 Civic 1.7 L ECM pinout](https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2004-Honda-Civic-1.7L-Except-HX-GX-ECM-pinout.md), [2003-2005 Civic Si ECM pinout](https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2003-2005-Honda-Civic-Si-2.0L-ECM-pinout.md).

Supporting observations:

- HDS drives DIAG-H on DLC pin 14 by default; a vendor property (`DIAG_H_PIN`) can select pin 1, 3, 8, 9, 11, 12, 13 or 14 (presumably needed on cars where pin 14 is CAN low; that reason is an inference). MEDIUM. [diagh_serial.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/transport/diagh_serial.rs)
- A 2002 Civic owner found DLC pins 4, 5, 7, 14 and 16 populated and measured about 12 V on pin 7 and about 5 V on pin 14; a 2004 Civic owner measured about 14 V on pin 7, 4.8 V on pin 9 and 4.9 V on pin 14. LOW each (search excerpts), but they agree with the diagram. [honda-tech thread 3366115](https://honda-tech.com/forums/honda-civic-2001-2005-78/honda-civic-2002-strange-obdii-pinout-3366115/), [hondacivicforum thread 98549](https://www.hondacivicforum.com/forum/mechanical-problems-technical-chat-8/2004-16p-data-link-connector-troubleshooting-98549/)

Resulting protocol map for a 2001-2005 Civic ("16-pin DLC model without CAN" in SAE J2534-2 terms):

| Unit | Line | Protocol family | Confidence |
|---|---|---|---|
| ECM/PCM (PGM-FI, and A/T inside the PCM) | K-line, pin 7 | Legislated ISO 9141-2 plus Honda tester mode (95Hm-CARB or H/99 mode 'H') | MEDIUM |
| ABS modulator-control unit | K-line, pin 7 | Unknown: either Honda framing under H/99 (which is what allows a second ECU on the K-line) or SAE J2809 | LOW |
| SRS unit | DIAG-H, pin 14 | 92Hm/2 | MEDIUM |
| Immobilizer control unit-receiver | DIAG-H, pin 14 | 92Hm/2 (HDS immobilizer row: wake-up `FE 04 B1 4D`, command D8) | MEDIUM |
| EPS control unit (cars that have EPS) | DIAG-H, pin 14 | 92Hm/2 | MEDIUM |
| Gauge assembly, multiplex control unit | No DLC data line in the diagram | Own self-diagnosis (multiplex inspection connector); the ECM-to-multiplex SEFMJ line is not a diagnostic line | MEDIUM |

Market differences (what is known and what is not):

- US cars (D17A1/A2/A6, K20A3): covered by the diagram and pinouts above (2004 model year). HIGH for 2004, MEDIUM for 2001-2003.
- European petrol cars and the K20A2 Type R: no source found in this round. The ECM family is the same, so the same lines are likely. LOW. The 1.7 CTDi diesel uses a different engine-management supplier and may not speak Honda protocols on the engine ECU at all; unknown.
- Japanese-market cars: HS-E5901 lists engine ECUs of non-OBD models among the DIAG-H users, so on JDM cars without legislated OBD the engine ECU may sit on DIAG-H instead of (or as well as) the K-line. LOW. One data point: a 2001 JDM Acty (Keihin 37820-PFE-043) answered standard ISO 9141-2 on pin 7 (5-baud address 0x33, key words 08 08, ECU address 0x10). MEDIUM. [HondaDiagnosticSuite issue 11](https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11)

### 2. Byte-level format of the dealer-tool protocol

Source for the car-side data: [HondaDiagnosticSuite](https://github.com/huhwhatbruh/HondaDiagnosticSuite) (Rust, GPL-3.0, first DIAG-H code in release 0.3.1), which states that its framing and per-row messages come from decoded tables of HDS 3.105.12 (`protocol_properties`), that nothing was ever run on a car, and that ECU replies have never been observed. Files: [diagh.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/diagh.rs), [diagh_serial.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/transport/diagh_serial.rs), [diagh_session.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/probes/diagh_session.rs), [build-db.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/bin/build-db.rs).

| Item | Value | Confidence |
|---|---|---|
| Frame layout | byte 0 = header / command; byte 1 = length of the whole message including header, length and checksum; then data; last byte = checksum | HIGH (HDS-derived code, HS-E5901 contents page, and the hobby and motorcycle implementations all agree) |
| Checksum | Two's complement of the 8-bit sum of all preceding bytes (all bytes sum to 0 modulo 256) | HIGH |
| Length limits | Minimum length byte 3, maximum message 255 bytes | MEDIUM |
| Baud, format | 9600, 8N1 on DIAG-H (HDS connects its DIAG-H channel with 0x2580 = 9600) | HIGH (second source: 3-pin hobby loggers use 9600) |
| Echo | Single wire, so the tester reads back every byte it sends | MEDIUM |
| Wake-up | Per protocol row, see next table. HDS's interface additionally sends an "H/99 pulse" before the wake-up bytes unless the row's `ESTABLISH_MODE_H99` property turns it off | MEDIUM |
| H/99 pulse timing | Not in the HDS tables (it lives in the interface firmware). Motorcycles: 70 ms low, 120 ms high. One experimenter tried 70 ms low and 130-145 ms high on a car without a valid result | LOW for cars |
| Keep-alive | A per-row keep-awake request is sent periodically while idle; the period is not published. Motorcycle ECUs go back to sleep after about 2 s without traffic | MEDIUM / LOW for the period |
| Stop | Per-row stop message | MEDIUM |
| Time-outs | Row fields t2 = 500 ms and t5 = 300 ms (rows 1-3), t5 = 30 ms (row 14). HS-E5901 names T2 and T5 as time-outs in its error-handling chapter; their exact meaning is in that document | MEDIUM |
| Reply layout | Not published for cars. Parsed by the project as the same layout as requests (header, length, data, checksum) | LOW |

Dealer-tool protocol rows (HDS `PROTOCOL_TYPE`), as published by that project:

| Row | Wake-up | Keep-awake | Stop | t2 ms | t5 ms | Read-out cmd | Info-read cmd | Clear cmd |
|---|---|---|---|---|---|---|---|---|
| 1 | `FE 04 91 6D` | `A2 07 72 00 00 01 E4` | `AD 04 F1 5E` | 500 | 300 | - | A2 | A1 |
| 2 | `FE 04 91 6D` | `A0 05 70 01 EA` | `AD 04 F1 5E` | 500 | 300 | A0 | - | A1 |
| 3 | `A0 05 00 01 5A` then `FE 04 91 6D` | `A0 05 70 01 EA` | `AD 04 F1 5E` | 500 | 300 | A0 | - | A1 |
| 13 | `FE 04 A9 55` | `5C 06 20 19 04 61` | `5C 04 00 A0` | 0 | 0 | 5C | - | 5C (same byte reads and clears) |
| 14 | `20 05 00 02 D9` | none | none | 0 | 30 | B8 | - | B9 |
| 17 (immobilizer, deliberately left out of the project) | `FE 04 B1 4D` | not published | not published | - | - | command D8 | - | - |

Confidence for the row table: MEDIUM (single project, derived from the dealer software, never exercised on a vehicle; checksums verified by hand).

Which control unit uses which row is not in the public source text. The only published mapping: the engine ECU system `FI_` is `PROTOCOL_TYPE 1` according to a user who read the project database ([issue 11](https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11)). LOW-MEDIUM. The per-system properties are in the project's SQLite database (`systems` table, JSON `props` column).

### 3. Known command sets

Car side (dealer protocol):

| Function | What is publicly known | Confidence |
|---|---|---|
| Read data ("read-out", A0; "info-read", A2; B8 on row 14; 5C on row 13) | Command bytes only. The meaning of the data bytes (for example `70 01`, `72 00 00 01`) is not published. By analogy with the motorcycle protocol they look like sub-command / table / offset / count, which is an inference | MEDIUM for the bytes, LOW for the meaning |
| Clear DTCs (A1; B9; 5C) | Command bytes only; no confirmed full message | MEDIUM |
| DTC storage | DIAG-H systems keep DTCs as bit flags in ECU memory. HDS carries a table that maps system + memory address + bit mask to a Honda DTC main/sub code (1,984 entries, 38 with multi-bit masks). Reading DTCs therefore means reading memory bytes and testing bits | MEDIUM ([diagh_dtc.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/diagh_dtc.rs)) |
| Freeze data, temporary and history codes | Not handled by any public source | - |
| Stop communication | `AD 04 F1 5E` (rows 1-3), `5C 04 00 A0` (row 13) | MEDIUM |
| Immobilizer | Wake-up `FE 04 B1 4D`, command byte D8 (HDS name `CB_IMMOBI`); nothing else public | MEDIUM |
| Function tests, ECM reset, idle learn, crank pattern learn, key registration | Nothing public at byte level for pre-CAN cars. In HDS they are implemented in bytecode modules (139 modules, 10,021 functions decompiled by the project above; the decompiled output is not published) | - |
| Addressing of ABS / SRS / EPS / immobilizer | By physical line (K-line or DIAG-H) plus protocol row (different wake-up data byte and command bytes per row). No address table is public | LOW |

3-pin hobby protocol (1992-2000 engine ECUs), from source code: [kerpz/ArduinoHondaOBD hobd_uni.ino](https://raw.githubusercontent.com/kerpz/ArduinoHondaOBD/master/hobd_uni/hobd_uni.ino). MEDIUM-HIGH.

| Item | Value |
|---|---|
| Baud | 9600 on the single data pin, half duplex |
| Read memory | `20 05 <address> <count> <checksum>`; reply is count + 3 bytes with the data starting at byte 2 and a checksum at the end; 200 ms reply time-out |
| Clear / reset (as used for OBD mode 04 emulation) | built with command byte 0x21, length byte 0x04, then 0x01, 0x00 |
| Start-up | the sketch sends an 11-byte sequence (68 6A F5 AF BF B3 B2 C1 DB B3 E9) and waits 300 ms; no line pulse |
| Memory map examples | block reads of 16 bytes at 0x00, 0x10, 0x20, 0x30; DTC flags at 0x40; single addresses 0x02 VSS, 0x10 ECT, 0x11 IAT, 0x12 MAP, 0x13 baro, 0x14 TPS, 0x15 O2, 0x17 battery, 0x20 short trim, 0x22 long trim, 0x26 ignition advance, 0x28 IACV; ECU ID at 0x76 (commented out) |
| Tested on | P2T (OBD2, 1990s) and chipped P30 (OBD1) ECUs; README says ECUs before 2002 |

Motorcycle K-line protocol, from three independent projects: [AutotronicCommunity README](https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol), [aeaphichart RunKline.ino](https://github.com/aeaphichart/Honda_K-line/blob/master/RunKline.ino), [HondaECU fork kline.py](https://raw.githubusercontent.com/Adeen117/HondaECU/master/src/threads/kline.py). HIGH.

| Item | Value |
|---|---|
| Baud | 10400 |
| Wake-up | K-line low 70 ms, high 120 ms, then `FE 04 72 8C`; reply `0E 04 72 7C` |
| Session start | `72 05 00 F0 99`; reply `02 04 00 FA`; the ECU sleeps again after about 2 s of silence |
| Read table | `72 05 71 <table> <checksum>` (for example `72 05 71 00 18` = ECU ID, `72 05 71 11 07`); reply `02 <len> 71 <table> <data> <checksum>` |
| Read part of a table | `72 07 72 <table> <start> <end> <checksum>` (for example `72 07 72 16 00 06 F9`) |
| DTCs | read with sub-commands 0x73 / 0x74 and an index 1 to 0x0B; clear with `72 .. 60 03` |
| Flash | header 0x7D (`01 01 03` = read flash count) and 0x7E (`01 05` erase status, `01 06` write block, `01 07` sent between blocks in 64-byte mode), security header 0x27 with a password, 128-byte or 64-byte write blocks |

### 4. Family relations

| Property | 1992-2000 3-pin hobby protocol | Dealer protocol rows for cars (HDS) | Motorcycle K-line |
|---|---|---|---|
| Line | 3-pin DLC data pin (DIAG-H) | DIAG-H (pin 14 by default) or K-line under H/99 | 4-pin connector K-line |
| Baud | 9600 | 9600 on DIAG-H; unknown for Honda mode on the K-line | 10400 |
| Line pulse before first message | none in the sketch | "H/99 pulse" unless disabled for the protocol row | 70 ms low, 120 ms high |
| First message | read request, or an 11-byte start sequence | `FE 04 91 6D` (rows 1-3), `FE 04 A9 55` (row 13), `FE 04 B1 4D` (row 17), `20 05 00 02 D9` (row 14) | `FE 04 72 8C` |
| Frame and checksum | header, length, data, two's-complement checksum | same | same |
| Read header | 0x20 | A0 / A2 / 5C / B8 | 0x72 |
| Clear header | 0x21 | A1 / 5C / B9 | 0x72 with sub-command 0x60 |
| Stop | none | `AD 04 F1 5E`, `5C 04 00 A0` | none (time-out) |
| Reply header | not stated in the source | not public | 0x0E after FE, 0x02 after 0x72 |

Evidence that 2001-2005 car ECUs speak a protocol of this family:

- For: the dealer tool's own table for cars uses the identical frame and checksum, an `FE 04 xx` wake-up of the same shape as the motorcycle one, and (row 14) a message in exactly the 3-pin format (`20 05 00 02 D9`). HDS files the engine ECU system `FI_` under this family (row 1). The 2004 Civic wiring diagram labels the SRS line DIAG-H, the line 92Hm/2 runs on. MEDIUM-HIGH.
- Against / missing: nobody has published a reply from a 2001-2005 car ECU to any of these messages. The one documented attempt (2001 JDM Acty) failed because of a bad pin contact and was abandoned once standard ISO 9141-2 worked. So the evidence is indirect.
- Differences: wake-up data byte (0x91, 0xA9, 0xB1 on cars; 0x72 on motorcycles), header bytes, baud rate (9600 vs 10400), an explicit stop message on cars, and a per-system switch for the line pulse.
- Untested hypothesis (inference from the motorcycle examples only): the reply header is the low nibble of the request header (FE gives 0E, 72 gives 02). If that holds for cars, replies to A0 / A1 / A2 would start with 00 / 01 / 02. LOW.

Status of the coordinator's unverified leads:

| Lead | Result |
|---|---|
| Motorcycle init: about 70 ms low, about 120 ms high, `FE 04 72 8C`, then `72 05 00 F0 99`, 10400 baud, two's-complement checksum | Confirmed by two independent projects. HIGH |
| Car protocols may be similar | Supported: same framing and wake-up shape in the dealer tool's tables, different bytes and baud. MEDIUM |
| bouletmarc repositories contain Honda tooling for this | [HondaReflashTool](https://github.com/bouletmarc/HondaReflashTool) is CAN-only (2006 and later, security access 0x27 0x41 / 0x27 0x01). Not applicable to a 2002 Civic. MEDIUM |
| gregjhogan repositories | Not verified: the rwd-xray URL returned 404 and the search quota was gone |
| openpilot / panda | Not checked |
| hondash.net documents supported cars | It lists only OEM 1992-2001 ECUs (3-pin connector) and some chipped ECUs. It does not cover the 7th-generation Civic. MEDIUM. [Hondash compatibility](https://www.hondash.net/2022/06/compatibility.html) |
| RyanHope/HondaECU | The original repository returns 404 on 2026-11-14; forks exist (for example [Adeen117/HondaECU](https://github.com/Adeen117/HondaECU)); the protocol core is a separate library named eculib that was not located |

### 5. Public captures and reverse-engineering write-ups

| Item | What it gives | Relevance |
|---|---|---|
| [huhwhatbruh/HondaDiagnosticSuite](https://github.com/huhwhatbruh/HondaDiagnosticSuite) | Dealer protocol rows, DIAG-H framing, 9600 baud, pin 14, DTC bit-map table, ISO 14230 body-system client, all from decoded HDS 3.105.12 and interface firmware; confirmed on a 2015 Accord over CAN only; K-line and DIAG-H never run on a car | Highest. Only public source of the car-side Honda messages |
| [Issue 11 of that project](https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11) | A 2001 JDM Acty answers ISO 9141-2 (5-baud 0x33, key words 08 08, address 0x10; mode 01 PIDs 03 05 06 0B 0C 0D 0E 11 13 14 1C). The Honda wake-up question stayed open | Shows the legislated path works on a 2001 Honda and that the Honda mode is still unexplored |
| [kerpz/ArduinoHondaOBD](https://github.com/kerpz/ArduinoHondaOBD) | 3-pin protocol with memory map | Same family, older ECUs |
| [andyzsmith/HonOBDapt](https://github.com/andyzsmith/HonOBDapt) | Arduino bridge from 1992-95 Honda diagnostics to OBD II; `doc` and `src` folders not read | Possible second source for the 3-pin protocol |
| Motorcycle projects ([AutotronicCommunity](https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol), [aeaphichart/Honda_K-line](https://github.com/aeaphichart/Honda_K-line), [sophienyaa/Honda-Motorcycle-ECU-Tools](https://github.com/sophienyaa/Honda-Motorcycle-ECU-Tools), HondaECU forks) | Full wake-up, table reads, flash commands | Same family, best-documented member |
| [rusEFI SEFMJ thread](https://www.rusefi.com/forum/viewtopic.php?p=47371) and [notes](https://github.com/rusefi/rusefi_documentation/blob/master/OEM-Docs/Honda/E24-SEFMJ-white-civic-si/E24-SEFMJ-white-civic-si.md) | Logic-analyser captures and decoding of the ECM E24 to multiplex-unit line on a 2004 Civic Si | Not the diagnostic line; cross-cutting |
| [Hondash](https://www.hondash.net/2022/06/compatibility.html) | Commercial 3-pin Bluetooth scanner, 1992-2001 OEM ECUs | Does not cover the target car |
| [PVTech Honda K-line ECU protocol](https://www.pvtech-ecu-research.com/honda-kline-ecu-protocol/) | Commercial motorcycle reflash protocol (2004 and later, 4-pin adapter) | Motorcycles only |

Not found: any sniffed HDS or PGM Tester session on a 2001-2007 Civic, Accord, CR-V, Jazz/Fit, S2000 or RSX; any ELM327 or STN custom-init recipe for Honda ABS / SRS / ECM. The HondaDiagnosticSuite README states that an ELM327 cannot speak the older Honda protocol. Not searched (quota): ScanGauge XGauge and Torque definitions, drive2.ru, Japanese blogs.

### 6. DIAG-H specifically

| Property | Finding | Confidence |
|---|---|---|
| Nature | Single-wire, bidirectional, half-duplex UART bus; tester plus at most seven ECUs | MEDIUM-HIGH (HS-E5901 foreword; vendor descriptions) |
| Pin, 3-pin DLC | Pin 1 = DIAG-H, pin 2 = battery or not connected, pin 3 = ground | LOW-MEDIUM (forum excerpt) |
| Pin, 16-pin DLC without CAN | Pin 14 (GRY wire on the 2004 Civic) | HIGH for the 2004 Civic |
| Pin, 16-pin DLC with CAN | Moved to another pin per system; HDS allows 1, 3, 8, 9, 11, 12, 13 | MEDIUM |
| Voltage level | About 5 V idle. Evidence: two forum measurements on Civic pin 14 (5 V and 4.9 V); a forum statement that the HDS DIAG line is a 5 V bus unlike the 12 V K-line; the 3-pin hobby loggers wire the line straight to a 5 V Arduino pin. The authoritative values (thresholds, pull-up, load circuit, recommended tester circuit) are in SAE J2534-2 section 13.2.6, figures 10-13 and tables 40-45 | MEDIUM |
| Baud and framing | 9600 baud, 8N1; frame as in section 2 | HIGH |
| Units that answer on it (general) | Engine ECU of non-OBD models, transmission, immobilizer, SRS, ABS/VSA, EPS, EV powertrain | MEDIUM-HIGH |
| Units on it in the 2004 Civic | SRS unit, immobilizer control unit-receiver, EPS control unit | HIGH |
| Tools that speak it | HIM, GNA600, MVCI (Honda); Mongoose Pro Honda, Blue Streak iflash, and other J2534-2 interfaces that implement `HONDA_DIAGH_PS`; not ELM327-class adapters. Whether the current Denso DST-i does was not checked | MEDIUM |
| Public reverse engineering | Only HondaDiagnosticSuite (messages from HDS tables, no bus contact yet) and the 3-pin hobby loggers | MEDIUM |

Hardware warning (inference, LOW confidence, high consequence): a standard ISO 9141 K-line interface pulls its line up to battery voltage. Connecting such an interface to pin 14 could put 12 V on the DIAG-H inputs of the SRS, immobilizer and EPS units. Do not do it before the electrical specification is confirmed.

### 7. What remains unknown and the cheapest reliable way to learn it

| Unknown | Cheapest reliable way |
|---|---|
| Which protocol row, pin and H/99 switch each system (engine, A/T, ABS, SRS, EPS, immobilizer) uses | Free: open the SQLite database shipped in the HondaDiagnosticSuite repository (`hondascan/data/hondascan.db`; the release copy is encrypted with a key that is in the repository) and read `systems.props` for the family "HONDA" rows |
| DIAG-H electrical interface (levels, pull-up, thresholds, tester circuit) and serial parameters | Buy SAE J2534-2 (section 13, pages 49-56 of the December 2020 revision) or SAE J2534-2/4_0500 |
| Exact message structure, timing (T2, T5 and the rest), error handling of 92Hm/2; initialisation pulse, modes and STOPCOM of H/99 | Read HS-E5901 (12 pages) and HS-E5903 (8 pages) on Scribd with an account; look for the matching 95Hm-CARB document (probably HS-E5902, not found) |
| What the ECUs actually answer; meaning of read-out / info-read data bytes; keep-awake period; H/99 pulse timing; baud of Honda mode on the K-line | Passive sniff: logic analyser on DLC pins 7 and 14 (through a divider or a high-impedance buffer) while a Honda-capable tool reads every system on the car. A clone HIM or a Honda-capable aftermarket scanner is enough; this is the single most valuable experiment |
| Function tests, resets, learns, key registration sequences | Same sniffing session, one function at a time; cross-check against the HDS bytecode if needed |
| Whether the Civic ABS unit speaks Honda framing or SAE J2809 on the K-line | Sniff an ABS session on pin 7; buy SAE J2809 only if the capture shows an address-byte / sync / key-byte start |
| Per-call view of what HDS asks its interface to do | A logging J2534 shim DLL between HDS and a J2534-2 interface that implements `HONDA_DIAGH_PS` (open-source loggers exist: [rollsch/j2534-logger](https://github.com/rollsch/j2534-logger), [joeyoravec/j2534-logger](https://github.com/joeyoravec/j2534-logger)). Works only with interfaces driven through J2534, not with the serial HIM |
| Anything still missing after the above (memory addresses behind live data, hidden commands, flash entry) | Reverse engineer HDS (bytecode modules, `test.dll`) and the ECM ROM serial handler |

Targets for the REA toolkit in later phases:

1. `hondascan.db` from HondaDiagnosticSuite: SQLite with `systems` (family, properties), `diagh_dtc_map`, `dtc`, `ecus` tables decoded from HDS 3.105.12. No reverse engineering needed, only queries.
2. HDS 3.105.12 installation: `test.dll` (connect calls for the DIAG-H channel, ECU discovery), the protocol and system property tables, and the diagnostic bytecode modules run by HDS's internal virtual machine.
3. Interface firmware (HIM / GNA600 / MVCI "vhonda" protocol driver): holds the H/99 pulse timing and the byte-level K-line and DIAG-H drivers. The vendor documentation file `vhonda_properties.htm` names the driver properties (`DIAG_H_PIN`, `ESTABLISH_MODE_H99`, `DTC_PERMANENT`).
4. A J2534 DLL of a Honda-capable interface, to see how `HONDA_DIAGH_PS` is configured.
5. Logic-analyser captures of pins 7 and 14 (UART decode at 9600 and 10400 baud), then a frame splitter that uses the length byte and checksum.
6. The ECM/PCM ROM: find the serial receive handler and its table of accepted header bytes (FE, A0, A1, A2, AD) to learn the replies and the memory-read semantics.

### 8. Cross-cutting facts noticed (belong to other dimensions)

| Fact | Dimension | Confidence | Source |
|---|---|---|---|
| ECM terminal E30 (WEN, RED/WHT) goes to DLC pin 12; the Mongoose Pro Honda offers programming voltage on pin 12 only. Pin 12 is therefore the line to study for ECM reprogramming | Reflash, hardware | HIGH for the wiring, MEDIUM for the role | [Civic ECM pinout](https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2004-Honda-Civic-1.7L-Except-HX-GX-ECM-pinout.md), [dashlogic.com](http://www.dashlogic.com/drewtech_mongoose_pro_honda) |
| ECM terminal E24 (SEFMJ, YEL) is a separate 9600-baud single-wire link to the multiplex control unit (coolant gauge value out, A/C request in), packets end with a CRC-8 (polynomial 0x1D, init 0xFF, reflected, final XOR 0xFF). It is not a diagnostic line | Wiring, body electronics | MEDIUM | [rusEFI notes](https://github.com/rusefi/rusefi_documentation/blob/master/OEM-Docs/Honda/E24-SEFMJ-white-civic-si/E24-SEFMJ-white-civic-si.md) |
| A 2001 JDM Honda engine ECU answered ISO 9141-2 with 5-baud address 0x33, key words 08 08, ECU address 0x10 and mode 01 PIDs 03 05 06 0B 0C 0D 0E 11 13 14 1C; the first attempts failed only because the cable did not touch DLC pin 7 | Legislated OBD, hardware | MEDIUM | [issue 11](https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11) |
| Honda Australia states that a generic J2534-1 / J2534-2 interface is supported by i-HDS for reading codes only, that key and immobiliser programming need the Honda interface (PCM programming with a generic device depends on the device supplier), and that not all legacy models have a reprogrammable ECM/PCM | Reflash, tooling | HIGH | [Honda Australia PDF](https://aasra.com.au/wp-content/uploads/2022/06/Honda-diagnostic-information.pdf) |
| The Honda MVCI covers all Honda OBD-equipped vehicles from model year 1992; from model year 2012 the newer HDS generation must be used | Tooling | HIGH | [Honda tool catalogue](https://honda.snapon.com/HondaAcura/Catalog/Product?id=OTCHON3828AF) |
| HDS 3.105.12 keeps its diagnostic logic as bytecode for an internal virtual machine (139 modules, 10,021 functions) and its ECU discovery in `test.dll` | REA targets | MEDIUM | [How It Was Built](https://github.com/huhwhatbruh/HondaDiagnosticSuite/wiki/How-It-Was-Built) |
| Open-source Honda reflash tooling found so far is CAN-only (2006 and later) or motorcycle-only (K-line, headers 0x7D / 0x7E / 0x27) | Reflash | MEDIUM | [HondaReflashTool](https://github.com/bouletmarc/HondaReflashTool), [HondaECU fork](https://raw.githubusercontent.com/Adeen117/HondaECU/master/src/threads/kline.py) |
| The rusEFI documentation repository holds more 2003-2005 Civic and RSX factory-style diagrams and ECM pinouts (engine performance, power distribution, multiplex, cluster) | Wiring | HIGH | [directory listing](https://api.github.com/repos/rusefi/rusefi_documentation/contents/OEM-Docs/Honda) |

## Conflicts and uncertainties

1. **Baud rate in the same project.** `diagh.rs` says the bus initialisation and baud rate are not known; `diagh_serial.rs` says 9600 is what HDS and the interface firmware use for DIAG-H (0x2580 in the connect call). The 3-pin hobby loggers also use 9600, so 9600 is taken as correct for DIAG-H.
2. **Baud rate of Honda tester mode on the K-line.** Not found. ISO 9141-2 fixes 10400 for the generic mode; motorcycles use 10400 for the Honda mode; a general K-line blog claims Honda uses 9600 and an address byte 0x8A ([obd-cable.com](https://obd-cable.com/iso-9141-2-k-line-5-baud-handshake-guide/), LOW, unexplained).
3. **Pin 14.** One pinout site annotates Honda pin 14 as CAN low with a "WEN" note and puts an unclear "input/output for VTM-4 and immobilizer" on "13 or 14" ([pinoutguide.com](https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml)). The 2004 Civic diagram and ECM pinouts settle it for this car: pin 12 = WEN, pin 14 = DIAG-H.
4. **Pin 9 on a 2002 car.** The 2002 Civic forum report lists pins 4, 5, 7, 14, 16 only (no 9, no 12), while the 2004 diagram has 9 and 12. Either the 2001-2002 cars differ, the market differs, or the observation was incomplete.
5. **DIAG-H voltage.** A 5 V bus according to one forum statement and two measurements; another excerpt from the same forum mentions 12 V idle on a hybrid-system line. No primary value seen.
6. **H/99 pulse timing.** 70 ms / 120 ms is proven for motorcycles only. The 70 ms / 130-145 ms figures tried on a car have no stated source.
7. **95Hm-CARB or H/99 on the 2001-2005 Civic.** Unknown. The ABS unit sharing the K-line with the ECM points to H/99 (95Hm-CARB forbids a second ECU on the K-line), but the ABS could also be using SAE J2809. LOW either way.
8. **Role of "Diag X".** Unidentified. It may or may not be the J2809 ABS/VSA protocol.
9. **Row 13 and row 14.** Row 13 uses one byte (5C) for both reading and clearing; row 14 starts with a 3-pin-style read. Which units they belong to is not published. Sending row 13 requests blind could erase stored data.
10. **Reliability of the HDS-derived rows.** One project, generated with heavy tool assistance, never tested on a vehicle. The data is internally consistent and matches three independent protocol relatives, but it is still a single source.
11. **Hybrid figure.** The hybrid diagram is small; which line its ABS unit uses could not be read with confidence.

## Open questions

1. On the actual car: which DLC cavities are populated (expect 4, 5, 7, 9, 12, 14, 16) and the idle voltages on 7, 9, 12, 14. Answer: multimeter on the car.
2. Does the 2002 model year (and the car's market) match the 2004 US diagram for pins 9, 12 and 14 and for the units on each line? Answer: the 2001-2003 electrical troubleshooting manual, "Data Link Connector Circuit" page, and the SRS / ABS / EPS / immobilizer connector tables.
3. Which protocol row, pin and pulse switch does HDS use per system? Answer: query `systems.props` in `hondascan.db`.
4. What are the electrical limits and the recommended tester circuit for DIAG-H? Answer: SAE J2534-2 section 13.2.6.
5. What do T2, T5 and the other timing parameters mean, and what is the keep-awake period? Answer: HS-E5901; confirm by sniffing.
6. What is the H/99 mode 'H' entry sequence on the K-line of a car (pulse widths, baud, first message, reply)? Answer: HS-E5903; confirm by sniffing an engine-ECU session of a Honda-capable tool.
7. What does the ECM answer to `FE 04 91 6D` and to the keep-awake request, and what do the data bytes of A0 / A2 requests select? Answer: sniffing; bench test on a spare ECM; ROM disassembly.
8. Which protocol does the ABS modulator-control unit use on the K-line (Honda framing or SAE J2809, which address)? Answer: sniffing; SAE J2809.
9. How are SRS, immobilizer and EPS told apart on the shared DIAG-H wire (wake-up data byte, header byte)? Answer: database query plus sniffing.
10. Byte sequences for ECM reset, idle learn, crank pattern learn, actuator tests, SRS and ABS clears, immobilizer key registration. Answer: sniffing each function; HDS bytecode.
11. Is "Diag X" used on this car at all? Answer: database query; interface documentation.
12. Does the 1.7 CTDi engine ECU (and other European or Japanese variants) follow the same scheme? Answer: market-specific wiring diagram; test on such a car.
13. Is there a 95Hm-CARB specification (HS-E5902?) in circulation? Answer: targeted search once the quota is back.
14. Do ScanGauge / Torque communities have working Honda-mode requests for pre-CAN cars? Answer: targeted search once the quota is back.

## Sources

Standards and manufacturer documents:

- https://www.normsplash.com/Samples/SAE/151155732/SAE-J-2534-2-2020-en.pdf - SAE J2534-2 (December 2020) preview: table of contents with section 12 (UART echo byte) and section 13 (Honda DIAG-H), figure and table titles, reference list.
- https://saemobilus.sae.org/content/J2534-2/4_5_00/ - SAE J2534-2/4_0500 (January 2022), HONDA_DIAGH extended feature for API 05.00; scope only.
- https://www.dinmedia.de/en/standard/sae-j-2534-2-4-5-00/351349577 - same document, 22 pages.
- https://saemobilus.sae.org/content/J2809_200710 - SAE J2809 Honda ABS/VSA diagnostic serial protocol; scope only.
- https://www.scribd.com/document/651887020/saej2809v001 - 15-page copy of SAE J2809; body not read.
- https://www.scribd.com/document/975378747/ECP-1-01-03-0327-HS-E5901-R0-92Hm2 - Honda R&D HS-E5901 Rev. 0, 92Hm/2 protocol, 12 pages; cover, contents and foreword read.
- https://www.scribd.com/document/975378743/ECP-1-03-03-0327-HS-E5903-R0-H-99 - Honda R&D HS-E5903 Rev. 0, H/99 protocol, 8 pages; description only.
- https://honda.snapon.com/HondaAcura/Catalog/Product?id=OTCHON3828AF - Honda MVCI catalogue page with its protocol list and vehicle coverage.
- https://aasra.com.au/wp-content/uploads/2022/06/Honda-diagnostic-information.pdf - Honda Australia: i-HDS and J2534 requirements, what a generic pass-thru can and cannot do.
- https://scandoc.org/en/develop/j2534/pt_ioctl_config.html - J2534 configuration parameter reference including the UART-echo-byte timing parameters and `J1962_PINS`.
- https://github.com/BrianHumlicek/J2534-Sharp/discussions/13 - J2534-2 protocol ID listing with `HONDA_DIAGH_PS` 0x800B.

Wiring (primary for the Civic):

- https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2004-civic-datalines.pdf - 2004 Civic "Computer Data Lines" diagrams (sedan/coupe, hatchback, hybrid).
- https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2004-Honda-Civic-1.7L-Except-HX-GX-ECM-pinout.md - 2004 Civic 1.7 L ECM pinout (K-LINE E23, SCS E29, WEN E30).
- https://raw.githubusercontent.com/rusefi/rusefi_documentation/master/OEM-Docs/Honda/2003-2005-Honda-Civic-Si-2.0L-ECM-pinout.md - 2003-2005 Civic Si ECM pinout.
- https://api.github.com/repos/rusefi/rusefi_documentation/contents/OEM-Docs/Honda - directory with further Civic / RSX diagrams and pinouts.

Open-source code and project pages:

- https://github.com/huhwhatbruh/HondaDiagnosticSuite - HDS-derived open-source Honda diagnostic tool (README, wiki, issues).
- https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/diagh.rs - dealer protocol rows, framing, checksum.
- https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/transport/diagh_serial.rs - 9600 baud, pin 14, pin property, H/99 pulse note.
- https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/probes/diagh_session.rs - session probe and its warnings.
- https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/diagh_dtc.rs - bit-mapped DTC table for DIAG-H systems.
- https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/kwp.rs - ISO 14230 client for HDS's VKWP systems.
- https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/HEAD/hondascan/src/bin/build-db.rs - database schema and the HDS version it was built from.
- https://github.com/huhwhatbruh/HondaDiagnosticSuite/wiki/How-It-Was-Built - how the HDS bytecode and interface firmware were decoded.
- https://github.com/huhwhatbruh/HondaDiagnosticSuite/issues/11 - 2001 Acty K-line attempt and ISO 9141-2 result.
- https://raw.githubusercontent.com/kerpz/ArduinoHondaOBD/master/hobd_uni/hobd_uni.ino - 3-pin protocol implementation and memory map.
- https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol - motorcycle K-line wake-up, init and table reads.
- https://github.com/aeaphichart/Honda_K-line/blob/master/RunKline.ino - motorcycle K-line sketch (VFR1200F).
- https://raw.githubusercontent.com/Adeen117/HondaECU/master/src/threads/kline.py - HondaECU fork: table, DTC, flash and security commands.
- https://github.com/sophienyaa/Honda-Motorcycle-ECU-Tools/blob/master/README.md - motorcycle K-line hardware notes.
- https://github.com/bouletmarc/HondaReflashTool - CAN-only Honda reflash tool (2006 and later).
- https://github.com/jakka351/OpenJ2534 - index of J2534 projects including two logging shims.
- https://github.com/andyzsmith/HonOBDapt - 1992-95 Honda diagnostics to OBD II bridge.
- https://github.com/rusefi/rusefi/blob/master/firmware/hw_layer/kline.cpp - rusEFI handler for the ECM-to-multiplex SEFMJ line.
- https://github.com/rusefi/rusefi_documentation/blob/master/OEM-Docs/Honda/E24-SEFMJ-white-civic-si/E24-SEFMJ-white-civic-si.md - SEFMJ packet and CRC notes.
- https://www.rusefi.com/forum/viewtopic.php?p=47371 - rusEFI forum thread on the SEFMJ line.

Vendor and community pages:

- http://www.dashlogic.com/drewtech_mongoose_pro_honda - Mongoose Pro Honda protocol list.
- https://bsecorp.com/blue-streak-electronics-announces-2534-iflash-vcis-validation-to-hondas-online-oem-diagnostic-system-hds/ - iflash VCI with Honda Diag-H.
- https://www.hondash.net/2022/06/compatibility.html - Hondash compatibility list.
- https://www.pvtech-ecu-research.com/honda-kline-ecu-protocol/ - commercial motorcycle K-line reflash protocol.
- https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml - generic Honda OBD-II pinout (conflicting pin 12-14 notes).
- https://obd-cable.com/iso-9141-2-k-line-5-baud-handshake-guide/ - general K-line blog with an unverified Honda 9600 / 0x8A claim.

Forum threads known only through search excerpts (gated to automated readers):

- https://www.driveaccord.net/threads/accord-2003-obdii-or-can-bus.24332/ - quotes the Honda description of 92Hm/2, 95Hm-CARB and H/99.
- https://www.preludeonline.com/3031499-post22.html - same text, 3-pin DLC pin roles, list of DIAG-H capable tools.
- https://www.insightcentral.net/threads/honda-him-hds-project.20427/ - DIY HIM project; 5 V DIAG line and 9600 8N1 statements.
- https://honda-tech.com/forums/honda-civic-2001-2005-78/honda-civic-2002-strange-obdii-pinout-3366115/ - 2002 Civic DLC pins and voltages.
- https://www.hondacivicforum.com/forum/mechanical-problems-technical-chat-8/2004-16p-data-link-connector-troubleshooting-98549/ - 2004 Civic DLC voltages.
