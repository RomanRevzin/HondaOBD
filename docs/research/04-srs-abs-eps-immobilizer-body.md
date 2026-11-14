# 04 - Non-powertrain control units on the 2001-2005 Honda Civic: SRS, ABS, EPS, immobilizer, multiplex/body, gauges

Research date: 2026-11-14. Scope: US-market cars first (2002 Civic EX sedan D17A2 and 2002 Civic Si EP3 K20A3 were the two manuals actually read), Europe and Japan only where a source was found.

Confidence tags: **HIGH** = primary source read directly (factory manual text or ETM connector-view image) or two independent sources agree; **MEDIUM** = one credible source, or primary text seen only through the fetch tool's summariser; **LOW** = single forum post, inference, or recollection without a source in this session.

How the sources were read: the factory service information for the 2002 US Civic is mirrored as text plus images by Operation CHARM (charm.li). Procedure text was read through WebFetch (a summarising model, so wording is paraphrased; numbers were asked for verbatim). Connector-view and pinout images were fetched with WebFetch, which stored them as tool results, and were then viewed directly, so the pin tables below are first-hand readings. manualslib.com, honda-tech.com, civicforums.com and manuals.autorepairshq.com refused the fetch tool; 7thgenhonda.com redirects automated clients to a paid gateway and was not read. The shared web-search budget for this research round ran out after ten of my searches, so everything after that came from direct fetches of known URLs. Immobilizer-by-market, locksmith tooling and European/Japanese differences are therefore the weakest parts of this file.

## Summary

- The 16-pin DLC of every 2002 US Civic trim has exactly seven wires: 4 ground, 5 signal ground, 7 K-line (LT BLU), 9 SCS (BRN, circuit F28), 12 WEN write-enable (RED/WHT), 14 GRY circuit F55, 16 battery (4 A max). HIGH.
- DLC pin 14 (GRY, F55) is the **DIAG-H** line: a separate single wire that idles at 5 V and is described by Honda as "Communications with HDS". It goes to the EPS control unit (Si), the SRS unit and the immobilizer control unit-receiver. HIGH.
- The ABS modulator-control unit is **not** on DIAG-H. It hangs on the K-line of pin 7 (its terminal 11, LT BLU, "DIAG-K") together with the ECM/PCM. HIGH. The coordinator's lead "SRS/ABS over DIAG-H on pin 14" is therefore right for SRS and wrong for ABS.
- Grounding SCS (pin 9) makes each unit blink its codes on its own indicator: ABS (ground before ignition ON), EPS (ground after ignition ON), SRS (main code/sub code). Honda only documents doing this through the tester's "SCS" menu. HIGH.
- Clearing without a tester: SRS needs the separate 2-pin MES (Memory Erase Signal) connector at the driver's under-dash fuse box and a timed sequence (about 6 s, then three 4 s windows); it is not reachable from the DLC. ABS clears after 20 ignition cycles. EPS clears with SCS grounded plus a timed steering-wheel sequence, which needs only DLC pin 9. HIGH/MEDIUM.
- DTC sets found: ABS 11-18, 21, 31-38, 51-54, 61, 81 (no 41-48, 62 or 71 in the 2002 manual); EPS 12, 16, 17, 18, 22, 23, 37, 41, 42, 43, 45, 47, 50, 51, 62, 64, 66, 67, 68, 69; SRS 1-x to 15-x plus 21-x and 22-x, with separate tables per SRS unit supplier (NEC, Siemens; a Denso unit is also reported). HIGH for the numbers.
- Documented dealer-tool functions: DTC read and clear for ABS, SRS and EPS, the SCS menu, OPDS initialisation (HDS: SRS > Misc Test > adjustments > OPDS INIT), and "rewrite" of a replaced immobilizer control unit-receiver. Data lists, function tests and the key-registration dialogue are not in the service manual; they live in the tester's own documentation. MEDIUM.
- Immobilizer (2002 US): transponder key, immobilizer control unit-receiver in the steering column cover, single-wire code line IMOCD (RED/BLU) to ECM/PCM terminal E27; the ECM/PCM makes the decision and withholds the fuel pump relay ground. The unit also has the DIAG-H wire to the DLC. HIGH.
- Multiplex control unit (inside the under-dash fuse/relay box), gauge assembly and ECM/PCM share a private multiplex line. Its self-test (mode 1 codes, mode 2 input test) needs a special tool on a separate multiplex inspection connector and reports through interior-lamp blinks; nothing on the DLC. The gauge assembly and the heater control panel have button-activated self-tests. MEDIUM.
- Equipment: in the 2002 US line-up the ABS unit exists on EX and GX only (plus the Si), EBD appears on 2003-2005 units, EPS exists only on the Si, side airbags are optional and come with a different SRS unit. HIGH/MEDIUM.
- Hazards: SRS work requires battery negative off and a wait of at least three minutes, a meter that outputs 10 mA or less, and no probing of SRS unit terminals. A DIY device shares pin 14 with the SRS unit, so its DIAG-H driver must be 5 V-safe and current-limited.
- Biggest unknown for the plan: the DIAG-H protocol itself (bit rate, framing, addressing) is not described in any service-manual page; it has to come from sniffing a dealer or aftermarket tool or from reverse engineering tester software.

## Findings

### 1. Factory self-diagnosis without a dealer tool

#### 1.1 What the SCS line is

| Fact | Value | Confidence and source |
|---|---|---|
| DLC cavity | 9, BRN wire, "Circuit F28" | HIGH - [DLC connector view, EX sedan][dlc-view-ex], [DLC connector view, Si][dlc-view-si] |
| Idle level | battery voltage when "SCS not grounded" (measured at EPS control unit C8) | HIGH - [EPS connector C table][eps-pinout-img] |
| Units that listen to it | ECM/PCM (terminal E29 on K-series), SRS unit (connector C terminal 6, BRN, "DLC input/output"), EPS control unit (C8, BRN, "SCS (Service check signal)"), ABS modulator-control unit (terminal 12, BRN, labelled "DIAG-L") | HIGH for SRS and EPS ([SRS unit view][srs-view-img], [EPS unit view][eps-view-img]); MEDIUM for ABS (same wire colour, and the manual says shorting SCS makes the ABS lamp blink, but the terminal is named DIAG-L, [ABS unit view][abs-view-img]); MEDIUM for ECM pin ([Rywire K-series DLC notes][rywire]) |
| How Honda grounds it | Through the tester: PGM Tester or HDS "SCS" menu. The 2002 manual never shows a jumper at the DLC. A Honda Jazz manual of the same era tells the technician to bridge DLC pin-box terminals "No.4 and No.9". | HIGH - [SRS read][srs-read-scan], [ABS read][abs-read], [EPS read][eps-read]; MEDIUM for the 4-9 bridge on the Civic ([Jazz SRS page][jazz-srs]) |

