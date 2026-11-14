# 01 - The vehicle and its control units (2002 Honda Civic, 7th generation 2001-2005)

> Research notes, dimension 1 of 9. Date: 2026-11-14. Scope: identifying the car, its control units, and its factory diagnostic interface so the owner can build a tool to read and clear codes on their own 2002 Civic.
> Confidence tags: HIGH (primary source or two independent sources agree), MEDIUM (one credible source), LOW (single forum post, inference, or researcher's prior knowledge not re-verified here).

## Summary

- A "2002 Civic" is a dozen different cars. North America: sedan ES1 (DX/LX, D17A1) and ES2 (EX, D17A2), coupe EM2 (DX/LX/EX/HX), GX CNG sedan EN2 (D17A7), and the UK-built Si/SiR hatchback EP3 (K20A3), new for 2002. Europe: UK-built 3-door EP1-EP4 and 5-door EU-series with 1.4 (D14Z6), 1.6 (D16V1), 1.7 CTDi diesel (Isuzu 4EE2) and 2.0 (K20A3 Type S / K20A2 Type R). Japan: 5-door EU1-EU4, Ferio ES1-ES3/ET2, Type R EP3, Hybrid ES9. Always identify a specific car by full VIN plus ECM part number, never by model code. HIGH.
- On the US D17 cars there is one powertrain computer: an ECM (manual) or PCM (automatic/CVT; transmission control is inside it, no separate TCM). Behind the glove box, connectors A(31P)/B(24P)/C(22P, automatics)/E(31P). HIGH.
- ECM part-number families confirmed: PLM (D17A1), PLR (D17A2 and JDM D17A), PLC (Japan-built export sedan), PMH (Euro 1.6), PMA (Euro 1.4), PNF (US Si K20A3), PRA (Euro Type R), PRD (JDM Type R). Second-to-last digit = 0 manual / 5 automatic. The lead "PMP" was not found; "EP3 Si = PRA/PND" is wrong for the US Si (it is PNF). HIGH/MEDIUM.
- The DLC is a 16-pin J1962 connector with a K-line on pin 7, a Honda service-check line on pin 9, a WEN line on pin 12, a separate SRS line on pin 14, grounds on 4/5, battery on 16. No CAN. HIGH.
- The US D17 ECM/PCM is updatable through the DLC with the factory Honda Interface Module, and a 2002 software-update bulletin exists. HIGH. The microcontroller itself could not be identified; forum evidence points to an Oki family part rather than SuperH, and no aftermarket tool lists a 2001-2005 D-series Civic. LOW.
- Other diagnosable units: ABS modulator-control unit (EX/GX, 23 codes), SRS unit (three suppliers; ~55+ codes) plus OPDS, EPS (Si only, 20 codes), multiplex control unit (own inspection connector, not on the DLC), heater panel. Two-digit MIL/"D"-indicator blink codes still apply. HIGH.
- The powertrain code set for US D17 cars 2001-2003 is 77 engine codes (45 P0xxx, 32 P1xxx) plus 11 automatic and 10 CVT codes; the complete factory tables are transcribed below. Model year matters: sensor sets and ECM part numbers change at 2003 and at the 2004 facelift. HIGH.

## Raw notes log (full findings, by question; each fact carries its confidence tag and inline source)

### R1. Honda Export Service Bulletin 00-098 (Feb 2001), "2001 Civic Product Update: Powertrain Control Module" - PRIMARY SOURCE
Source: http://www.geocities.ws/pfeifferperformance/00-098CivicProductUpdate-PCM.pdf (read as PDF, 3 pages)

- Background (verbatim): "A power surge may damage components in the ECM/PCM. If this happens, the MIL may come on with a DTC 5-1 (short in transmission range circuit) stored."
- ECM/PCM part numbers for 2001 Civic sedan by destination code (verbatim table):

| Destination | Trim / transmission | ECM/PCM P/N |
|---|---|---|
| KK | LX 5-speed | 37820-PLM-M01 |
| KK | EX 5-speed | 37820-PLR-K01 |
| KK | LX A/T | 37820-PLM-M51 |
| KK | EX A/T | 37820-PLR-K51 |
| KX | LX/EX 5-speed | 37820-PLR-K01 |
| KX | LX/EX A/T | 37820-PLR-K51 |
| KY/KV | Exi 5-speed | 37820-PLC-Y01 |
| KY/KV | Exi A/T | 37820-PLC-Y51 |
| KU | Exi A/T | 37820-PLC-U51 |
| KU | Exi 5-speed | 37820-PLC-U01 |
| KA (USA) | DX/LX 5-speed | 37820-PLM-A03 |
| KA (USA) | DX/LX A/T | 37820-PLM-A53 |

- VIN prefixes in the bulletin: 1HGES15/1HGES16 (US-built sedan M/T / A/T), 2HGES15/2HGES16 (Canada-built), JHMES85/JHMES86 (Japan-built, KY/KV/KU).
- Suffix convention visible in the table: x0x = manual-transmission ECM, x5x = automatic-transmission PCM (A03 vs A53, M01 vs M51, K01 vs K51, Y01 vs Y51, U01 vs U51). HIGH.
- Replacement procedure uses the **PGM Tester** (the pre-HDS handheld) on the 16P DLC: VIN entry, odometer entry, "REPLACE ECM/PCM", password entry, then "the registration process for the immobilizer is complete". So a 2001 Civic ECM/PCM carries the immobilizer function and key registration is done through the DLC. HIGH.
- ECM/PCM location: behind the glove box (passenger side on LHD). HIGH.
- After replacement: idle for 10 minutes after warm-up "to allow the ECM/PCM to perform the idle learning operation". HIGH.

### R2. US 2002 Civic service manual (ALLDATA copy on charm.li), "DTC List - Fuel and Emissions" - PRIMARY SOURCE (table images read directly)
Source page: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Fuel%20and%20Emissions/
Images: https://charm.li/images/IMP02Q313/asian140/5812834/ , .../5839613/ , .../5862345/ , .../5887455/

Column header in the manual: "DTC (MIL indication*)", "Temporary DTC", "Detection Item". Footnote *: "These DTCs are indicated by a blinking Malfunction Indicator Lamp (MIL) when the SCS service signal line is jumped with the Honda PGM Tester." Footnote **: "The D indicator and the MIL may come on simultaneously."
Other footnotes: *1 D17A2, D17A6 engines; *2 A/T, CVT; *3 except Canada model; *4 D17A6 engine; *5 M/T, A/T; *6 D17A1, D17A2 engines; *7 M/T; *8 A/T; *9 CVT; *10 '02-03 models; *11 D17A6 engine ('02-03 models); *12 D17A6 engine ('01 model); *13 '03 model; *14 '01-02 models.

| DTC | MIL blink code | Temporary DTC stored? | Detection item | Applies to |
|---|---|---|---|---|
| P0107 | 3 | no | MAP sensor circuit low voltage | all |
| P0108 | 3 | no | MAP sensor circuit high voltage | all |
| P0112 | 10 | no | IAT sensor circuit low voltage | all |
| P0113 | 10 | no | IAT sensor circuit high voltage | all |
| P0116 | 86 | yes | ECT sensor range/performance problem | all |
| P0117 | 6 | no | ECT sensor circuit low voltage | all |
| P0118 | 6 | no | ECT sensor circuit high voltage | all |
| P0122 | 7 | no | TP sensor circuit low voltage | all |
| P0123 | 7 | no | TP sensor circuit high voltage | all |
| P0128 | 87 | yes | Cooling system malfunction | all |
| P0131 | 1 | no | Primary HO2S (sensor 1) circuit low voltage | D17A1, D17A2 (footnote superscript small; read as *6) |
| P0132 | 1 | no | Primary HO2S (sensor 1) circuit high voltage | D17A1, D17A2 |
| P0133 | 61 | yes | Primary HO2S (sensor 1) slow response | D17A1, D17A2 |
| P0134 | 41 | no | A/F sensor (sensor 1) no activity detected | D17A6 |
| P0135 | 41 | no | Primary HO2S (sensor 1) heater circuit malfunction | D17A1, D17A2 |
| P0137 | 63 | yes | Secondary HO2S (sensor 2) circuit low voltage | all |
| P0138 | 63 | yes | Secondary HO2S (sensor 2) circuit high voltage | all |
| P0139 | 63 | yes | Secondary HO2S (sensor 2) slow response | all |
| P0141 | 65 | no | Secondary HO2S (sensor 2) heater circuit malfunction | all |
| P0143 | 101 | yes | Third HO2S (sensor 3) circuit low voltage | D17A6 '02-03 |
| P0144 | 103 | yes | Third HO2S (sensor 3) circuit high voltage | D17A6 '02-03 |
| P0145 | 103 | yes | Third HO2S (sensor 3) slow response | D17A6 '02-03 |
| P0147 | 104 | yes | Third HO2S (sensor 3) heater circuit malfunction | D17A6 '02-03 |
| P0171 | 45 | yes | Fuel system too lean | all |
| P0172 | 45 | yes | Fuel system too rich | all |
| P0300 (+ some of P0301-P0304) | 71-74 | yes | Random misfire | all |
| P0301 | 71 | yes | No. 1 cylinder misfire | all |
| P0302 | 72 | yes | No. 2 cylinder misfire | all |
| P0303 | 73 | yes | No. 3 cylinder misfire | all |
| P0304 | 74 | yes | No. 4 cylinder misfire | all |
| P0325 | 23 | no | Knock sensor circuit malfunction | all |
| P0335 | 4 | no | CKP sensor no signal | all |
| P0336 | 4 | no | CKP sensor intermittent interruption | all |
| P0340 | 8 | no | CMP sensor no signal | '03 model |
| P0341 | 8 | no | CMP sensor intermittent interruption | '03 model |
| P0401 | 80 | yes | EGR insufficient flow | D17A2, D17A6 |
| P0420 | 67 | yes | Catalyst system efficiency below threshold | all |
| P0451 | 91 | yes | FTP sensor range/performance problem | all |
| P0452 | 91 | yes | FTP sensor circuit low voltage | all |
| P0453 | 91 | yes | FTP sensor circuit high voltage | all |
| P0500 | 17 | no | VSS circuit malfunction | M/T |
| P0501 | 17 | no | VSS range/performance problem | A/T |
| P0505 | 14 | yes | Idle control system malfunction | all |
| P0563 | 34 | no | ECM/PCM power source circuit unexpected voltage | all |
| P0600 | 39 | no | Serial communication link malfunction. Note: "Refer to the Multiplex Control system Troubleshooting" | all |
| P07xx, P0700 | 70 | no | Automatic transaxle system malfunction. Note: "Refer to the Automatic Transmission DTC Troubleshooting Index" | A/T, CVT |
| P1106 | 13 | yes | BARO sensor range/performance problem | all |
| P1107 | 13 | no | BARO sensor circuit low voltage | all |
| P1108 | 13 | no | BARO sensor circuit high voltage | all |
| P1121 | 7 | yes | TP sensor signal lower than expected | all |
| P1122 | 7 | yes | TP sensor signal higher than expected | all |
| P1128 | 5 | yes | MAP sensor signal lower than expected | all |
| P1129 | 5 | yes | MAP sensor signal higher than expected | all |
| P1130 | 111 | yes | Demand for changing both secondary HO2S (sensor 2) and third HO2S (sensor 3) | D17A6 '02-03 |
| P1149 | 61 | yes | A/F sensor (sensor 1) range/performance problem | D17A6 |
| P1157 | 48 | no | A/F sensor (sensor 1) range/performance problem | D17A6 '02-03 |
| P1158 | 48 | no | A/F sensor (sensor 1) AFS- terminal low voltage | D17A6 '02-03 |
| P1159 | 48 | no | A/F sensor (sensor 1) AFS+ terminal low voltage | D17A6 '02-03 |
| P1162 | 48 | no | A/F sensor (sensor 1) circuit malfunction | D17A6 '01 |
| P1163 | 61 | yes | A/F sensor (sensor 1) slow response | D17A6 |
| P1164 | 61 | yes | A/F sensor (sensor 1) range/performance problem | D17A6 |
| P1165 | 61 | yes | A/F sensor (sensor 1) range/performance problem | D17A6 |
| P1166 | 41 | no | A/F sensor (sensor 1) heater circuit malfunction | D17A6 |
| P1167 | 41 | no | A/F sensor (sensor 1) heater system malfunction | '01-02 models |
| P1259 | 22 | no | VTEC system malfunction | D17A2, D17A6 |
| P1297 | 20 | no | ELD circuit low voltage | except Canada |
| P1298 | 20 | no | ELD circuit high voltage | except Canada |
| P1324 | 23 | no | Knock sensor power source circuit low voltage | all |
| P1361 | 8 | no | TDC sensor intermittent interruption | '01-02 models |
| P1362 | 8 | no | TDC sensor no signal | '01-02 models |
| P1420 | 105 | yes | NOx adsorptive catalyst system efficiency below threshold (M/T) | D17A6 '02-03 |
| P1456 | 90 | yes | EVAP control system leakage (fuel tank system) | all |
| P1457 | 90 | yes | EVAP control system leakage (EVAP canister system) | all |
| P1491 | 12 | yes | EGR valve insufficient lift | D17A2, D17A6 |
| P1498 | 12 | no | EGR valve position sensor circuit high voltage | D17A2, D17A6 |
| P1505 | 109 | yes | PCV air leakage | '02-03 models |
| P1519 | 14 | no | IAC valve circuit malfunction | all |
| P1607 | (blank) | no | ECM/PCM internal circuit malfunction | all |
| P17xx | 70 | (P17xx) | Automatic transaxle system malfunction (refer to A/T DTC index) | A/T, CVT |
| P18xx | 70 | no | Automatic transaxle system malfunction (refer to A/T DTC index) | CVT |

Count: 77 distinct engine-side P-codes (45 P0xxx + 32 P1xxx) plus the P07xx / P17xx / P18xx transmission families. HIGH (primary table).
Model-year signal inside the table: '01-02 cars have a TDC sensor (P1361/P1362) while the '03 model has a CMP sensor (P0340/P0341); P1505 (PCV) and the D17A6 third-HO2S/NOx-catalyst codes only exist from '02. So the ECM/PCM software (and sensor set) differs between '01, '02 and '03. HIGH.

### R3. US 2002 Civic service manual (charm.li), A/T, CVT, ABS and SRS DTC lists - PRIMARY SOURCE (table images read directly)

A/T (4-speed automatic, "Without Continuously Variable Transmission"), image https://charm.li/images/IMP02Q313/asian140/7286374/
Footnote (1): "The DTC in parentheses is the flash code the D indicator indicates when the data link connector (DLC) is connected to the Honda PGM Tester, and the Tester is in the SCS mode." Footnote (2): electrical-only cause.

| DTC | D-indicator flash code | D indicator | MIL | Detection item |
|---|---|---|---|---|
| P0715 | 15 | Blinks | ON | Mainshaft speed sensor |
| P0720 | 9 | Blinks | ON | Countershaft speed sensor |
| P0730 | 41 | OFF | ON | Shift control system |
| P0740 | 40 | OFF | ON | Lock-up control system |
| P0753 | 7 | Blinks | ON | Shift solenoid valve A |
| P0758 | 8 | Blinks | ON | Shift solenoid valve B |
| P1705 | 5 | Blinks | ON | Transmission range switch (short to ground) |
| P1706 | 6 | OFF | ON | Transmission range switch (open) |
| P1753 | 1 | Blinks | ON | Torque converter clutch solenoid valve |
| P1768 | 16 | Blinks | ON | A/T clutch pressure control solenoid valve A |
| P1773 | 23 | Blinks | ON | A/T clutch pressure control solenoid valve B |

CVT (HX coupe, GX), image https://charm.li/images/IMP02Q313/asian140/7249887/

| DTC | D-indicator flash code | D indicator | MIL | Detection item |
|---|---|---|---|---|
| P1705 | 5 | Blinks | ON | Transmission range switch (short to ground) |
| P1706 | 6 | OFF | ON | Transmission range switch (open) |
| P1870 | 30 | Blinks | ON | CVT speed change control valve assembly |
| P1873 | 31 | Blinks | ON | CVT pulley pressure control valve assembly |
| P1879 | 32 | Blinks | ON | CVT start clutch pressure control valve assembly |
| P1882 | 33 | Blinks | OFF | Inhibitor solenoid |
| P1885 | 34 | Blinks | ON | CVT drive pulley speed sensor |
| P1886 | 35 | Blinks | ON | CVT driven pulley speed sensor |
| P1888 | 36 | Blinks | ON | CVT speed sensor |
| P1889 | 93 | Blinks | ON | CVT speed sensor 2 |
| P1890 | 42 | Blinks | ON | Shift control system |
| P1891 | 43 | Blinks | ON | Start clutch control system |

Note: the "DTC 5-1 (short in transmission range circuit)" quoted in bulletin 00-098 is consistent with flash code 5 = P1705 (the "-1" is probably a PGM Tester sub-code). MEDIUM (inference).

ABS (US, where fitted), image https://charm.li/images/IMP02Q313/asian140/4614496/ - 23 two-digit codes, no P/C codes:

| DTC | Detection item |
|---|---|
| 11 / 13 / 15 / 17 | RF / LF / RR / LR wheel sensor (open / short to body ground / short to power) |
| 12 / 14 / 16 / 18 | Faulty RF / LF / RR / LR wheel sensor pulse signal |
| 21 | Continuous operation (chipped pulser) |
| 31-38 | Solenoid (eight solenoid codes) |
| 51 | Motor lock |
| 52 | Motor stuck off |
| 53 | Motor stuck on |
| 54 | Main relay stuck off |
| 61 | Ignition voltage |
| 81 | CPU (Central Processing Unit) |

SRS: the manual has two code lists because two different SRS units were fitted: "NEC (M2) SRS unit (without side airbags)" and a "Siemens SRS Unit" list. NEC list, images https://charm.li/images/IMP02Q313/asian140/6741437/ and .../6769678/:

| DTC | Detection item |
|---|---|
| 1-1 .. 1-5 | Driver's airbag inflator: open / increased resistance / short to another wire or decreased resistance / short to power / short to ground |
| 2-1 .. 2-5 | Front passenger's airbag inflator (same five sub-codes) |
| 3-1 .. 3-5 | Driver's seat belt tensioner |
| 4-1 .. 4-5 | Front passenger's seat belt tensioner |
| 21-1 .. 21-5 | Driver's seat belt buckle tensioner |
| 22-1 .. 22-5 | Front passenger's seat belt buckle tensioner |
| 5-1 .. 5-5, 6-1 .. 6-4, 7-1 .. 7-3, 8-1 .. 8-6 | Internal failure of SRS unit |
| 9-1 | Internal failure of the SRS unit / faulty indicator circuit |
| 9-2 | Internal failure of the SRS unit / power supply (VB line) |
| 9-3 / 9-4 | Faulty driver's / front passenger's seat belt buckle switch |
| 9-6 / 9-7 | Faulty left / right front sensor |
| 10-1 | Seat belt and seat belt buckle tensioners (and airbag(s)) deployed |

SRS second list heading (image https://charm.li/images/IMP02Q313/asian140/6852530/): "SIEMENS (M1), KARE (M3) SRS unit (with side airbags)". Same code families as the NEC list but x-2 sub-codes merged into x-1 ("Open or increased resistance"), internal-failure codes 5-1, 5-2, 5-4, 5-8, 6-3, 6-4, 6-7, 6-8, 7-1..7-3, 8-1..8-8, and further families 9-x .. 15-x for side airbags / side impact sensors / OPDS (index lists 9-1..9-7 except 9-5, 10-1..10-7, 11-1, 11-3..11-5, 12-1, 12-3..12-5, 13-1..13-4, 14-1..14-4, 15-1..15-3). HIGH that three SRS unit suppliers exist: NEC (M2, no side airbags), Siemens (M1) and "KARE" (M3) (with side airbags).

### R4. US 2002 Civic service manual (charm.li): ECM/PCM update, immobilizer, ECM/PCM connectors - PRIMARY SOURCE

- "Updating the ECM/PCM" / "ECM/PCM Updating and Substitution for Testing" pages (Engine Control Module > Testing and Inspection > Programming and Relearning). Special tool: Honda Interface Module (HIM), P/N EQS05A35570. Procedure: ignition ON (II), connect the HIM to the DLC (under the driver's side of the dashboard), follow the update system; do not switch the ignition off during the update ("the ECM/PCM can be damaged"), battery fully charged, no accessories. "Update the ECM/PCM whenever it is substituted or replaced" (paraphrase by fetch tool). Substitution for testing: install known-good ECM/PCM, "rewrite the immobilizer code" with the "ECM/PCM replacement procedure" in the Honda PGM Tester; repeat when the original unit goes back. HIGH.
  Source: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Powertrain%20Management/Relays%20and%20Modules%20-%20Computers%20and%20Control%20Systems/Engine%20Control%20Module/Testing%20and%20Inspection/Programming%20and%20Relearning/Updating%20the%20ECM%2FPCM/
- Honda Service Bulletin 02-011 (first issued 26 Feb 2002, later revised): "ECM/PCM Software Update" for 2002 Civic EX. "Use the HDS to update the software in the ECM/PCM", software "from the January 2002 or later Hondanet CD", refers to S/B 01-023 "Updating Control Units/Modules". Failed part given as "P/N 37520-PLR-L54" (as transcribed by the fetch tool; almost certainly 37820-PLR-L54, a California-spec EX A/T PCM - to be verified). HIGH that a D17A2 2002 ECM/PCM was field-reflashed by dealers through the DLC.
  Source: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Powertrain%20Management/Relays%20and%20Modules%20-%20Computers%20and%20Control%20Systems/Engine%20Control%20Module/Technical%20Service%20Bulletins/All%20Technical%20Service%20Bulletins/Engine%20Controls%20-%20ECM%20Software%20Update/
- Honda Service News, May 2003: 2002-03 Civic with DTC P1607 ("ECM/PCM internal circuit malfunction"): units "may have defective printed circuit boards", remedy is to replace the ECM/PCM. MEDIUM-HIGH.
- Other ECM-related bulletins listed for the 2002 EX: "A/T, Engine Controls - MIL ON/No Movement In Drive/P0730", "Engine Controls - MIL ON/DTC P0128 Is Stored In Memory", "Emissions - OBD II DTC's And Associated Monitors".
- Immobilizer (US 2002): components are the transponder key, the "immobilizer control unit-receiver" (in the steering column cover), the immobilizer indicator, PGM-FI main relays and the ECM/PCM. "The ECM/PCM then sends power to the ignition key transponder through the immobilizer control unit-receiver"; the transponder's coded signal is returned to the ECM/PCM through the receiver; the ECM/PCM enables fuel (grounds PGM-FI main relay 2) only if the code matches. So key authorisation lives in the ECM/PCM; the receiver is an antenna/amplifier front end, not a DLC node. HIGH.
  Source: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Accessories%20and%20Optional%20Equipment/Alarm%20Module/Description%20and%20Operation/

ECM/PCM connector views (US D17 cars, 2001-2005; "228. ECM/PCM - Behind glove box"), images https://charm.li/images/IMP02Q313/asian140/239558802/ (A), .../239573049/ (B), .../239581369/ (C), .../239588844/ (E):

| Connector | Colour | Cavities | Harness | Contents |
|---|---|---|---|---|
| A | Gray | 31 | Engine wire harness | Sensors and ignition: PO2SHTC/AFSHTC, IGP1/2, PG1/2, PHO2S or AFS(+), CKP, VCCR, KS, SG1/2, IACV, EGRP, SO2SHTC, TPS, AFS(-), INJ MODE (GX), VSS (CVT: VEL2), MAP, VCC1/2, LG1/2, SHO2S, TDC, IGPLS1-4 |
| B | White | 24 | Engine wire harness | INJ1-4, FANC, LSA+/HLCLS+, ECT, VTPSW, ALTL, ALTF, EGR, VTS, LSB+/SCLS+, IAT, ALTC, PCS, SHLS+ |
| C | Black | 22 | Engine wire harness | A/T and CVT only: solenoids (LSA-, LSB-, SHA, SHB, LC), speed sensors (NM, NC, NMSG, NCSG), range switch inputs (ATPR, ATPNP, ATP2, ATPD3, ATPD4, ATPFWD); CVT set (HLCLS-, INHSOL, NDR, NDN, SCLS-, SHLS, VEL1, ATPS, ATPL, ATPD) |
| E | White | 31 | ECM/PCM wire harness (body side) | See pin table below |

Connector E pins that matter to this project (verbatim signal names and wire colours):

| Pin | Wire colour | Signal | Meaning (signal names are Honda's; expansions are mine) |
|---|---|---|---|
| E1 | GRN/YEL | IMOFPR | Immobilizer fuel pump relay control |
| E7 | RED/YEL | MRLY | Main relay |
| E9 | YEL/BLK | IG1 | Ignition power |
| E18 | RED | ACC | A/C clutch relay ('01 and '02-'05 listed separately) |
| E22 | WHT/BLK | BKSW | Brake switch |
| E23 | LT BLU | K-LINE | Diagnostic K-line to the DLC |
| E24 | YEL | SEFMJ | Serial link to the multiplex / gauge side (the line behind P0600) |
| E26 | BLU | NEP | Engine speed pulse output |
| E27 | RED/BLU | IMOCD | Immobilizer code line to the immobilizer control unit-receiver |
| E29 | BRN | SCS | Service check signal |
| E30 | RED/WHT | WEN | Write enable (present on DX, LX, EX, GX, HX) |
| E31 | GRN/ORN | MIL | Malfunction indicator lamp |

Running changes visible in the connector tables: '01-'03 DX/LX/EX use a conventional primary HO2S (PHO2S, PO2SHTC) and '04-'05 DX/LX/EX switch to an A/F sensor (AFS+/-, AFSHTC, AFSHTCR) and DX/LX gain EGR (EGR, EGRP) - so '04-'05 ECM/PCMs are not interchangeable with '01-'03 ones. HIGH.

### R5. US 2002 Civic service manual (charm.li): multiplex control system, Si (EP3) control units - PRIMARY SOURCE

- Multiplex control system ("Information Bus" in ALLDATA's tree): "There are exclusive communication lines between the ECM/PCM, the gauge assembly, and the under-dash fuse/relay box." Functions: entry light control, wiper/washer, interlock, keyless/power door lock, key-in reminder, meter assembly / temperature gauge / indicators, HVAC compressor and fan control. Self-diagnosis "Mode 1 for the system. Mode 2 for input lines." HIGH.
  Source: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Information%20Bus/Description%20and%20Operation/System%20Description/
- Multiplex troubleshooting is NOT done through the DLC: a special tool (short connector) is plugged into the "multiplex inspection connector" behind the dashboard lower cover; after ~5 s the spotlight and ceiling light blink the mode indication and then the DTCs (ascending order). Mode 2 = switch input test (one blink per good input). Sleep-mode check: communication wire at terminal 10 of under-dash fuse/relay box connector E (YEL) or K (WHT/GRN) goes from about 4-10 V to battery voltage, current from 70-80 mA to under 10 mA. HIGH.
  Source: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Information%20Bus/Testing%20and%20Inspection/Multiplex%20Control%20System%20Troubleshooting/
- 2002 Civic Si (EP3, K20A3) module list from the same manual set: ECM, ABS ("Electronic Brake Control Module"), EPS ("Steering Control Module"), SRS unit, OPDS ("Seat Occupant Classification Module"), immobilizer ("Alarm Module"), keyless, HVAC control module with its own DTC list (codes 7, 8, 9, 10, 11, 12, 14, 15), "Fuel Pump Control Unit". DTC lists exist for ABS, EPS, Fuel and Emissions, Heating/Air Conditioning, SRS. HIGH.
  Source: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/
- Si P-code charts (64): P0010, P0011, P0107, P0108, P0112, P0113, P0116, P0117, P0118, P0122, P0123, P0128, P0134, P0137, P0138, P0139, P0141, P0171, P0172, P0300-P0304, P0325, P0335, P0336, P0340, P0341, P0344, P0420, P0451-P0453, P0500, P0505, P0563, P0600, P0700, P1106-P1108, P1121, P1122, P1128, P1129, P1157-P1159, P1163, P1164, P1166, P1167, P1259, P1297, P1298, P1361, P1362, P1399, P1456, P1457, P1505, P1519, P1607. HIGH.
- EPS DTCs (Si), image https://charm.li/images/IMP02Q313/asian140/4897827/:

| DTC | Detection item |
|---|---|
| 12 | Voltage for torque sensor T/SIG |
| 16 | Average of voltage for torque sensor VT3 and VT6 |
| 17 | Torque sensor 12 V power source Vcc1 |
| 18 | Torque sensor 5 V power source Vcc2 |
| 22 | Average for vehicle speed and engine speed / excessive change of the vehicle speed sensor signal |
| 23 | Engine speed signal circuit |
| 37 | Circuit for input motor voltage in the EPS control unit |
| 41 | Motor voltage |
| 42 | Motor driven current (open circuit or short to ground) |
| 43 | Motor driven current (short to power) |
| 45 | Motor driven current (open circuit or short to ground) |
| 47 | Motor relay in the EPS control unit |
| 50 | CPU or microcomputer in the EPS control unit |
| 51 | EEPROM in the EPS control unit |
| 62 | Fail-safe relay stuck ON |
| 64 | Low battery voltage / fail-safe relay contact failure |
| 66 | Motor driven voltage |
| 67 | Torque sensor I/F circuit |
| 68 | Interlock circuit (torque) |
| 69 | Interlock circuit (current) |

### R6. ECU microcontroller evidence (web)

- Hondata K-Pro compatible ECU list (verbatim): "PRB (A01-A12 & 305) PND (A01-A09) PNF PRA PRC (JDM DC5 Integra Type R) PRD PPA (A0#, A5#) PPL (A0#, A5#)"; vehicles "Civic Si 2002-2005, Civic Type R 2001-2005, RSX Type S 2002-2004, RSX Base 2002-2004, Integra Type R 2002-2006", MT only; K-Pro "consists of a hardware modification to any K-Series ECU". https://hondata.com/kpro/kpro-product . Vendor listings: PNF = 02-05 US Civic Si, PRA = Euro Civic Type R, PRD = JDM Type R, PRB = RSX Type S, PND = RSX base. HIGH for the list, MEDIUM for the per-code assignment.
- Hondata FAQ (mirrored 2008): K-Pro adds "a daughterboard and a USB connector to your ECU"; a separate "reflash" product existed for RSX / Civic Si ("You send in your ECU, immobilizer and key, and Hondata will program the ECU"). https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/ MEDIUM.
- PCMflash module 13 "Honda/Acura K-Line" list (no 2001-2005 Civic entry): Accord 7 2003-2005 Matsushita SH7055/512KB; CR-V 2002/2005-2007 Keihin PPA/PNL SH7058/1MB; Element/Inspire Keihin SH7055/512KB; "Honda: Fit/Jazz/Airwave 2001-2007 (Keihin OKI ML66Q592/192KB)". https://ecutools.eu/chip-tuning/pcmflash/ MEDIUM.
- MMC Flasher module 30 seller page: "862 Honda/Acura K-line ECU configurations", write only, includes 37820-PNC-305/306/307, 37820-PND-307/405, 37805-RCJ-336. https://ecupro.eu/30-modul-hondaacura-k-line-mmc-flash MEDIUM.
- MHH Auto forum (search-result excerpts, pages not fetchable): about a 37820-PMH-E02 (European 1.6 D16V1) "these ECUs have OTP OKI MCU and is not possible to write"; "keihin ecu of EP models are impossible to do because are OKI OTP memories"; a second thread about a 2004 Civic Keihin: "probably OKI chip is OTP, even if can read, cant write". https://mhhauto.com/Thread-Remap-keihin-37820-PMH-E02 , https://mhhauto.com/Thread-Honda-Civic-2004-Keihin-DTC-Off LOW-MEDIUM.
- Hondata forum (search-result excerpt): Hondata "evaluated what options we have with the D17, and have determined that the only economically feasible option will not work, so it is unlikely that we will develop anything for the D17". https://hondata.com/forum/viewtopic.php?start=30&t=502 MEDIUM.
- ChipSoft J2534 adapter note quoted on ecutools.eu: "Set programming voltage +5V to 12 pin of OBD2 socket (Honda reflash)" / "12 - Boot Pin for programming Honda ECU (only +5V can be set)". https://ecutools.eu/chip-tuning/mmc-flasher/ MEDIUM (cross-cutting).

### R7. Engines by market (Wikipedia, Honda D engine + 7th-gen Civic articles) - MEDIUM
- D14Z5: ES4 pre-facelift, Europe, ECU code PMA. D14Z6: EP1, EU4(?), EU7, ES4 facelift, ES6, Europe, ECU PMA. D16V1: European EM, EP2, ES, EU8, ECU PMH. D16W7: ES, Europe/Turkey/Singapore. D16W9: VTi Philippines/Pakistan/Middle East (3-stage VTEC). D15Y3: EXi Africa/Dubai/Pakistan. D15Y4: ES8 Singapore/Malaysia/Sri Lanka. D15B 3-stage VTEC: JDM. D17A: Civic Japan, ECU "37820 PLR J01-13 (manual)". D17A1: DX/LX/VP. D17A2: EX (US), LX (Europe), Si (Canada), Acura EL. D17A5: 170i VTEC South Africa. D17A6: HX. D17A7: GX (CNG). D17A8: Coupe LS Europe. D17A9: Coupe ES Europe. D17Z2/D17Z3: Brazil LX / EX.

## Conflicts and uncertainties

- "Stock D17 ECU cannot be reflashed" (forums) vs the factory "Updating the ECM/PCM" procedure and software-update bulletin S/B 02-011. Resolution: Honda could reprogram it with dealer tools (HIM/HDS) through the DLC; what is absent is any aftermarket tool or public protocol for it. Both facts are HIGH and are not actually in conflict once "who can write it" is separated from "can it be written".
- ECM microcontroller: forum excerpts say European D-series Keihin units use an Oki one-time-programmable microcontroller; the coordinator's lead said SH7052/SH7055 (SuperH). No source ties a 2001-2005 Civic to either. Unresolved; the Oki hypothesis is only LOW.
- Fit/Jazz Keihin Oki part size is given as "ML66Q592/192KB" by one vendor and "256KB" by another. Minor, and not a Civic part anyway.
- S/B 02-011 failed-part number was printed by the summarising fetch tool as "37520-PLR-L54"; the 37820- prefix is almost certainly correct. Treat the exact suffix as MEDIUM until the bulletin image is read.
- Model codes are reused across markets (ES1 = US 1.7 sedan vs JDM 1.5 Ferio). Any per-market mapping is only as strong as its single Wikipedia source (MEDIUM/LOW).
- Whether the European UK-built hatchback's fuse-box control unit participates in engine-start authorisation (as two forum threads imply) is unverified and differs from the US architecture. LOW.

## Open questions (and how to close them)

- Which microcontroller and memory are in the 37820-PLM/PLR ECM/PCM? Close by reading the chip markings off a scrap D17 ECU and matching a datasheet, or by identifying the part in the HDS/HIM update package with Binwalk/Ghidra.
- Exact DLC rewrite protocol and the role of the WEN (pin 12) and SCS (pin 9) lines during an ECM update. Close by capturing the HIM or HDS talking to the car on the K-line, or by reverse engineering the HDS update software and the update files (REA targets).
- Full ABS, SRS and EPS diagnostic command sets, and whether the SRS reads over DLC pin 14 at a different rate than the ECM K-line. Close by reading the image-only factory troubleshooting pages in full and by bus capture.
- European and Japanese DLC wiring and immobilizer architecture (is it EOBD on the same K-line? does a body/fuse-box unit gate start?). Close with a European or Japanese factory service manual.
- Diesel 1.7 CTDi ECU maker and type (prior knowledge suggests a Bosch EDC15-class unit; unverified). Close with EOBD tuner databases or the European manual.
- Complete HX (D17A6) and GX (D17A7) ECM part-number families, and the full 2004-2005 part-number list. Close with a Honda parts catalogue by VIN.

## Sources

- Honda Export Service Bulletin 00-098 (Feb 2001, "2001 Civic Product Update: PCM"): http://www.geocities.ws/pfeifferperformance/00-098CivicProductUpdate-PCM.pdf - primary PDF; ECM part numbers, PGM Tester procedure, immobilizer registration.
- charm.li 2002 Civic EX service manual, Fuel/Emissions DTC list (P-code table, read from images): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Fuel%20and%20Emissions/
- charm.li A/T DTC list: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Transmission/Without%20Continuously%20Variable%20Transmission/
- charm.li CVT DTC list: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Transmission/With%20Continuously%20Variable%20Transmission/
- charm.li ABS DTC list: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Antilock%20Brake%20System/
- charm.li SRS NEC list: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Supplemental%20Restraint%20System/NEC%20SRS%20Unit/
- charm.li SRS Siemens/KARE list: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Supplemental%20Restraint%20System/Siemens%20SRS%20Unit/
- charm.li ECM/PCM connector views (A/B/C/E pinouts): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Powertrain%20Management/Relays%20and%20Modules%20-%20Computers%20and%20Control%20Systems/Engine%20Control%20Module/Diagrams/Connector%20Views/ECM%2FPCM/
- charm.li DLC connector view (16-pin pinout): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Data%20Link%20Connector/Diagrams/Connector%20Views/
- charm.li DLC circuit test (terminal functions): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Data%20Link%20Connector/Testing%20and%20Inspection/
- charm.li ABS modulator-control unit connector view: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Brakes%20and%20Traction%20Control/Electronic%20Brake%20Control%20Module/Diagrams/
- charm.li SRS unit connector view: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Restraints%20and%20Safety%20Systems/Air%20Bag%20Control%20Module/Diagrams/
- charm.li Updating the ECM/PCM (HIM procedure): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Powertrain%20Management/Relays%20and%20Modules%20-%20Computers%20and%20Control%20Systems/Engine%20Control%20Module/Testing%20and%20Inspection/Programming%20and%20Relearning/Updating%20the%20ECM%2FPCM/
- charm.li ECM software-update bulletin (S/B 02-011): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Powertrain%20Management/Relays%20and%20Modules%20-%20Computers%20and%20Control%20Systems/Engine%20Control%20Module/Technical%20Service%20Bulletins/All%20Technical%20Service%20Bulletins/Engine%20Controls%20-%20ECM%20Software%20Update/
- charm.li immobilizer description: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Relays%20and%20Modules/Relays%20and%20Modules%20-%20Accessories%20and%20Optional%20Equipment/Alarm%20Module/Description%20and%20Operation/
- charm.li multiplex system description: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Information%20Bus/Description%20and%20Operation/System%20Description/
- charm.li multiplex troubleshooting (Mode 1 / Mode 2): https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Information%20Bus/Testing%20and%20Inspection/Multiplex%20Control%20System%20Troubleshooting/
- charm.li Civic Si (EP3) DTC index: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/
- charm.li Civic Si EPS DTC list: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/Diagnostic%20Trouble%20Code%20Descriptions/DTC%20List%20-%20Electrical%20Power%20Steering/
- charm.li scan-tool usage page: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Testing%20and%20Inspection/Scan%20Tool%20Testing%20and%20Procedures/How%20to%20Use%20the%20Honda%20PGM%20Tester%20or%20A%20Scan%20Tool/
- Wikipedia, 7th-generation Civic: https://en.wikipedia.org/wiki/Honda_Civic_(seventh_generation)
- Wikipedia, Honda D engine (engine/market/ECU-code mapping): https://en.wikipedia.org/wiki/Honda_D_engine
- Wikipedia, Honda Civic Type R (EP3): https://en.wikipedia.org/wiki/Honda_Civic_Type_R
- Hondata K-Pro ECU list: https://hondata.com/kpro/kpro-product
- Hondata FAQ mirror (K-Pro hardware, reflash): https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/
- Hondata D17 forum post: https://hondata.com/forum/viewtopic.php?start=30&t=502
- PCMflash K-line Honda module list: https://ecutools.eu/chip-tuning/pcmflash/
- MMC Flash module 30 (Honda K-line) list: https://ecupro.eu/30-modul-hondaacura-k-line-mmc-flash
- Keihin SH7055/SH7058 flasher module list: https://ecutools.eu/chip-tuning/ecuflash-tool/module-honda-ecu-keihin-matsushita-motorola-mcu-sh7058-sh7055-k-line/
- Forum excerpts (not directly fetchable; LOW): Keihin PMH "OKI OTP" thread https://mhhauto.com/Thread-Remap-keihin-37820-PMH-E02 ; 2004 Civic Keihin thread https://mhhauto.com/Thread-Honda-Civic-2004-Keihin-DTC-Off ; UK MICU/immobilizer thread https://mhhauto.com/Thread-Honda-Civic-VII-2001-BCM-MICU-Problem-EEprom-IMMO-93c46
- ECM part-number listings: 37820-PLM-A03 (Keihin) https://www.ebay.com/p/1828217967 ; 04-05 PLM-A14 https://www.importapart.com/product/04-05-civic-dx-vp-lx-mt-ecu-ecm-engine-computer-control-unit-37820-plm-a14-4182/ ; PLM C/L series https://modulemechanics.com/product/37820-plm-a03-oem-ecu-w-immobilizer-security-programming-honda-civic/
- Honda ECU part-number format explainer: https://www.hondacivicforum.com/forum/ecu-tuning-fuel-management-33/identifying-your-ecu-75347/
- 2002-2003 Civic factory manual scan (SRS read method, not fetchable): https://www.manualslib.com/manual/1252291/Honda-Civic-2002-2003.html?page=1030
- Community all-DTC list (behind bot paywall): https://www.7thgenhonda.com/threads/list-of-all-possible-dtcs-for-the-2001-2005-honda-civic.8742/