Consequence: grounding pin 9 makes every listening unit react at once (MIL, SRS, ABS, EPS indicators). Each unit has its own rule for when the ground must be applied, see below.

#### 1.2 Per-unit procedures

| Unit | Indicator | Enter code display | Code format | Clear without tester | Confidence |
|---|---|---|---|---|---|
| ABS | ABS indicator | Ignition OFF, ground SCS, then ignition ON (II). Brake pedal must not be pressed. | Long blink = 10, short blink = 1, add them. If nothing is stored the indicator goes off for 3.6 s and comes back on. | "turning the ignition switch ON (II) then OFF at least 20 times" | HIGH - [ABS read][abs-read], [ABS on-board diagnostics][abs-obd], [ABS clear, no tool][abs-clear-noscan], [ABS clear, tool page][abs-clear-scan] |
| EPS (Si) | EPS indicator | Ignition ON (II), engine stopped, front wheels straight ahead, then ground SCS (after ignition ON). | Long blink = 10, short blink = 1. | SCS grounded first, then ignition ON and a timed steering sequence (below). | MEDIUM - [EPS read][eps-read], [EPS clear][eps-clear] |
| SRS | SRS indicator | Ignition OFF, select SCS, ignition ON. Indicator lights about 6 s, goes off, then blinks the DTC. | Main code then sub code (details below). | 2P MES connector sequence (below). | HIGH - [SRS read][srs-read-scan], [SRS erase][srs-clear] |
| Immobilizer | Immobilizer indicator (key symbol) | No SCS mode documented. | Correct key: on for 2 s, then off. Not recognised: keeps blinking after the 2 s. | Not applicable. | MEDIUM - [immobilizer troubleshooting][immo-test] |
| Heater control panel | Recirculation indicator | Fan off, hold recirculation and rear-defogger switches, ignition ON (II), release. Self-test starts after about 2 s and ends about 10 s later. | Recirculation indicator blinks the DTC number (codes 7, 8, 9, 10, 11, 12, 14, 15 exist). | Not documented on the page read. | MEDIUM - [heater self-diagnosis][hvac-read], [DTC index EX][dtc-index-ex] |

SRS blink format (2002 manual, [SRS read][srs-read-scan]):

- "When the main code is less than ten, just count the number of long blinks. Each blink is just over a second." HIGH.
- Main code 11 to 19: "you'll first see a set of four fast blinks. This set of blinks counts as 10." Main code 20 or more: "two sets of fast blinks ... count as 20". HIGH.
- Sub-code and pause timing are given only in figures, not in text; unknown from this research.
- Up to three malfunctions are shown, most recent first. A continuous fault repeats; an intermittent fault is shown once and then the indicator stays on; with no DTC stored the indicator simply stays on. HIGH (also in the [Jazz SRS page][jazz-srs]).
- If the tester's SRS menu reports no DTC, or DTC 3-6 to 3-10, 4-6 to 4-10, 9-1 or 9-2, Honda says to double-check with the SCS blink method. HIGH.
- Wait 10 s after ignition OFF before disconnecting. MEDIUM.
- The "Without Scan Tool" page for the SRS contains only: "Manufacturer does not supply information for reading trouble codes without a scan tool." ([SRS read, no tool][srs-read-noscan]) HIGH.

SRS memory erase with the MES connector ([SRS erase][srs-clear]; identical timing in the [Jazz SRS page][jazz-srs]; HIGH):

1. Ignition OFF.
2. Connect the SCS service connector, tool 07PAZ-0010100, to the MES connector (2P). "Do not use a jumper wire."
3. Ignition ON (II). The SRS indicator lights for about 6 s and goes off. Remove the short within 4 s of it going off.
4. The indicator lights again. Reconnect the short within 4 s.
5. The indicator goes off. Remove the short within 4 s.
6. The indicator blinks twice: memory erased.
7. Ignition OFF, wait 10 s, ignition ON: indicator on about 6 s then off means OK.

The alternative given by the manual is a Honda PGM Tester "with its SRS supplement", or the HDS.

EPS clear with SCS ([EPS clear][eps-clear]; MEDIUM, single pass through the summariser):

1. Vehicle on the ground, front wheels straight ahead, ground SCS, then ignition ON. The EPS indicator lights for about 6 s.
2. Within 4 s of ignition ON, turn the wheel 45 degrees left and hold until the indicator goes out.
3. Within 4 s of it going out, return to straight ahead and release. The indicator lights again 4 s after release.
4. Within 4 s of it lighting, turn 45 degrees left and hold. The indicator goes out after 4 s.
5. Within 4 s, return to straight ahead and release. Four seconds later the indicator blinks twice: erased. If not, ignition OFF and repeat.

Honda says to use this method "when the HDS software doesn't match the vehicle's year and model".

#### 1.3 The MES connector

| Fact | Value | Confidence and source |
|---|---|---|
| Meaning | "Memory Erase Signal (MES) connector (2P)" | HIGH - [SRS component location index][srs-loc-img] |
| Location | Drawn at the driver's-side under-dash fuse/relay box, next to the DLC label. Mitchell locator: sedan "Under left side of dash", coupe "Under middle of dash". | HIGH for the sedan drawing; MEDIUM for the coupe ([Mitchell locator][portal-2001]) |
| Reachable from the DLC | No. None of the seven DLC wires is an MES wire. | HIGH - [DLC connector view][dlc-view-ex] |
| Wiring | SRS unit connector C has three diagnostic terminals: C2 GRY "DLC input/output", C6 BRN "DLC input/output", and C1 LT GRN/BLK whose far end is listed as ground point G551. The Mitchell data-lines diagram labels three SRS-unit signals "(MES)(SCS)(DIAG-H)". Reading these together, C1 is very probably the MES input and the MES connector simply closes it to G551. | HIGH for the terminal list ([SRS unit view][srs-view-img]); MEDIUM-LOW for "C1 = MES to G551" (inference; the MES connector's own view was not found) |
| Colour | Not confirmed for the Civic. One web article about another Honda calls it yellow. | LOW |
| Tool | SCS service connector 07PAZ-0010100 is a shorting plug. Later Civic manuals list 07PAZ-001010A. | HIGH / MEDIUM (search snippet) |

### 2. DTC lists

#### 2.1 ABS (2002 US Civic, same list for EX sedan and Si)

Code numbers are HIGH ([DTC index EX][dtc-index-ex], [DTC index Si][dtc-index-si]). Titles are the page headings of the individual DTC pages ([example: DTC 11][abs-dtc11]).

| DTC | Manual title | Notes | Confidence |
|---|---|---|---|
| 11, 13, 15, 17 | "Wheel Sensor (Open/Short to Body Ground/Short to Power)" | Indicator stays on until ignition OFF. 11 is right-front per a search snippet of the EP3 manual; the usual Honda order (11/12 RF, 13/14 LF, 15/16 RR, 17/18 LR) was not confirmed. | HIGH title, MEDIUM-LOW wheel mapping |
| 12, 14, 16, 18 | "Wheel Sensor Pulse Signal" | Indicator goes off after the car is driven again and the system is OK. | HIGH |
| 21 | "Continuous Operation (Chipped pulser)" | | HIGH |
| 31-38 | "Solenoid" | Which solenoid each number means was not read. | HIGH title |
| 51 | Motor lock | Title from a secondary source for an unspecified Honda. | LOW-MEDIUM |
| 52 | "Motor Stuck OFF" | | HIGH |
| 53 | Motor stuck ON | Secondary source. | LOW-MEDIUM |
| 54 | "Main Relay Stuck OFF" | Other Honda lists call this the fail-safe relay. | HIGH |
| 61 | "Ignition Voltage" | Checks battery voltage at 31P terminal 15 (IG2). Indicator goes off by itself when the fault disappears. | HIGH |
| 81 | "CPU (Central Processing Unit)" | | HIGH |

Not present in the 2002 manual: 41-48, 62, 71 (these belong to other Honda ABS generations). HIGH.

ABS memory behaviour ([ABS general information][abs-general]; MEDIUM): three DTCs are kept, most recent first, a repeat overwrites the earlier entry, storage is EEPROM so battery disconnection does not clear them. The unit is powered from IG2 and tests the pump motor on the first start after ignition ON.

The full table image: [DTC List - Antilock Brake System][abs-dtc-list].

#### 2.2 EPS (2002 Civic Si; headings of the individual DTC pages, HIGH; [example: DTC 12][eps-dtc12])

| DTC | Manual title |
|---|---|
| 12 | Torque Sensor T/SIG |
| 16 | Torque Sensor VT3 and VT6 |
| 17 | Torque Sensor Vcc1 |
| 18 | Torque Sensor Vcc2 |
| 22 | Vehicle Speed Sensor Signal |
| 23 | Engine Speed Signal Circuit |
| 37 | EPS Control Unit Internal Circuit (Input Circuit for Motor Voltage) |
| 41 | Voltage for Motor |
| 42 | "Motor Driven (Current ..." (heading truncated on the page) |
| 43 | Motor Driven Current is Excessively High |
| 45 | "Motor Driven (Current ..." (heading truncated on the page) |
| 47 | EPS Control Unit Internal Circuit (Motor Relay) |
| 50 | EPS Control Unit Internal Circuit (CPU or Microcomputer) |
| 51 | EPS Control Unit Internal Circuit (EEPROM) |
| 62 | EPS Control Unit Internal Circuit (Fail-safe Relay Stuck ON) |
| 64 | Battery Voltage is Excessively Low (Fail-safe Relay Contact Failure and Motor Voltage Fall Off) |
| 66 | EPS Control Unit Internal Circuit |
| 67 | Torque Sensor I/F Circuit |
| 68 | EPS Control Unit Internal Circuit |
| 69 | EPS Control Unit Internal Circuit |

EPS memory behaviour ([EPS on-board diagnostics][eps-obd]; MEDIUM): codes are stored in EEPROM; with 12, 16, 17, 18 or 67 stored the indicator stays on until the code is erased; for 22, 23, 64 and 66 the indicator clears by itself on recovery; power assist is shut off when a fault is detected (manual steering remains).

#### 2.3 SRS

Code numbers that exist (HIGH; [DTC index EX][dtc-index-ex], [NEC list][srs-nec], [Siemens list][srs-siemens]):

| SRS unit | Codes listed in the 2002 manual |
|---|---|
| NEC | 1-1 to 1-5, 2-1 to 2-5, 3-1 to 3-5, 4-1 to 4-5, 5-1 to 5-5, 6-1 to 6-4, 7-1 to 7-3, 8-1 to 8-6, 9-1, 9-2, 9-3, 9-4, 9-6, 9-7, 10-1, 21-1 to 21-5, 22-1 to 22-5 |
| Siemens | 1-1, 1-3, 1-4, 1-5, 2-1, 2-3, 2-4, 2-5, 3-1, 3-3, 3-4, 3-5, 4-1, 4-3, 4-4, 4-5, 5-1, 5-2, 5-4, 5-8, 6-3, 6-4, 6-7, 6-8, 7-1 to 7-3, 8-1 to 8-8, 9-1, 9-2, 9-3, 9-4, 9-6, 9-7, 10-1 to 10-7, 11-1, 11-3, 11-4, 11-5, 12-1, 12-3, 12-4, 12-5, 13-1 to 13-4, 14-1 to 14-4, 15-1 to 15-3, 21-1, 21-3, 21-4, 21-5, 22-1, 22-3, 22-4, 22-5 |

The NEC table has no side-airbag codes (10-2 onward, 11-x to 15-x), the Siemens table has them. A search snippet of a honda-tech document for a 2003 Civic reads "NEC (M2), DENSO (M1) SRS unit (Without Side Airbag Model)". So the supplier follows the airbag configuration: NEC or Denso without side airbags, Siemens with side airbags. MEDIUM (inference from two sources).

Meanings (third-party table headed "Honda CIVIC, CR-V, ELEMENT & RIDGELINE Codes", no years given, wording paraphrased; MEDIUM; [myairbags][myairbags]):

| Main code | Circuit | Sub codes |
|---|---|---|
| 1 | Driver's airbag inflator | -1/-2 open or high resistance, -3 short to another wire or low resistance, -4 short to power, -5 short to ground |
| 2 | Front passenger's airbag inflator | -1 open, -2 high resistance, -3 short to another wire, -4 short to power, -5 short to ground |
| 3 | Driver's seat belt tensioner | same pattern as main code 1 |
| 4 | Front passenger's seat belt tensioner | same pattern |
| 5, 6, 7, 8 | Internal failure of the SRS unit (check battery voltage first) | 5-1 to 5-5 and 5-8, 6-1 to 6-8, 7-1 to 7-3, 8-1 to 8-8 |
| 9 | Miscellaneous | 9-1 SRS unit or indicator circuit, 9-2 SRS unit power supply, 9-3 driver's seat belt buckle switch, 9-4 front passenger's buckle switch, 9-6 driver's-side front impact sensor, 9-7 passenger's-side front impact sensor |
| 10 | Deployment records | 10-1 airbags and tensioners deployed, 10-2 driver's side airbag deployed, 10-3 tensioners plus driver's side airbag, 10-4 passenger's side airbag, 10-5 tensioners plus passenger's side airbag, 10-6 both side airbags, 10-7 tensioners plus both side airbags |
| 11 | Driver's side airbag inflator | -1 open or high resistance, -3, -4, -5 as above |
| 12 | Front passenger's side airbag inflator | -1, -3, -4, -5 |
| 13 | Driver's side impact sensor | -1/-2 internal failure, -3 no signal, -4 power supply |
| 14 | Front passenger's side impact sensor | -1/-2, -3, -4 |
| 15 | OPDS | 15-1 OPDS unit faulty or not initialised, 15-2 side airbag cutoff indicator circuit, 15-3 OPDS sensor |
| 21 | Driver's seat belt buckle tensioner | -1/-2 open or high resistance, -3 short to another wire, -4/-5 short to power or ground |
| 22 | Front passenger's seat belt buckle tensioner | same pattern |

9-3 (driver's buckle switch) is reported as the most common SRS code on this generation (search snippets from honda-tech and ALLDATA; LOW-MEDIUM). Codes 10-x mean the unit has recorded a deployment; Honda practice is to replace such a unit rather than clear it (recollection; LOW).

The exact Honda wording is in two table images per supplier: [NEC list][srs-nec] and [Siemens list][srs-siemens], and in each DTC page under [DTC index EX][dtc-index-ex].

#### 2.4 Immobilizer, multiplex, heater

- Immobilizer: no DTC numbers. Only the indicator behaviour in 1.2. The troubleshooting flow starts from "indicator blinks" or "indicator does not come on" ([immobilizer troubleshooting][immo-test]). MEDIUM.
- Multiplex control system: mode 1 shows DTCs as blinks of the spotlight and ceiling light, lowest number first ([multiplex troubleshooting][mpcs-test]). The DTC chart itself was not read.
- Heater control panel: codes 7, 8, 9, 10, 11, 12, 14, 15 exist ([DTC index EX][dtc-index-ex]); meanings not read.

### 3. With the dealer tool: which unit talks on which wire, and what the tool offers

#### 3.1 DLC pinout (2002 US Civic; read directly from the ETM connector views; HIGH)

Identical for DX/LX, GX, EX, HX ([sedan and coupe view][dlc-view-img-ex]) and for the Si ([Si view][dlc-view-img-si]). The connector is grey.

| DLC cavity | Wire | Honda description | Goes to |
|---|---|---|---|
| 4 | BLK | "Ground for PGM Tester" | G451 |
| 5 | BRN/YEL | "Signal ground for communication network", circuit Z20 | ECM/PCM terminal E3; also immobilizer unit-receiver pin 1 |
| 7 | LT BLU | "K line for off-board communication" | ECM/PCM terminal E23; ABS modulator-control unit terminal 11 |
| 9 | BRN | circuit F28 (SCS) | ECM/PCM, SRS unit C6, EPS unit C8, ABS unit terminal 12 |
| 12 | RED/WHT | "WEN input" / "Write enable" | ECM/PCM (E30 on K-series per [Rywire][rywire]) |
| 14 | GRY | circuit F55 (DIAG-H) | SRS unit C2, immobilizer unit-receiver pin 3, EPS unit C20 (Si) |
| 16 | WHT/RED | "Power for PGM Tester (B+, 4 amps max)", fuse 9 under-hood | battery |
| 1, 2, 3, 6, 8, 10, 11, 13, 15 | empty | | |

K-line check values from the manual: battery voltage between 16 and 4 and between 16 and 5; with ignition ON, 8.5 V or more between 5 and 7 ([DLC circuit troubleshooting][dlc-test]). MEDIUM.

#### 3.2 Unit by unit

| Unit | Serial line to the tester | Unit terminal | SCS terminal | Evidence | Confidence |
|---|---|---|---|---|---|
| ECM/PCM | K-line, DLC 7 | E23 (D17) | E29 (K-series) | [DLC circuit troubleshooting][dlc-test], [Rywire][rywire] | HIGH / MEDIUM |
| ABS modulator-control unit (31P, black, left side of engine compartment) | K-line, DLC 7 | 11 LT BLU "DIAG-K" | 12 BRN "DIAG-L" | [ABS unit view][abs-view-img]; a honda-tech post (search snippet) independently says the K-line has one branch to the ECU and one to the large ABS plug | HIGH |
| SRS unit (yellow connectors, under middle of dash; A 18P, B 18P, C 8P) | DIAG-H, DLC 14 | C2 GRY "DLC input/output" | C6 BRN "DLC input/output" | [SRS unit view][srs-view-img]; Mitchell diagram text carries "(MES)(SCS)(DIAG-H)" and no K-LINE in the SRS diagram ([data lines][scribd-data-lines], [SRS diagram][scribd-srs]) | HIGH |
| EPS control unit (Si; under right side of dash; A 2P, B 2P, C 20P) | DIAG-H, DLC 14 | C20 GRY "DIAG-H (Diagnosis-H)": "Communications with HDS"; "HDS not connected: 5 V" | C8 BRN "SCS (Service check signal)" | [EPS connector C table][eps-pinout-img], [EPS unit view][eps-view-img] | HIGH |
| Immobilizer control unit-receiver (7P, green, in steering column cover) | DIAG-H, DLC 14 | 3 GRY "Circuit F55" | none | [immobilizer unit view][immo-view-img]; [Rywire][rywire] measured "+5V from immobilizer control unit pin 3" on DLC 14 | HIGH |
| Multiplex control unit, gauge assembly | none on the DLC | - | - | [multiplex description][mpcs-desc], DLC view has no further wires | HIGH |
| Heater control panel, keyless receiver | none on the DLC | - | - | [heater self-diagnosis][hvac-read], [keyless programming][keyless] | MEDIUM |

Other EPS connector C signals that matter for a bench setup: 6 YEL/BLU WLP (drives the EPS indicator), 7 BLU/WHT VSP (vehicle speed pulse, 4 pulses per revolution), 19 BLU NEP (engine pulse), 10 YEL IG1, 3/11 torque sensor supplies VCC1 (12 V) and VCC2 (5 V), 13 VT3, 5 VT6, 15 T/SIG, 17 PSW. HIGH ([EPS connector C table][eps-pinout-img]).

ABS 31P terminals (HIGH, [ABS unit view][abs-view-img]): 1/2 RRS -/+, 4/5 FRS -/+, 6/7 FLS -/+, 8/9 RLS -/+, 10 BLU/RED ABS indicator ('01-'02), 11 LT BLU DIAG-K, 12 BRN DIAG-L, 14 WHT/BLK STOP, 15 BLK/ORN IG2, 16 BLK M-GND, 17 WHT/RED +B-MR, 18 WHT/GRN +B-FSR, 19 BLK GND, 20 BLU/RED ABS indicator ('03-'05), 21 BRN/YEL EBD ('03-'05).

#### 3.3 Functions the service manual attributes to the tester

| Unit | Function | Source | Confidence |
|---|---|---|---|
| ABS | Read DTCs ("follow the prompts"), clear DTCs, ground SCS | [ABS read][abs-read], [ABS clear, tool page][abs-clear-scan] | HIGH |
| SRS | "SRS" menu: read DTCs; "SCS" menu: blink codes; erase memory (PGM Tester needs the "SRS supplement"); HDS: SRS > Misc Test > adjustments > OPDS INIT after replacing the seat-back cover, cushion or OPDS unit (TSB 02-052) | [SRS read][srs-read-scan], [SRS erase][srs-clear], [OPDS initialisation][srs-opds] | MEDIUM-HIGH |
| EPS | Read and clear DTCs with HDS; SCS grounding. Torque-sensor neutral-position memorisation was not found in the 2002 Si pages that were read. | [EPS read][eps-read], [EPS clear][eps-clear] | MEDIUM; neutral learn unknown |
| Immobilizer | After replacing the control unit-receiver: "rewrite it with a Honda PGM Tester/HDS". Key registration is not described in the service manual pages read. | [immobilizer troubleshooting][immo-test] | MEDIUM |
| ECM/PCM | The multiplex page tells the technician to "update the ECM/PCM software before replacing it", so dealer reflash exists for this generation. | [multiplex troubleshooting][mpcs-test] | MEDIUM (cross-cutting) |
| Data lists, function/actuator tests | Not mentioned on any page read. | - | unknown |

Blink-code or self-test only (no tester line): multiplex control unit, gauge assembly, heater control panel, keyless receiver.

#### 3.4 Implications for the device (engineering inference, not sourced)

- Two different single-wire physical layers are needed: an ISO 9141-style K-line on pin 7 (battery-level, reaches ECM/PCM and ABS) and a 5 V single wire on pin 14 (DIAG-H, reaches SRS, EPS, immobilizer). A 12 V K-line transceiver must not be connected to pin 14.
- An open-drain switch to ground on pin 9 (SCS), under firmware control, gives blink-code reading for ABS, EPS and SRS and tool-free EPS clearing even before any protocol is known. The device cannot see the indicators, so the user counts blinks unless an optical sensor is added.
- SRS erase without protocol knowledge needs a second lead to the MES connector at the fuse box.

### 4. Immobilizer

#### 4.1 Architecture on the 2002 US Civic (HIGH unless noted)

- Components listed by the manual: ignition key with transponder, immobilizer control unit-receiver, immobilizer indicator, PGM-FI main relays, fuel pump (GX: injector relay and tank solenoid valve), ECM/PCM ([immobilizer description][immo-desc]).
- Sequence: with ignition ON or START the ECM/PCM powers the transponder through the unit-receiver, the transponder returns a coded signal to the ECM/PCM through the receiver, and if the code is correct the ECM/PCM grounds PGM-FI main relay 2 (fuel pump). Otherwise the fuel supply stays off ([immobilizer description][immo-desc]). So the decision is made in the ECM/PCM, and an ECM swap involves the immobilizer.
- Unit-receiver connector, 7P, green, in the steering column cover ([immobilizer unit view][immo-view-img]):

| Pin | Wire | Function |
|---|---|---|
| 1 | BRN/YEL | circuit Z20 (the DLC pin 5 signal ground network) |
| 2 | RED/BLU | "Immobilizer code input (IMOCD)", to ECM/PCM terminal E27 |
| 3 | GRY | circuit F55, to DLC 14 (DIAG-H) |
| 4 | GRN/ORN | circuit 207, parking brake switch (1 V or less applied, 5 V or more released) |
| 5 | BLU/ORN | "Indicator control (IMOLMP)" |
| 6 | YEL/BLK | circuit D04, ignition feed through PGM-FI main relay 1 |
| 7 | WHT/RED | fuse 9, under-hood, battery |

- Why a parking-brake input reaches the immobilizer unit is not explained on the pages read; it may be a condition used during tester-driven registration. Unknown.
- Indicator: 2 s on then off with a registered key; continued blinking means the key was not accepted; other transponder keys or large fobs on the ring can disturb it ([immobilizer troubleshooting][immo-test]). MEDIUM.

#### 4.2 Transponder type and public reverse engineering

- The Megamos Crypto paper (Verdult, Garcia, Ege; USENIX Security 2013 supplement) lists Honda "Accord, Civic, CR-V, FR-V, HR-V, Insight, Jazz (2002), Legend, Logo, S2000, Shuttle, Stream" among vehicles that used Megamos Crypto "for some version/year"; the authors experimented on a 2002 Jazz. The transponder has a 96-bit secret key and a 32-bit PIN that protects writes; the cipher was recovered from the Tango Programmer software; eavesdropping, emulation and reader support were built for the Proxmark III ([paper][megamos]). HIGH that some Civics use Megamos Crypto (ID48); which 2001-2005 Civics do is not stated.
- Recollection, not verified in this session (LOW): early US 7th-generation cars are usually listed by locksmith catalogues with a fixed-code Megamos ID13 chip and later ones with ID48 Megamos Crypto, and European cars with ID48. Treat as a hypothesis; the owner can settle it by having the key read on any transponder identifier.
- No public reverse engineering of the IMOCD wire between the unit-receiver and the ECM/PCM, or of immobilizer commands on DIAG-H, was found.

#### 4.3 Registration, ECM replacement, markets, locksmith tools

Not answered from sources. The search budget ran out before these could be researched. What is known:

- The service manual delegates everything to the tester ("rewrite it with a Honda PGM Tester/HDS"). MEDIUM.
- Which US trims have the immobilizer in 2001-2002 could not be established. The ALLDATA-derived trees for the 2002 DX and EX both carry the same immobilizer section, and the ETM connector view only distinguishes "GX" and "except GX", so the documents do not settle it. The coordinator's hypothesis (EX first, DX/LX later) remains unverified.
- Europe and Japan: no source read.

### 5. Multiplex control unit, gauge assembly, climate control, keyless

- Multiplex control system ([multiplex description][mpcs-desc]; MEDIUM): the ECM/PCM, the gauge assembly and the under-dash fuse/relay box (which contains the multiplex control unit) exchange serial data over "exclusive communication lines". Functions: entry light control, wiper/washer, interlock, keyless and power door lock, key-in reminder, gauges and indicators, A/C compressor and fan control. Hardware and software fail-safes fix the outputs if a unit or the line fails. Sleep after ignition OFF; current falls from 70-80 mA to under 10 mA.
- Self-diagnosis ([multiplex troubleshooting][mpcs-test]; MEDIUM): a special tool is plugged into the multiplex inspection connector with ignition ON, ceiling light in the middle position and doors closed. After about 5 s the spotlight and ceiling light come on for 2 s, go out, then blink once for 0.2 s: mode 1, followed by DTC blinks in ascending order. Unplug the tool for 5 to 10 s and plug it in again: two 0.2 s blinks, mode 2, in which each operated switch input is answered by one blink. The communication wire sits at about 4-10 V awake and at battery voltage asleep (under-dash connector E or K, terminal 10).
- The tool is a separate connector from the DLC. The Mitchell data-lines diagram carries an "MPCS CHK" label and the body-control diagram a "SERVICE CHECK CONNECTOR" ([data lines][scribd-data-lines], [body control diagram][scribd-bcm]). The special tool's part number was not seen. No DLC-reachable diagnostics. MEDIUM.
- Gauge assembly self-diagnostic ([gauge self-diagnostic][gauge]; MEDIUM): hold the trip/reset button, lighting switch ON, ignition ON (II); within 5 s lighting switch OFF, ON, OFF; within 5 s release the button and press and release it four times. The beeper sounds five times, indicators blink, the LCD blinks five times, the needles sweep, and a communication-line check runs: all segments lit means OK, "Error" means a fault between gauge assembly, multiplex control unit and ECM/PCM. Ends at ignition OFF or above 2 km/h.
- Heater control panel self-diagnosis: see 1.2.
- Keyless ([keyless programming][keyless]; MEDIUM): the receiver is in the multiplex control unit. Transmitters are registered with ignition ON/OFF cycles and button presses, each step within 1 to 4 s, no tool; three transmitters maximum. An accessory security system ("Carlink" card reader and interface box) appears in the diagrams.

### 6. What can be written, and the hazards

| Unit | Write-type operations known | Path |
|---|---|---|
| SRS | Erase DTC memory; OPDS initialisation | MES connector, or tester over DIAG-H |
| ABS | Erase DTC memory | Tester over K-line, or 20 ignition cycles |
| EPS | Erase DTC memory | Tester over DIAG-H, or SCS plus steering sequence |
| Immobilizer | Rewrite of the unit-receiver after replacement; key registration (details unknown) | Tester (DIAG-H and/or K-line; split unknown) |
| Keyless | Register transmitters | Ignition and button sequence, no tool |
| Multiplex, gauge, heater | Nothing documented | - |

Hazards:

- SRS ([SRS general precautions][srs-precautions], [SRS troubleshooting information][srs-general]; HIGH): "disconnect the negative cable from the battery, and wait at least three minutes before beginning work"; memory is not erased by battery disconnection; use only a digital multimeter whose output is 10 mA or less on the lowest ohm range because "a tester with a higher output could cause accidental deployment"; do not bump the SRS unit with ignition ON or within 3 minutes after OFF; do not touch probes to SRS unit terminals or harness connectors and do not jumper them; back-probe from the wire side only.
- Design consequence (inference): the device is electrically connected to the SRS unit through pins 14 and 9. Its pin 14 driver should be a current-limited 5 V open-drain stage, it should never source battery voltage onto pin 14, and SRS functions other than read and clear should be left out until the protocol is well understood. A unit that stored a deployment (10-x) should not be "repaired" by clearing.
- ABS and EPS: reading and clearing are benign. A fault that is detected switches ABS or power assist off, so the device must not disturb the K-line or DIAG-H while the car is moving. Any future actuator test would run the pump and valves and belongs on a stationary car only. (Inference.)
- Immobilizer: a failed or interrupted rewrite can leave the car unable to start, and the recovery path (existing keys, codes from Honda) is unknown. Treat as the last and riskiest feature.
- Tester supply: pin 16 is limited to 4 A.

### 7. Which units exist on which car: what the owner should check

| Item | What the documents say | How to check on the car |
|---|---|---|
| ABS | 2002 US: the ABS unit connector view is headed "(EX, GX)"; the Si manual has the same ABS code list. 2003-2005 units add an EBD output. HIGH | ABS indicator at ignition ON; modulator with 31P black connector at the left of the engine compartment |
| EPS | US: only the Si manual has an EPS section; the EX sedan manual has hydraulic-steering sections. The Mitchell hatchback diagram shows an EPS control unit. HIGH for the US | EPS indicator at ignition ON; no power-steering fluid reservoir |
| Side airbags, OPDS | Optional; the SRS unit connector B lists terminals prefixed "Side airbags:"; with them comes the OPDS unit in the passenger seat-back, the side airbag cutoff indicator and (by inference) the Siemens SRS unit. HIGH/MEDIUM | "SRS" tag on the front seat-backs; "SIDE AIRBAG" indicator in the gauge |
| SRS unit supplier | NEC, Siemens, Denso depending on configuration; DTC table differs | Label on the unit under the centre of the dash (battery off, 3 minutes) |
| Immobilizer | Present in the 2002 manuals; trim coverage not established | Key-shaped indicator lighting for 2 s at ignition ON; green 7P unit around the ignition lock |
| DLC wires | 4, 5, 7, 9, 12, 14, 16 on every 2002 US trim | Look into the DLC: cavities 14 and 12 populated confirms the Honda layout; a missing 14 on a non-US car would change the plan |
| Gauge type | "Gauge assembly with four meters" or "with three meters" (indicator positions differ) | Visual |
| Transmission | Separate transmission DTC lists "with CVT" (HX) and "without CVT" | Outside this dimension |

Europe and Japan: not verified. Open question 6.

### 8. Reverse-engineering targets for later phases (REA)

- A capture of pin 14 (DIAG-H) and pin 7 (K-line) while a dealer tool or an aftermarket scanner reads SRS, EPS, immobilizer and ABS on this car: the most direct way to get the two protocols. Logic-analyser traces can be decoded and diffed with REA's capture tooling.
- Honda tester software (PGM Tester data cards, HDS for Windows and its interface firmware) and aftermarket J2534 drivers that implement the Honda single-wire protocols: Ghidra targets for the frame format, unit addresses and the SRS/EPS/immobilizer command tables.
- Android or Windows diagnostic applications that claim SRS/ABS coverage on pre-CAN Hondas: JADX or Ghidra targets for the same tables.
- ECM/PCM firmware: the IMOCD routine (unit-receiver to ECM code exchange) and the multiplex-line handler.
- Bench units from a breaker: an SRS unit (EEPROM with DTC and crash records; no inflators connected), an EPS control unit and an immobilizer unit-receiver can all be powered on the bench with the pin tables above and exercised on DIAG-H without touching the car.

## Conflicts and uncertainties

- **DIAG-H lead**: the coordinator's lead put SRS and ABS on pin 14. Primary connector views show SRS, EPS and immobilizer on pin 14 and ABS on the K-line (terminal named DIAG-K). The lead is wrong for ABS.
- **Pin 14 naming**: pinoutguide.com lists Honda pin 14 as "CAN Low J-2284 (WEN)" and puts "Input/Output for VTM-4 and immobilizer control" on "13 or 14?" ([pinoutguide][pinoutguide]). For this car both labels are wrong: WEN is pin 12 and pin 14 is DIAG-H.
- **ABS clear procedure**: the coordinator expected a brake-pedal method. The 2002 manual gives only the tester or 20 ignition cycles. A brake-pedal sequence exists in other Honda manuals (a search snippet of a Jazz/Fit manual mentions pedal presses and "blinks twice"). Other model years of the Civic were not checked.
- **ABS code list**: the expected 41-48, 62 and 71 do not exist in the 2002 manual; 21 does.
- **SRS unit connector size**: the connector view shows A 18P, B 18P, C 8P. The "SRS Indicator Stays ON" page was summarised as mentioning "connector A (28P)" ([page][srs-stayson]). Either a summariser error or a different unit variant; not resolved.
- **DTC 54 wording**: "Main Relay Stuck OFF" in the 2002 Civic page versus "fail-safe relay" in other Honda lists; the unit's supply terminal is named +B-FSR.
- **Fuse 9 rating**: the DLC troubleshooting page says "No. 9 BACK UP (7.5A)"; the immobilizer, multiplex and gauge pages say No. 9 (10 A) under-hood. Not resolved; the 4 A tester limit is the figure that matters.
- **SCS timing for EPS**: reading requires SCS grounded after ignition ON, clearing requires it grounded before ignition ON. Both statements come from single summariser passes over the two Si pages.
- **Manual revision**: the 2002 pages mix "Honda PGM Tester" (original) and "HDS" (later revision) wording. Menu names quoted here are from whichever revision the page carries.
- **SRS DTC meanings** come from a third-party table that does not state model years; the code numbers match the Civic manual.
- **Trim coverage in ALLDATA-derived trees** is unreliable: the 2002 DX tree contains ABS and immobilizer sections although the ABS connector view says "(EX, GX)".

## Open questions

1. **DIAG-H protocol** (bit rate, framing, checksum, unit addressing, whether the unit or the tester provides the 5 V pull-up). Not in any service-manual page. Answer by sniffing pin 14 during a dealer-tool or aftermarket-scanner session, by a bench test on a scrapyard EPS or SRS unit, or by reverse engineering tester software. Recollection to verify: SAE J2534-2 defines a Honda "DIAG-H" protocol identifier, so J2534 driver binaries would be good REA targets.
2. **ABS on the K-line**: does it use the same Honda proprietary K-line protocol and timing as the ECM/PCM with a different address? Sniff pin 7 while a tool reads ABS.
3. **SCS by plain jumper**: Honda only documents the tester's SCS menu. Confirm on the car that grounding pin 9 to pin 4 gives the ABS, EPS and SRS blink modes, and note that all units react together.
4. **MES connector**: colour, wire colours and whether it closes SRS unit C1 (LT GRN/BLK) to G551. Answer by reading the SRS circuit diagram or the MES connector view on charm.li, or by inspection at the fuse box.
5. **Immobilizer by market and year**: transponder type in the key, which trims have the system in 2001-2002, what the tester needs for adding a key, for all keys lost and for an ECM/PCM swap (existing keys, codes from Honda, online access), and what locksmith tools advertise. Answer from the PGM Tester immobilizer manual or HDS help, the owner's manual, a transponder identifier on the actual key, and locksmith coverage lists. Not researched: search budget exhausted.
6. **European (D14Z6, D16V1, D16W7, 1.7 CTDi, K20A2) and Japanese cars**: DLC population, which units exist, whether the same DIAG-H/K-line split applies, EPS on all hatchbacks, immobilizer generation. Answer from the European electronic service manual and by inspecting the car's DLC cavities.
7. **Exact SRS DTC wording per supplier and the ABS solenoid/wheel mapping**: read the table images at the charm.li URLs listed below.
8. **Tester function lists** (data list items, function tests, EPS torque-sensor neutral learn, ABS tests): inspect HDS software or watch a dealer tool. The 2002 Si pages read contain no neutral-position procedure; the Si "Steering Control Module > Service and Repair" page was not read.
9. **Multiplex DTC chart, the inspection connector's location and the special tool's part number**; the "Keyless Entry Programming Connector" entry in the manual index.
10. **2003-2005 changes**: EBD on the ABS unit, possible SRS unit changes, immobilizer coverage.
11. **Sub-code blink timing for the SRS** (only in figures).
12. **Whether common K-line-only OBD adapters can reach the SRS at all** on this car (they would need pin 14); this decides whether off-the-shelf hardware can be reused.

## Sources

Primary (factory service information for the 2002 US Civic as mirrored by Operation CHARM):

- [DTC index, 2002 EX sedan][dtc-index-ex] - list of every ABS, SRS and heater code page.
- [DTC index, 2002 Si][dtc-index-si] - adds the EPS codes.
- [ABS read / SCS method][abs-read]; [ABS on-board diagnostics, PGM Tester][abs-obd]; [ABS clear without tool][abs-clear-noscan]; [ABS clear with tool][abs-clear-scan]; [ABS general troubleshooting information][abs-general]; [ABS DTC 11 page][abs-dtc11]; [ABS DTC list image page][abs-dtc-list].
- [ABS modulator-control unit connector view page][abs-view] and [image][abs-view-img].
- [SRS read with tester / SCS][srs-read-scan]; [SRS read without tool][srs-read-noscan]; [SRS memory erase][srs-clear]; [SRS general troubleshooting information][srs-general]; [SRS general precautions][srs-precautions]; [SRS components][srs-components]; [OPDS initialisation][srs-opds]; [SRS indicator stays on][srs-stayson]; [NEC DTC list][srs-nec]; [Siemens DTC list][srs-siemens].
- [SRS unit connector view page][srs-view] and [image][srs-view-img]; [SRS component location index image][srs-loc-img].
- [Immobilizer description][immo-desc]; [immobilizer troubleshooting][immo-test]; [immobilizer unit connector view page][immo-view] and [image][immo-view-img].
- [Keyless transmitter programming][keyless].
- [Multiplex description][mpcs-desc]; [multiplex troubleshooting][mpcs-test].
- [Gauge assembly self-diagnostic][gauge]; [heater self-diagnosis][hvac-read].
- [DLC circuit troubleshooting][dlc-test]; [DLC connector view page, EX sedan][dlc-view-ex] and [image][dlc-view-img-ex]; [DLC connector view page, Si][dlc-view-si] and [image][dlc-view-img-si].
- [EPS read][eps-read]; [EPS clear][eps-clear]; [EPS on-board diagnostics][eps-obd]; [EPS DTC 12 page][eps-dtc12]; [EPS pinout page][eps-pinout] and [connector C table image][eps-pinout-img]; [EPS unit connector view page][eps-view] and [image][eps-view-img].

Secondary:

- [Mitchell "Computer Data Lines" diagram, 2005 Civic (Scribd text)][scribd-data-lines] - signal labels K-LINE, LG3, SCS, WEN, NEP, MES, DIAG-H, "MPCS CHK".
- [Mitchell "Supplemental Restraints" diagram (Scribd text)][scribd-srs] - "MES CON", "DIAG-H", no K-LINE.
- [Mitchell body control module diagram (Scribd text)][scribd-bcm] - multiplex control unit, service check connector.
- [Mitchell component locator, 2001 Civic][portal-2001] - MES connector and immobilizer unit locations.
- [Rywire K-series DLC pinout][rywire] - wire colours and ECM pins for 4, 5, 7, 9, 12, 14, 16.
- [pinoutguide.com Honda OBD-II pinout][pinoutguide] - generic, partly wrong for this car.
- [myairbags.com Honda SRS DTC codes][myairbags] - SRS code meanings.
- [Honda Jazz manual, SRS troubleshooting][jazz-srs] - same MES timing, SCS as pins 4 and 9.
- [Dismantling Megamos Crypto, USENIX Security supplement][megamos] - Honda Civic in the vehicle table.
- [Wikipedia, Honda Civic (seventh generation)][wiki-7g] - body codes and engines by market; nothing on immobilizers.

Seen only as search-result snippets (pages themselves refused the fetch tool; LOW):

- https://honda-tech.com/forums/honda-civic-2001-2005-78/new-guy-dlc-pin-7-communication-issues-3093207/ - K-line branches to ECU and ABS plug on a 2002 EX.
- https://honda-tech.com/forums/attachments/honda-civic-2001-2005-78/512280d1631121078-intermittent-srs-indicator-light-ellessitesrsguide.pdf - 2003 Civic SRS DTC index, "NEC (M2), DENSO (M1) SRS unit (Without Side Airbag Model)", code 9-3 case.
- https://www.manualslib.com/manual/464698/Honda-Civic.html?page=1755 and https://www.manualslib.com/manual/813630/Honda-Civic.html?page=627 - Civic service manual pages for SRS erase and ABS DTCs.
- https://www.7thgenhonda.com/threads/list-of-all-possible-dtcs-for-the-2001-2005-honda-civic.8742/ - community DTC list, not read.

[dtc-index-ex]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/
[dtc-index-si]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/
[abs-read]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/
[abs-obd]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/Description%20of%20On-Board%20Diagnostics/With%20Honda%20PGM%20Tester/
[abs-clear-noscan]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/Without%20Scan%20Tool/
[abs-clear-scan]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/With%20Manufacturer%27s%20Scan%20Tool/
[abs-general]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/Diagnostic%20Strategies/General%20Troubleshooting%20Information/
[abs-dtc11]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Manufacturer%20Code%20Charts/Antilock%20Brakes%20%28ABS%29%20%2F%20Traction%20Control%20System%20%28TCS%29/DTC%2011/
[abs-dtc-list]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Antilock%20Brake%20System/
[abs-view]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Electronic%20Brake%20Control%20Module/Diagrams/
[abs-view-img]: https://charm.li/images/IMP02Q313/asian140/176976643/
[srs-read-scan]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/With%20Manufacturer%27s%20Scan%20Tool/
[srs-read-noscan]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/Without%20Scan%20Tool/
[srs-clear]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/
[srs-general]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/General%20Troubleshooting%20Information/
[srs-precautions]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Maintenance/Air%20Bag%28s%29%20Arming%20and%20Disarming/Service%20Precautions/Vehicle%20Damage%20Warnings/General%20Precautions/
[srs-components]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Description%20and%20Operation/SRS%20Components/
[srs-opds]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Programming%20and%20Relearning/
[srs-stayson]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Symptom%20Related%20Diagnostic%20Procedures/SRS%20Indicator%20Stays%20ON%3A/
[srs-nec]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Supplemental%20Restraint%20System/NEC%20SRS%20Unit/
[srs-siemens]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Supplemental%20Restraint%20System/Siemens%20SRS%20Unit/
[srs-view]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Restraints%20and%20Safety%20Systems/Air%20Bag%20Control%20Module/Diagrams/
[srs-view-img]: https://charm.li/images/IMP02Q313/asian140/278422372/
[srs-loc-img]: https://charm.li/images/IMP02Q313/asian140/276755943/
[immo-desc]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Accessories%20and%20Optional%20Equipment/Antitheft%20and%20Alarm%20Systems/Description%20and%20Operation/Immobilizer%20System/
[immo-test]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Accessories%20and%20Optional%20Equipment/Antitheft%20and%20Alarm%20Systems/Testing%20and%20Inspection/
[immo-view]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Accessories%20and%20Optional%20Equipment/Antitheft%20and%20Alarm%20Systems/Alarm%20Module/Diagrams/
[immo-view-img]: https://charm.li/images/IMP02Q313/asian140/134191906/
[keyless]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Accessories%20and%20Optional%20Equipment/Antitheft%20and%20Alarm%20Systems/Keyless%20Entry/Keyless%20Entry%20Transmitter/Testing%20and%20Inspection/Programming%20and%20Relearning/
[mpcs-desc]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Body%20and%20Frame/Body%20Control%20Systems/Description%20and%20Operation/
[mpcs-test]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Body%20and%20Frame/Body%20Control%20Systems/Testing%20and%20Inspection/Multiplex%20Control%20System%20Troubleshooting/
[gauge]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Instrument%20Panel%2C%20Gauges%20and%20Warning%20Indicators/Instrument%20Cluster%20%2F%20Carrier/Testing%20and%20Inspection/
[hvac-read]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Heating%20and%20Air%20Conditioning/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/
[dlc-test]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Data%20Link%20Connector/Testing%20and%20Inspection/
[dlc-view-ex]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Data%20Link%20Connector/Diagrams/Connector%20Views/
[dlc-view-img-ex]: https://charm.li/images/IMP02Q313/asian140/673344130/
[dlc-view-si]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Data%20Link%20Connector/Diagrams/Connector%20Views/
[dlc-view-img-si]: https://charm.li/images/IMP02Q313/asian140/673337889/
[eps-read]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/
[eps-clear]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/
[eps-obd]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/Description%20of%20On-Board%20Diagnostics/
[eps-dtc12]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Manufacturer%20Code%20Charts/Electrical%20Power%20Steering%20%28EPS%29%20System/DTC%2012/
[eps-pinout]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Pinout%20Values%20and%20Diagnostic%20Parameters/
[eps-pinout-img]: https://charm.li/images/IMP02Q313/asian140/1086986115/
[eps-view]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Power%20Steering/Steering%20Control%20Module/Diagrams/
[eps-view-img]: https://charm.li/images/IMP02Q313/asian140/283936492/
[scribd-data-lines]: https://www.scribd.com/document/722396339/COMPUTER-DATA-LINES-Honda-Civic-Hybrid-2005-SYSTEM-WIRING-DIAGRAMS
[scribd-srs]: https://www.scribd.com/document/722396344/SUPPLEMENTAL-RESTRAINTS-Honda-Civic-Hybrid-2005-SYSTEM-WIRING-DIAGRAMS
[scribd-bcm]: https://www.scribd.com/document/722396342
[portal-2001]: https://portal-diagnostov.com/en/2020/07/04/2001-honda-civic/
[rywire]: https://blog.rywire.com/k-series-dlc-connector-pinout/
[pinoutguide]: https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml
[myairbags]: https://info.myairbags.com/kb/honda-srs-airbag-dtc-codes
[jazz-srs]: https://hondziarze.duckdns.org/jazz/A00/HTML/00/SAA2E00K79100000000BBAT00.HTML
[megamos]: https://www.usenix.org/sites/default/files/sec15_supplement.pdf
[wiki-7g]: https://en.wikipedia.org/wiki/Honda_Civic_(seventh_generation)
