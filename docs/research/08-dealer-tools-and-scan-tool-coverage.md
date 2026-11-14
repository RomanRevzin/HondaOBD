# 08 - Honda dealer diagnostic tools and aftermarket scan-tool coverage (2001-2005 Civic)

Research date: 2026-11-14. Dimension: what the Honda dealer tool can do on a 7th-generation Civic, which
interfaces carry its lines, how to get the software legitimately, what clones and aftermarket tools cover.

Confidence tags: **HIGH** = primary source (Honda bulletin, factory manual text or diagram, vendor datasheet) or two
independent sources agree; **MEDIUM** = one credible source; **LOW** = single forum post, inference, or conflicting
reports.

Method limits that shaped this file (read before relying on it):

- The shared web-search budget ran out about two thirds of the way through. Everything after that came from direct
  page fetches. Questions 5 (HDS internals) and 6 (Autel / Launch / Snap-on per-function coverage) are therefore thin.
- `techinfo.honda.com` could not be fetched (TLS chain error in the fetch tool). Facts attributed to it come from
  search-engine summaries of its public pages and are tagged MEDIUM at best.
- Honda-Tech, ManualsLib, Scribd and several vendor blogs refused the fetch tool. Forum facts from them are
  search-summary level and tagged LOW.
- The 2002 and 2005 US Civic service information was read through the Operation CHARM mirror (`charm.li`). Its wiring
  diagrams and connector views are scans of Honda's own drawings and were read directly as images.

## Summary

1. The dealer tool lineage is: PGM Tester (a Vetronix Mastertech with a Honda PCMCIA program card, sold to dealers
   until HDS took over, coverage ends around model year 2002-2003), then HDS on a PC/tablet/pocket PC through the
   Honda Interface Module (HIM) or the Teradyne GNA600, then HDS "3G" through the Bosch/SPX MVCI, then the Denso
   DST-i, now the Honda Antares Capsule (HAC) with the i-HDS suite. The legacy HDS application inside today's i-HDS
   suite is what covers a 2002 Civic (HDS covers model years 1992-2016).
2. The 2002 US Civic's 16-pin DLC has only seven populated pins, and Honda's own wiring diagrams show exactly which
   control unit sits on which pin: pin 7 K-line = ECM/PCM + ABS; pin 14 (grey wire, the line tool vendors call
   DIAG-H) = SRS unit + immobilizer control unit-receiver; pin 9 SCS = ECM/PCM + SRS + ABS; pin 12 = WEN (write
   enable input); pins 4, 5, 16 = grounds and battery. The multiplex (body) unit is not on the DLC at all.
3. The device therefore needs two single-wire serial channels (pin 7 and pin 14), a switch to ground on pin 9 (SCS),
   and a controllable +5 V on pin 12 (WEN) for reflashing. A K-line-only ELM327-class adapter can reach the ECM/PCM
   and (electrically) the ABS unit, never the SRS or immobilizer units.
4. "Every possible code" on this car is a short list: PGM-FI (with A/T inside the PCM), ABS, SRS, immobilizer, plus
   EPS on the Civic Si / EP3. Body (multiplex), keyless entry and HVAC have self-diagnosis without any scan tool.
5. Dealer-tool functions confirmed from the manual and bulletins: DTC read/clear, freeze data, data list, snapshot,
   ECM/PCM reset, SCS mode (blink codes for PGM-FI, A/T, ABS, SRS, EPS), actuator commands (EVAP bypass solenoid ON),
   SRS OPDS initialization, immobilizer setup (replace ECM/PCM, replace immobilizer unit, add/delete keys), VIN write
   (2004-2005), and ECM/PCM software update.
6. The ECM/PCM of the 2001-2005 1.7 L Civic is field-reflashable through the DLC: the factory manual has an
   "Updating the ECM/PCM" procedure with the HIM, and bulletins 02-011 and 03-002 order software updates on 2001-2002
   cars. The stretch goal is feasible in principle.
7. Immobilizer writes need a secret from Honda (a tester password in 2001, a 4-digit ECM/PCM code from the dealer
   network today, Security Professional registration for independents). That is the main non-technical blocker.
8. Legitimate access today: i-HDS subscription from store.honda.com (US; a 1-day pass was USD 60 in August 2026),
   HDS annual licence through MaRIS Pay & Go in Europe. Honda lists the DST-i and MVCI as validated, the MongoosePro
   Honda and Dearborn d-briDGe Pro as independently validated, recommends the HAC (USD 896), and accepts generic
   J2534-1/-2 devices without support.
9. SAE publishes the two Honda-specific pieces of the puzzle: J2534-2 defines the Honda DIAG-H pass-thru protocol
   (protocol ID 0x800B) and J2809 defines the Honda ABS/VSA diagnostic protocol. Buying those two documents is the
   cheapest legitimate source of ground truth for the pin-14 line and the ABS dialogue.
10. The protocol names "92Hm/2" and "95Hm-CARB" from the brief could not be found in any public source reached in
    this round. Names that are documented: K-Line / ISO9141, Diag-H, Diag-X, P-Line, SCS, WEN.
11. Clone hardware is useful only as a fact base: the cheap "HDS cable" class is K-line + CAN only and sees just
    PGM-FI and A/T on pre-CAN cars; the "GNA600" clone is a copied Drew Tech Mongoose (FT232R + LPC2148); the HIM
    clone is a two-board RS-232 unit.
12. Best traffic-capture plan: a legitimate i-HDS day pass plus a Honda-capable J2534 interface, logged at the J2534
    API boundary, with a logic analyser on DLC pins 7, 9, 12 and 14 as the electrical cross-check.

## Findings

### 1. Dealer tool history and architecture

| Generation | Hardware | Software | Coverage relevant here | Confidence and source |
|---|---|---|---|---|
| PGM Tester | Vetronix Mastertech (MT3100/MTS 3100) hand-held, co-engineered by Honda and Vetronix; identity comes from a PCMCIA "PGM TESTER PROGRAM CARD, 4MByte Reprogrammable" | On the card: Honda functions, generic OBD-II, DSO | System menu on the tester: PGM-FI, A/T, SRS, DBW, IMMOBI, ABS/TCS, ATTS. "Only offers coverage until 2003" per AutoInc; one vendor listing says 1994-2002 | HIGH for architecture ([Tech-2-Tech module 27][tt27]); MEDIUM for the end year ([AutoInc 2014][autoinc]) |
| HIM | Honda Interface Module, P/N EQS05A35570. Used standalone for ECM/PCM updates ("as described on the HIM label", status lights, "red (#3) light") and as the PC interface for HDS. Two versions, the later with a CAN daughter module | "ECM/PCM update system" (HondaNet CD), HDS PC | "All modules", OBD-I and OBD-II through 2010 (AutoInc). Honda: covers all reprogrammable vehicles through 2005 | HIGH for P/N and use ([2002 manual][cm-ecm-update]); MEDIUM for the rest ([AutoInc][autoinc], [Honda legacy tools page][ti-legacy]) |
| HDS Tablet / Pocket Tester | Tablet with integrated HIM; pocket PC | HDS | Same as HIM; described as unreliable | MEDIUM ([AutoInc][autoinc]) |
| GNA600 | Teradyne laptop interface | HDS (2nd generation) | OBD-I through 2010, body and chassis, most key functions, module programming. Replaced by the MVCI | MEDIUM ([AutoInc][autoinc], [Honda GNA600 page][ti-gna600]) |
| MVCI | Bosch/SPX "Modular Vehicle Communications Interface", OTCHON3828AF (discontinued, superseded by the DST-i) | HDS 3G; "CM Update" mode in the MVCI menu, later removed | "With the new 3G HDS software, the MVCI covers Honda OBD-equipped vehicles from model year 1992 onward" | HIGH ([Snap-on MVCI listing][so-mvci], [SB 01-023][sb01023]) |
| DST-i | Denso DSCWIFI (EU listing: "DST-010"), USB or Wi-Fi (discontinued, superseded by BSEHAC001) | HDS and i-HDS | Same | HIGH ([Snap-on DST-i listing][so-dsti]) |
| HAC | Honda Antares Capsule, Bright Star Engineering, BSEHAC001, USD 896 | i-HDS suite (i-HDS + HDS + J2534 Rewrite + Immobilizer Setup Tool) | "100% HDS and i-HDS compatibility" | HIGH ([Snap-on HAC listing][so-hac]) |

Further facts:

- **HIGH** - HDS + MVCI was Honda's prescribed tool for a 2001 Civic as late as June 2014: bulletin 00-098 revision
  tells the technician to connect the HDS through the MVCI, log in to iN, then select IMMOBI from the System
  Selection Menu. [SB 00-098 rev. 2014][sb00098b]
- **MEDIUM** - "HDS software version 3.001.004 and later is ONLY compatible with the MVCI diagnostic tool and will NOT
  work with any of our Legacy Diagnostic Tools (GNA600, HIM, etc.)". The last HDS release usable with a real HIM or
  GNA600 is not stated. [Honda legacy tools page][ti-legacy] (via search summary)
- **MEDIUM** - Software split: the HDS package "covers all vehicles from 1992 to the current model year"; i-HDS is
  for full model changes from model year 2017; the quick start guide calls the rest "Existing HDS Coverage" (up to
  16MY). [Honda Canada update page][ti-che-update], [Honda J2534 page][ti-j2534] (via search summaries)
- **HIGH** - i-HDS first shipped around May 2016 (HDS Download Manager v2.1.98 showing "i-HDS Software 1.001.001",
  "Database Update 12-MAY-2016"). [SB 01-023][sb01023]
- **LOW** - Version strings seen on grey-market listings, useful only to date installers: HDS 3.015.033 (2015),
  3.101.015 / 3.101.044 (2017), 3.105.043 with i-HDS 1.010.011 (05/2025). The legacy HDS application is still being
  released alongside i-HDS. [blog teardown 2015][blog-mk9], [clone vendor blog][blog-hds]
- **HIGH** - HDS has a market setting: "Select F12 ... Set-up Details screen. Make sure Market is set to USA".
  A UK user had to set the software to "EUROPE" for a UK-built car. [SB 00-098 rev. 2014][sb00098b],
  [civicx thread][civicx]

Lines the official interfaces list (verbatim from the Honda tool-programme catalogue):

| Interface | Vehicle-side lines listed | Source |
|---|---|---|
| Bosch MVCI | two-pin selectable dual-wire CAN, single-wire CAN, "K-Line, Honda Diag H, Honda Diag X" | [Snap-on MVCI listing][so-mvci] (HIGH) |
| Denso DST-i | "ISO9141, CAN, P-Line, Diag-X, Diag-H, SCS, WEN" | [Snap-on DST-i listing][so-dsti] (HIGH) |
| HAC | not listed; "100% HDS and i-HDS compatibility" | [Snap-on HAC listing][so-hac] |

### 2. What the dealer tool offers on a 2001-2005 Civic

#### 2.1 Which control unit is on which DLC line (2002 US Civic DX/LX/EX/HX/GX)

Read directly from Honda's connector view "213. DLC" and splice diagrams F28, F29, F55. **HIGH** for the US 1.7 L
cars; not verified for the Civic Si (EP3), European or Japanese cars.

| DLC pin (SAE) | Honda manual no. | Wire | Honda's description | Connected units |
|---|---|---|---|---|
| 4 | 12 | BLK | "Ground for PGM Tester" (G451) | body ground |
| 5 | 13 | BRN/YEL | "Signal ground for communication network" (circuit Z20) | ECM/PCM E3 (LG3), immobilizer unit pin 1 |
| 7 | 15 | LT BLU | "K line for off-board communication" (circuit F29) | ECM/PCM E23 (K-LINE); ABS modulator-control unit terminal 11 "DIAG-K ... Communications with Honda PGM Tester"; optional CarLink box |
| 9 | 1 | BRN | circuit F28 "Service Check Control" | ECM/PCM E29 (SCS); SRS unit C6; ABS unit terminal 12 "DIAG-L ... DTC indication" |
| 12 | 4 | RED/WHT | "WEN input" | (ECM/PCM; destination terminal not read) |
| 14 | 6 | GRY | circuit F55 "DLC Input/Output" | SRS unit C2 "DLC input/output"; immobilizer control unit-receiver pin 3; optional CarLink box |
| 16 | 8 | WHT/RED | "Power for PGM Tester (B+, 4 amps max)" (fuse 9) | battery |
| 1, 2, 3, 6, 8, 10, 11, 13, 15 | | | not populated | |

Sources: [DLC connector view][img-dlc], [circuit F28][img-f28], [circuit F29][img-f29], [circuit F55][img-f55],
[ABS unit terminal table][img-abs], [SRS unit connector view][img-srs], [immobilizer circuit diagram][img-immo].

Consequences:

- **HIGH** - No wire runs from the DLC to the multiplex control unit, gauge assembly or keyless receiver. The body
  system is outside the scan tool's reach on this car; the manual's multiplex troubleshooting uses a "special tool"
  on the "multiplex inspection connector" and never mentions the tester. [multiplex troubleshooting][cm-mpx]
- **MEDIUM** - The pin-14 line is what the interface vendors call "Diag-H" / "DIAG-H". The manual never prints that
  name for the 2002 Civic; the identification rests on the vendor line lists, on SAE J2534-2 having a HONDA_DIAGH
  protocol, and on the forum report in section 4.
- **HIGH** - Tester K-line idle level: 8.5 V or more between DLC 5 and DLC 7 with the ignition ON; 0 V with ECM/PCM
  connector E (31P) unplugged. Inference (MEDIUM): the ECM/PCM, not the ABS unit, pulls the line up.
  [DLC circuit troubleshooting][cm-dlc-test]

#### 2.2 System and function list

PGM Tester screens (1996-2000 cars, same tester and menu family as used on a 2001-2002 Civic). **HIGH**,
[Tech-2-Tech module 27][tt27], [module 28][tt28]:

- SYSTEM SELECT: 1 PGM-FI, 2 A/T, 3 SRS, 4 DBW, 5 IMMOBI, 6 ABS/TCS, 7 ATTS.
- TEST MODE MENU (PGM-FI): 1 TROUBLESHOOT, 2 DATA LIST, 3 FREEZE DATA, 4 SNAPSHOT, 5 CLEAR, 6 INSPECTION, 7 SETUP.
- DATA LIST MENU: ALL DATA LIST, CUSTOM LIST, LED LIST, BAR GRAPH, LINE GRAPH. 39 parameters on a 1996 Civic against
  19 in generic OBD-II mode; Honda freeze frame 16 parameters against 9.
- SNAPSHOT MENU: ANY CODE, MANUAL TRIGGER, TRIGGER POINT, EVENT REPLAY; 30 s of data, one sample every 0.2 s.
- Two clearing options: clear DTCs (also clears freeze data and readiness) or ECM reset (also clears adaptive
  learning, "equivalent to removing power").
- "The Honda software uses a much faster protocol" than the generic OBD-II mode; a full Honda data pull refreshes
  about as fast as a single generic parameter.
- Readiness monitors are only in the tester's generic OBD-II mode.

HDS screens (2006 Civic example, same application). **HIGH**, [Import Service 2011][antitheft]:

- All DTC Check groups: PGM-FI confirmed and pending, A/T confirmed and pending, SRS, "HONDA ABS", "BODY ELECTRICAL".
- Immobilizer Menu: "System Check, Number of Keys and Status Log", "Add and Delete Keys", "Replace Immobilizer
  Receiver/Control Unit", "Replace ECM/PCM", "Replace MPCS/MICU/IMOES", "Keyless Transmitter Registration".

Per-system facts for the 2002 US Civic from the service information:

| System | Function | What the manual says | Confidence, source |
|---|---|---|---|
| PGM-FI | Read DTC, freeze data, current data | "connect the Honda PGM Tester or an OBD II scan tool to the DLC"; they "can read the DTC, freeze frame data, current data, and other ECM/PCM data" | HIGH, [PGM-FI scan tool page][cm-pgmfi-use] |
| PGM-FI | Read codes without a tool | "Manufacturer does not supply information for Reading Codes without a scan tool" | HIGH, [page][cm-pgmfi-noscan] |
| PGM-FI | Reset ECM/PCM | OBD II scan tool or PGM Tester, or remove the No. 6 ECU (ECM/PCM) 15 A fuse for 10 s; then idle learn | HIGH, [page][cm-pgmfi-clear] |
| PGM-FI | Actuator command | "Turn the EVAP bypass solenoid valve ON with the HDS" (2005 P1456 chart; alternative: ground ECM/PCM E20) | HIGH, [2005 P1456][cm-05-p1456] |
| PGM-FI | ECM/PCM software update | Special tool HIM EQS05A35570 on the DLC, ignition ON (II), never OFF during the update, battery fully charged, no electrical loads | HIGH, [Updating the ECM/PCM][cm-ecm-update] |
| PGM-FI | VIN write | 2004-2005 only: "Input the VIN into the ECM/PCM with the HDS"; P0630 "VIN Not Programmed or Mismatch" otherwise | HIGH, [2005 update page][cm-05-update] |
| PGM-FI | Relearns | Idle learn only (no CKP pattern clear/learn in the 2002 or 2005 1.7 L tree) | HIGH, [2005 ECM index][cm-05-ecm] |
| A/T (in PCM) | DTC | HDS: "select SCS mode from the SELECT MODE MENU", D indicator blinks; flash DTC 70 = an A/T DTC exists | HIGH, [A/T SCS page][cm-at-scs] |
| A/T | Reset | Scan tool / PGM Tester or No. 6 fuse for more than 10 s | HIGH, [PCM reset][cm-at-reset] |
| CVT (HX) | Start clutch calibration | Driving procedure only, no tool | HIGH, [page][cm-cvt] |
| ABS | Read DTC | PGM Tester on the 16P DLC, "follow the prompts"; or "Short the SCS circuit to body ground using the Honda PGM Tester" and count ABS indicator blinks. Three DTCs kept in EEPROM. DTC numbers 11-18, 21, 31-38, 51-54, 61, 81 | HIGH, [ABS with PGM Tester][cm-abs-pgm], [ABS behaviour][cm-abs-noscan] |
| ABS | Clear DTC | PGM Tester prompts, or ignition ON/OFF at least 20 times | HIGH, [page][cm-abs-clear] |
| ABS | Generic scan tool | "Honda does not provide instructions for retrieving ABS DTCs with a generic scan tool" | HIGH, [page][cm-abs-generic] |
| SRS | Read DTC | HDS direct; PGM Tester "SRS" menu direct; PGM Tester/HDS "SCS" menu = flash codes on the SRS indicator | HIGH, [SRS read][cm-srs-read] |
| SRS | Erase DTC | PGM Tester ("Honda PGM Tester SRS Vehicle System Supplement") or SCS service connector 07PAZ-0010100 on the 2P MES connector with a timed sequence; "Do not use a jumper wire" | HIGH, [SRS erase][cm-srs-clear] |
| SRS | OPDS initialization | HDS Main Menu > SRS > Misc Test > adjustments > Adjustment Menu > OPDS INIT | HIGH, [page][cm-srs-opds] |
| Immobilizer | Diagnosis | No DTC list; indicator behaviour (2 s on = normal, blinking = fault) and wiring checks | HIGH, [immobilizer test][cm-immo-test] |
| Immobilizer | Unit replacement | "rewrite the unit with a Honda PGM Tester/HDS" | HIGH, [same][cm-immo-test] |
| Immobilizer | ECM/PCM replacement | PGM Tester: REPLACE ECM/PCM, VIN and odometer entry, password prompt, registration. HDS: IMMOBI > Immobilizer Setup > Replace ECM/PCM, with the "4-digit ECM/PCM code from the iN Immobilizer Code Inquiry" when not logged in | HIGH, [SB 00-098 2001][sb00098a], [SB 00-098 rev. 2014][sb00098b] |
| EPS (Civic Si) | Read DTC | HDS direct, or "Short the SCS circuit to body ground using the HDS" and count blinks (wheels straight, engine stopped, SCS shorted after ignition ON) | HIGH, [Si steering page][cm-si-eps] |
| Multiplex (body) | Self-test | Special tool on the multiplex inspection connector; Mode 1 = fault blinks on spotlight and ceiling light, Mode 2 = input test | HIGH, [page][cm-mpx] |
| Keyless | Transmitter programming | On-board ignition and lock-button sequence, three transmitters, no tester | HIGH, [page][cm-keyless] |
| HVAC | DTC | The 2005 manual has HVAC DTC charts; the retrieval method was not fetched | LOW, [2005 DTC index][cm-05-dtc] |

Immobilizer architecture, because it decides what "key registration" touches. **HIGH**,
[immobilizer description][cm-immo-desc], [circuit diagram][img-immo]:

- The immobilizer control unit-receiver (7P connector) talks to the ECM/PCM on a dedicated line: unit pin 2 RED/BLU
  to ECM/PCM E27 "IMOCD". The ECM/PCM powers the transponder through the unit, gets the code back, and grounds
  PGM-FI main relay 2 (E1 "IMOFLR") when the code matches.
- Unit pin 3 GRY goes to the DLC (pin 14). So the tester reaches the immobilizer unit on pin 14 and the ECM/PCM on
  pin 7; the IMMOBI functions very likely use both.
- Other unit pins: 1 BRN/YEL logic ground (E3 LG3), 4 GRN/ORN parking-brake / brake-fluid switch, 5 BLU/ORN
  indicator, 6 YEL/BLK ignition feed, 7 WHT/RED battery.

ECM/PCM reflashing evidence:

- **HIGH** - Bulletin 02-011 "ECM Software Update" (original 26 Feb 2002): update the 2002 Civic ECM/PCM "with the
  HDS", software from the "January 2002 or later" HondaNet CD. Bulletin 03-002 (2001-02 Civic DX, LX, EX, P0128):
  update to "January 2003 or later software". Both refer to bulletin 01-023. [SB 02-011][cm-sb02011],
  [SB 03-002][cm-sb03002]
- **HIGH** - Bulletin 01-023 (version 12, 10 June 2016): "Most 2002 and later Honda vehicles have reprogrammable
  control modules"; updating now needs the DST-i or Bosch MVCI plus i-HDS, whose "J2534 Rewrite" icon does the
  update; "The CM Update (rewrite) feature within the MVCI menu is removed". Screens show "Honda J2534 Pass Thru ECU
  Reprogramming", "Calibration Files: 1728", "CM Update Mode", "Current Program ID", "Program Part No" (example
  37805-58R-U55), system selection "PGM-FI", "A/T (CVT)". Battery must not drop below 10 V; on failure keep the
  ignition ON. [SB 01-023][sb01023]
- **MEDIUM** - Honda's J2534 page carries a legacy table ("X = Reprogrammable") with a Civic row covering the early
  2000s; the search summary could not align the year columns. [Honda J2534 page][ti-j2534]
- **MEDIUM** - Honda reflash uses a +5 V programming voltage on DLC pin 12: an independent J2534 vendor documents
  "Set programming voltage +5V to 12 pin of OBD2 socket (Honda reflash)" and "12 - Boot Pin for programming Honda
  ECU (only +5V can be set)". That matches the DLC view's "WEN input" on pin 12 and the "WEN" entry in the DST-i
  line list. [Chipsoft J2534 Lite datasheet][chipsoft], [DLC connector view][img-dlc]

Market differences found in this dimension:

- **HIGH** - Bulletin 00-098 lists separate ECM/PCM part numbers per market code (KA, KL, KK, KX, KY/KV, KU), all
  serviced with the same PGM Tester procedure. [SB 00-098 2001][sb00098a]
- **HIGH** - The US Civic Si (EP3, K20A3) adds EPS as a tester system. [Si steering page][cm-si-eps]
- Not found: system lists for European (D14Z6, D16V1, D16W7, 1.7 CTDi, K20A2) and Japanese cars. See open questions.

### 3. Legitimate access today

United States:

- **MEDIUM** - Sold through store.honda.com as the "i-HDS subscription" (dealer-level diagnostics and ECU
  reprogramming). A separate "i-HDS Vehicle Security Professional (VSP)" subscription is required for immobilizer
  functions; key and immobilizer codes are sold separately, US vehicles only, to members of the NASTF Vehicle
  Security Professional Registry. [Honda store][store] (via search summary)
- **HIGH** for the 1-day price, **LOW** for the others - A buyer paid "60dollars" for one day in August 2026 and the
  "HDM" (Honda Download Manager) then downloaded the latest i-HDS; the store summary also shows USD 60 for one day.
  Other tiers conflict between sources (see Conflicts). [civicx thread][civicx], [Honda store][store]
- **MEDIUM** - Windows requirement moved from Windows 7/8/10 (2016-2020 documents) to Windows 11 Professional or
  Enterprise (current store and HAC listing). [cnautotool mirror of Honda's J2534 page][cnauto],
  [Snap-on HAC listing][so-hac]
- **MEDIUM** - Validated interfaces as Honda lists them: Denso DST-i (firmware 2.00.0010, J2534 API 04.04) and Bosch
  MVCI (firmware 3.01.54, API 04.04) validated by Honda; Drew Technologies MongoosePro Honda (version 1.7.7) and
  Dearborn Group d-briDGe Pro (3.303) "independently validated"; any generic VCI "compliant with SAE J2534-1 and
  J2534-2" is accepted without support. Today Honda recommends the HAC. [cnautotool mirror][cnauto],
  [Honda J2534 page][ti-j2534]
- **MEDIUM** - A third-party pass-thru vendor publishes Honda / i-HDS / J2534 as "Supported Year for Diagnostics
  1996 - present" and "Supported Year for Programming 2001 - present". [TopDon RLink Lite list][topdon]
- **MEDIUM** - "The J2534 software will reprogram powertrain ECUs only, per regulation", and "not all legacy
  Honda/Acura models are equipped with a reprogrammable ECM/PCM". [Honda J2534 page][ti-j2534]

Does the J2534 route reach a pre-CAN car's Honda-specific lines?

- **HIGH** - The standards exist for exactly that: SAE J2534-2 (2010-10) "Added support for Honda Diag-H Protocol";
  SAE J2534-2/4_0500 (January 2022) is "Pass-Thru Extended Feature - HONDA_DIAGH"; SAE J2809 (October 2007,
  stabilized October 2017) defines "the proprietary diagnostic communication protocol for ABS or VSA ECU" on some
  Honda vehicles and "should be used in conjunction with SAE J2534-2 ... in an enhanced SAE J2534 interface".
  [SAE J2534-2 2010][sae-j2534-2], [SAE J2534-2/4][sae-diagh], [SAE J2809][sae-j2809]
- **MEDIUM** - So HDS diagnostics of the SRS and immobilizer units through J2534 needs an interface that implements
  the optional HONDA_DIAGH protocol. The Honda-branded and Honda-validated devices do; an arbitrary J2534-1 cable
  does not. This is inference from the standards plus the clone behaviour in section 4, not a Honda statement.

Europe:

- **HIGH** - `techinfo.honda-eu.com`: HDS hardware is a "Windows tablet (Getac A140) and VCI (Denso DST-010)" sold
  through Snap-on; "There is an annual fee for access to download a single machine licence HDS software for 1 year",
  bought together with a MaRIS Pay & Go subscription; ECU updates use "Honda J2534 Pass-thru software and a suitable
  Vehicle Communication interface"; security functions (key coding, ECU management) need SERMI approval through a
  Trust Centre. No prices on the page. [Honda EU RMI page][eu-rmi]
- **LOW** - A UK user paid "30 euros" for an EU portal account, found no i-HDS download there, and was pointed to
  Snap-on. [civicx thread][civicx]

Japan: nothing found in this round.

### 4. Clone ecosystem (facts for capture planning only; nothing was downloaded)

| Clone | What is inside / how it connects | Software it is sold with | What it reaches | Price seen | Confidence, source |
|---|---|---|---|---|---|
| "HIM" clone | Two PCBs marked 3557-1235-00 and 3557-1025-00, Rev A, 2006-03; "double-board with OBD II cable" plus RS-232 cable, needs a real COM port (COM1) | HDS 3.015.033 worked in 2015; vendors advertise 3.101.x / 3.103.x "no need activation" | Advertised "1992-2016", all systems incl. immobilizer; key programming still asks for a password | about USD 100 (2015) | MEDIUM, [blog teardown][blog-mk9], [clone vendor blog][blog-hds] |
| "GNA600" clone | FT232R USB-UART + NXP LPC2148; described as a copy of the Drew Tech Mongoose | HDS 3.015.033 as a J2534 device | On the author's 9th-generation (CAN) Civic: most modules, e.g. PGM-FI, AT, SRS, "not all". No pre-CAN test reported | about USD 30 (2015) | MEDIUM, [blog teardown][blog-mk9] |
| Cheap "HDS cable" / Xhorse-type MVCI | USB cable with built-in USB-serial; Xhorse MVCI Pro is selected in HDS as "XH-HONDA" after installing a Honda driver from the Xhorse tool | HDS + i-HDS suite on Windows 7 | On a pre-CAN car only AT and PGM-FI appeared; bridging DLC pin 7 to pin 14 made ABS, IMMO and SRS appear and allowed a key to be programmed on a 2005 Fit | not captured | LOW, [Honda-Tech thread][ht-mvci] (search summary); MEDIUM for setup, [Xhorse MVCI Pro review][vvdi] |

- **LOW** - The pin 7 / pin 14 bridge report is the only field evidence in this round about the electrical nature of
  the pin-14 line: a K-line transceiver can talk on it, so it is a 12 V single-wire serial line of the same family.
  It agrees with the factory wiring (SRS and immobilizer on pin 14). Baud rate, initialisation and framing remain
  unknown.
- Legal position, briefly (**not verified in this round, not legal advice**): HDS / i-HDS is licensed by subscription;
  the copies bundled with clones are unlicensed, and the interfaces copy Drew Technologies / Teradyne designs. Use
  them as described facts only. Analysing lawfully obtained software for interoperability has statutory room in the
  EU (Software Directive, decompilation for interoperability) and the US Copyright Office has renewed a DMCA
  exemption for vehicle software diagnosis, repair and modification; both should be checked before the REA phase
  starts on any Honda binary.

### 5. HDS software internals (thin; search budget exhausted before this question)

Known from public sources:

- **HIGH** - Components installed by the Honda/HDS Download Manager in 2016: "Honda Download Manager", "i-HDS
  Software", "Database Update" (dated, e.g. 12-MAY-2016), "EPS Rewrite", "J2534 Rewrite Special ...". The J2534
  Rewrite application reports "PC Application version", "Database Update", "Calibration Files: 1728" and "NoticeDB
  version". Entering a service bulletin number launches a "J2534 Rewrite Special Application". [SB 01-023][sb01023]
- **HIGH** - i-HDS HOME menu: Connect to the Vehicle, Stored Data Playback, Settings, Related applications,
  Immobilizer Setup Tool, J2534 Rewrite. [SB 01-023][sb01023]
- **MEDIUM** - Install order seen by a user: HDS from the root of the suite, then i-HDS from a sub-folder, "because
  HDS is launched from within iHDS". [Xhorse MVCI Pro review][vvdi]
- **MEDIUM** - Calibration files live in "the installation folder of 'J2534 Rewrite software'" as `.rwd` / `.gz`
  files that are "only partial and encrypted" (no bootloader). A public repository names the folder `CalibFiles`
  and maps file-name prefixes to modules: 37805 = ECM, 37806 and 28101 = TCM, 39990 = EPS, 57114 = VSA, 77959 = SRS,
  78109 = odometer, 38897 = gateway. The open-source reflash tool built on them is CAN-only (2006+); nothing public
  was found for the K-line generation's file format. [HondaReflashTool README][hrt], [HondaCalibFiles][hcf]
- **HIGH** - HDS help documents carry identifiers such as "4R_IMM3_APP_CHECK_01_05" (immobilizer system-check help),
  i.e. guided-diagnosis content is stored as named documents. [Import Service 2011][antitheft]
- **MEDIUM** - Update files are named by program part number (37805-xxx-xxxx) while the control unit hardware is
  37820-xxx-xxxx (e.g. 37820-PLM-A52, 37820-PLR-L54). [SB 01-023][sb01023], [SB 00-098 rev. 2014][sb00098b]

Not found: install paths, runtime/language, database format, where the per-vehicle system and parameter tables
live, any published protocol extraction for the pre-CAN systems.

Targets for the REA toolkit, in order of value:

1. The J2534 API boundary between HDS and the interface DLL: every `PassThruConnect` (protocol ID, flags, baud),
   `PassThruIoctl` (pin selection, `SET_CONFIG`, programming voltage on pin 12, initialisation) and message. This
   yields the exact pre-CAN protocol parameters without electrical guessing.
2. The HDS vehicle/system database of a legitimately licensed install: system list per model, data-list parameter
   definitions and scaling, DTC text, function-test and adjustment definitions, immobilizer flows.
3. The J2534 Rewrite calibration store and its index ("Database Update", "NoticeDB"): check whether 37805-PLM / PLR /
   PMM / PNF-family files for 2001-2005 Civics are present and how the K-line-era files are packaged.
4. Captures on DLC pins 7, 14, 9 and 12 (logic analyser or TShark-readable serial captures) taken while HDS runs
   each function, to cross-check item 1.
5. A PGM Tester program card image, if one is acquired legally: it contains the whole 1992-2002 protocol set
   (PGM-FI, A/T, SRS, IMMOBI, ABS/TCS) in a 4 MB linear-flash card.
6. The interface firmware (DST-i 2.00.0010, MVCI 3.01.54) is a last resort; it only matters if the HONDA_DIAGH
   physical layer cannot be learned from SAE J2534-2 and captures.

### 6. Aftermarket coverage as a yardstick

| Tool | What is documented for this generation | Confidence, source |
|---|---|---|
| Any OBD-II scan tool / ELM327 | Engine and (PCM) transmission P-codes, freeze frame, readiness, generic data over the pin-7 K-line; the manual treats "an OBD II scan tool" as equivalent to the PGM Tester for DTC, freeze data and ECM reset | HIGH, [PGM-FI page][cm-pgmfi-use] |
| Vetronix Mastertech with the aftermarket Honda/Acura card (2000) | Only PGM-FI, A/T, DBW (no SRS, IMMOBI, ABS); software locks stopped PGM Tester cards from running in a Mastertech and the reverse | HIGH, [Tech-2-Tech module 27][tt27] |
| HonDash (Android app + own scanner) | Scanner only for 1992-2001 cars with the 3-pin or 5-pin DLC; 16-pin cars "require a ELM327 scanner"; "can read and clear diagnostic trouble codes from the engine control unit (ECU) only"; "ELM327 firmware does not support the proprietary Honda OBD1 protocol" | HIGH, [HonDash FAQ][hondash-faq], [compatibility][hondash-compat] |
| OBDLink MX+ with the Honda/Acura enhanced add-on | Add-on exists and is exclusive to the MX+; model-year and system coverage for a 2002 Civic not verified | LOW, [OBDLink article][obdlink] |
| Unbranded Chinese hand-held, "HONDA_V01.20 Diag Function List" (2019) | Lists "CIVIC - 2004" and marks ECU info, read codes, erase codes, live data as supported for ECM, AT, ABS, SRS, BCM, EPS, IMMOBILIZER and about 30 more systems in one blanket table, with no per-model detail | LOW, [function list PDF][fnlist] |
| Foxwell NT510 Elite (Honda) | Marketing: engine, ABS, SRS, transmission, BCM, IMMO, service resets; no model-year table read | LOW, [Foxwell product page][foxwell] (search summary) |
| TopDon RLink (J2534) + Honda's own software | Pass-thru only: i-HDS diagnostics 1996-present, programming 2001-present | MEDIUM, [TopDon list][topdon] |
| Autel, Launch, Snap-on | Not researched: search budget exhausted. Their public coverage look-ups are interactive and need a manual query for "2002 Civic" | none |

Honda's own position, for calibration: "Honda does not provide instructions for retrieving ABS DTCs with a generic
scan tool". [page][cm-abs-generic] (HIGH)

### 7. Vendor protocol naming

| Name as printed | Where | What it tells us | Confidence |
|---|---|---|---|
| "K line for off-board communication" | Honda DLC connector view, pin 7 | Honda's own name for the pin-7 line | HIGH, [DLC view][img-dlc] |
| "DIAG-K (Diagnosis K) ... Communications with Honda PGM Tester" | ABS modulator-control unit terminal 11 | The ABS unit's tester line is the K-line | HIGH, [ABS table][img-abs] |
| "DIAG-L (Diagnosis L) ... DTC indication" | ABS unit terminal 12 (BRN = SCS circuit) | Honda's unit-side name for the SCS line | HIGH, [ABS table][img-abs] |
| "DLC input/output" (circuit F55, GRY) | SRS unit C2, immobilizer unit pin 3, DLC pin 14 | The second tester line; no protocol name printed | HIGH, [circuit F55][img-f55] |
| "WEN input" | DLC pin 12 | Honda prints only "WEN input" (HIGH). Reading it as the write-enable line for reprogramming rests on the pin-12 reflash evidence in section 2.2 (MEDIUM) | [DLC view][img-dlc] |
| "K-Line, Honda Diag H, Honda Diag X" | Bosch MVCI listing | Tool-side names | HIGH, [Snap-on MVCI][so-mvci] |
| "ISO9141, CAN, P-Line, Diag-X, Diag-H, SCS, WEN" | Denso DST-i listing | Tool-side names; SCS and WEN are driven lines, not protocols | HIGH, [Snap-on DST-i][so-dsti] |
| `HONDA_DIAGH_PS = 0x0000800B`, `HONDA_DIAGH_CH1 = 0x00009680` | J2534-2 protocol IDs in an open-source J2534 library | The pass-thru protocol ID an HDS-compatible device must answer to | MEDIUM, [J2534-Sharp Protocol.cs][j2534sharp] |
| "Honda Diag-H Protocol" | SAE J2534-2 (2010-10) change list; SAE J2534-2/4_0500 "HONDA_DIAGH" | Normative definition (paywalled) | HIGH that it exists, [SAE J2534-2][sae-j2534-2], [SAE J2534-2/4][sae-diagh] |
| "Honda Diagnostic Serial Data Link Protocol - ABS/VSA System" | SAE J2809 | Normative definition of the ABS/VSA dialogue (paywalled) | HIGH that it exists, [SAE J2809][sae-j2809] |
| "92Hm/2", "95Hm-CARB", "H-Line" | brief's unverified leads | **Not found** in any source reached. The Mongoose Pro Honda pages reached only say "complete SAE J2534-1 and J2534-2 capabilities" and "support for HDS diagnostics and Honda J2534 reprogramming software" | unverified, [MongoosePro Honda description][vsp-mongoose] |

No source in this round gave a baud rate, initialisation sequence or frame format for the pin-14 line, or for the
Honda-specific (non-OBD-II) dialogue on the K-line.

### 8. Function matrix: control unit x function x who can do it

Scope: 2002 US Civic 1.7 L unless stated. "Dealer" = PGM Tester and/or HDS. "Aftermarket" lists only what a source
supports; "unverified" means no evidence either way in this round.

| Control unit (DLC line) | Function | Dealer tool | Aftermarket | No-tool / blink-code alternative | Sources |
|---|---|---|---|---|---|
| ECM/PCM PGM-FI (pin 7) | Read DTC | Yes | Yes, any OBD-II tool (generic) | SCS mode makes the MIL flash codes (manual gives no procedure for PGM-FI; the A/T page records "fuel and emissions DTCs" in SCS mode) | [1][cm-pgmfi-use], [2][cm-pgmfi-noscan], [3][cm-at-scs] |
| | Freeze data | Yes, wider set | Yes, generic set | none | [tt27][tt27] |
| | Clear DTC / reset ECM | Yes, two levels | Yes (generic clear) | Pull No. 6 ECU 15 A fuse for 10 s | [cm][cm-pgmfi-clear] |
| | Data list, snapshot | Yes (Honda list, 0.2 s snapshot) | Generic PIDs only; Honda list via Mastertech card; others unverified | none | [tt27][tt27], [tt28][tt28] |
| | Readiness monitors | Generic mode | Yes | none | [tt27][tt27] |
| | Actuator / function tests | Yes (e.g. EVAP bypass solenoid ON) | unverified | Ground the ECM/PCM terminal by jumper (E20 for that valve) | [cm][cm-05-p1456] |
| | Idle learn | Reset by tool, learn by procedure | Reset by tool | Warm up, idle 10 minutes | [sb][sb00098b] |
| | VIN write (2004-2005) | Yes, HDS | unverified | none | [cm][cm-05-update] |
| | Software update (reflash) | Yes: HIM, HDS CM Update, now J2534 Rewrite | Only as pass-thru for Honda's software; independent K-line reflash unverified | none | [cm][cm-ecm-update], [sb][sb01023] |
| PCM A/T (pin 7) | Read / clear DTC | Yes | Generic P07xx codes | SCS mode, D indicator blinks | [cm][cm-at-scs] |
| | CVT start clutch calibration | Not needed | Not needed | Driving procedure | [cm][cm-cvt] |
| ABS (pin 7) | Read DTC | Yes | unverified (Honda gives no generic procedure) | SCS mode, ABS indicator blinks | [cm][cm-abs-pgm], [cm][cm-abs-generic] |
| | Clear DTC | Yes | unverified | Ignition ON/OFF at least 20 times | [cm][cm-abs-clear] |
| | Data list / tests | Not stated for this car | unverified | none | |
| SRS (pin 14) | Read DTC | Yes (direct and SCS) | unverified; impossible for K-line-only adapters | SCS mode, SRS indicator blinks | [cm][cm-srs-read] |
| | Erase DTC | Yes | unverified | MES 2P connector + SCS service connector timed sequence | [cm][cm-srs-clear] |
| | OPDS initialization | Yes, HDS | unverified | none | [cm][cm-srs-opds] |
| Immobilizer unit (pin 14) + ECM/PCM (pin 7) | System check / status | Yes (HDS immobilizer menu) | unverified | Indicator behaviour only | [is][antitheft], [cm][cm-immo-test] |
| | Add / delete keys | Yes, needs password or code | Key-programmer class tools, unverified | none | [is][antitheft], [blog][blog-hds] |
| | Replace ECM/PCM (code rewrite) | Yes, needs password / 4-digit code | unverified | none | [sb][sb00098a], [sb][sb00098b] |
| | Replace immobilizer unit | Yes | unverified | none | [cm][cm-immo-test] |
| EPS (Civic Si; line not verified) | Read DTC | Yes, HDS | unverified | SCS mode, indicator blinks | [cm][cm-si-eps] |
| Multiplex / body (not on DLC) | Fault codes, input test | No | No | Mode 1 / Mode 2 with the multiplex inspection connector tool | [cm][cm-mpx] |
| Keyless receiver (not on DLC) | Transmitter registration | No | No | On-board sequence | [cm][cm-keyless] |
| HVAC | DTC | not checked | not checked | Self-diagnosis exists (2005 charts) | [cm][cm-05-dtc] |

## Conflicts and uncertainties

- **PGM Tester end of coverage**: "only offers coverage until 2003" ([AutoInc][autoinc]) against "1994 - 2002 models"
  (a vendor listing in a search summary). Either way a 2002 car is covered; 2003-2005 cars are HDS territory.
- **Which tool the 2002 bulletins named**: the mirrored 2011 revisions of bulletins 02-011 and 03-002 say "HDS", the
  2002 manual page says HIM. The originals may have named the HIM/PGM Tester; the mirror shows only the revision.
- **i-HDS prices**: store summaries give USD 60 / 275 / 1,600 in a scrambled order; older Honda pages give USD 45 /
  200 / 1,800 and USD 10 / 133 / 1,547 for 1 / 30 / 365 days. Only "1 day = USD 60" has a second source
  ([civicx][civicx]). Check the live store before budgeting.
- **Generic J2534 devices**: an older Honda FAQ says generic J2534 devices "are for re-programming only" and that
  i-HDS "will only work with a PC and a Honda scan tool, such as the DST-i"; the newer J2534 page says i-HDS
  "supports the use of a generic pass-thru VCI". Field report: a TopDon RLink worked with HDS on a 2017 (CAN) car.
  Nothing confirms a generic device working on the pre-CAN pin-14 line. [Honda FAQ][ti-faq], [Honda J2534 page][ti-j2534]
- **ABS on which line**: the factory diagrams put the 2002 Civic ABS unit on the K-line (pin 7). The Honda-Tech
  report says a K-line-only cable saw only AT and PGM-FI and needed the 7-14 bridge for "ABS/IMMO/SRS", but that
  test car was a 2005 Fit, not a Civic. Either the K-line-only cable failed on ABS for a protocol reason, or the
  Fit's ABS is wired differently. Treat ABS-over-K on the Civic as wiring fact, protocol unknown.
- **7th-generation "MICU gateway"**: a Honda-Tech post describes a proprietary K-line to the MICU acting as a
  gateway. The 2002 US wiring shows no DLC wire to the multiplex unit, so that description does not fit the US 1.7 L
  cars (it may describe another market or a later car). LOW.
- **Fuse rating**: the DLC troubleshooting text calls the DLC feed "No. 9 BACK UP (7.5A)"; the immobilizer page calls
  fuse No. 9 "10 A". Check the fuse box label.
- **Part number typo**: the mirrored bulletin 02-011 prints the failed part as "37520-PLR-L54"; bulletin 03-002
  prints "37820-PLR-L54". 37820 is the ECM/PCM prefix.
- **"Diag-X" and "P-Line"**: listed on the MVCI and DST-i, never explained in any source reached. Probably not used on
  this car, but that is an assumption.
- **Operation CHARM** is a mirror of an aftermarket repair database built from Honda service information. Its
  diagrams are Honda scans; its text has been re-edited (later revisions mention HDS inside 2002 pages).

## Open questions

| Question | Why it matters | How to answer |
|---|---|---|
| Physical layer, baud rate, initialisation and framing on DLC pin 14 (SRS, immobilizer) | Second transceiver channel design; without it no SRS or immobilizer access | Buy SAE J2534-2 (or J2534-2/4_0500); capture pin 14 with a logic analyser while HDS reads SRS; J2534 API log |
| Dialogue used on the K-line for ABS and for Honda-specific PGM-FI data | ABS codes, Honda data list, function tests | Buy SAE J2809; J2534 API log of an HDS session; compare with the other researchers' K-line findings |
| Is the pin-14 line electrically identical to a K-line | Whether one transceiver plus an analogue switch is enough | Scope pin 14 idle level and edges on the car; the 7-14 bridge report suggests yes |
| Where pin 12 (WEN) goes in the ECM/PCM and what voltage/timing HDS applies | Reflash entry | Read the ECM/PCM connector E pin table in the manual; measure pin 12 during a J2534 Rewrite session |
| Does today's J2534 Rewrite database still contain calibrations for 2001-2005 Civic program IDs | Whether a legitimate reference flash exists to capture | Run J2534 Rewrite on the car with a day pass; inspect the calibration store of a licensed install |
| HDS install layout, database formats, per-vehicle tables | Points the REA toolkit | Inspect a licensed install; Honda's i-HDS Quick Start Guide and "CHE HDS Software Installation Instructions" on techinfo.honda.com (fetch them in a normal browser) |
| Exact system and function tree HDS shows for a 2001-2005 Civic (data-list parameter names, Inspection and Adjustment menus for PGM-FI, ABS, SRS) | Defines "every possible code" precisely | Screenshots from a day-pass session; Honda PGM Tester user guides and the "SRS Vehicle System Supplement" |
| How the immobilizer password / 4-digit ECM/PCM code is derived and checked | Key and ECM registration without Honda | Out of scope for the web; needs the immobilizer researcher's EEPROM work and a capture of a registration |
| Last HDS version that drives a genuine HIM or GNA600 | Only if legacy hardware is used for capture | Honda Special Tools hotline; legacy tools page read in a browser |
| European and Japanese 7th-generation system lists (EPS, VSA on Type R, 1.7 CTDi diesel ECU, immobilizer type) and DLC pinouts | The car's market is unknown | European service manual DLC view and splice diagrams; HDS with Market set to Europe / Japan |
| EP3 (Civic Si / Type R) DLC line for EPS and ABS | Hardware coverage for K-series cars | Same Operation CHARM pages for "2002 Civic Si" (connector view "DLC", EPS control unit table) |
| Autel / Launch / Snap-on / Foxwell coverage for a 2002 Civic by system | Independent yardstick, and a cheap capture source | Vendor coverage look-ups queried by hand; a bench session with one of these tools on the car |
| What "Diag-X" and "P-Line" are | Rule them out for this car | J2534-2 text; DST-i user guide on techinfo.honda.com |
| Do "92Hm/2" and "95Hm-CARB" exist as protocol names | Brief's lead | Drew Technologies / Opus IVS CarDAQ and Mongoose datasheets read in a browser |

## Sources

Honda primary documents:

- [SB 00-098 (Feb 2001), Export Service Bulletin][sb00098a] - 2001 Civic PCM replacement with the PGM Tester, part
  numbers per market code.
- [SB 00-098 revision of 5 June 2014][sb00098b] - same job with HDS + MVCI, IMMOBI menu path, iN code.
- [SB 01-023 version 12, 10 June 2016][sb01023] - Updating Control Units/Modules, i-HDS and J2534 Rewrite screens.
- [Honda legacy diagnostic tools page][ti-legacy], [GNA600 page][ti-gna600], [J2534 page][ti-j2534],
  [FAQ][ti-faq], [Honda Canada control module update page][ti-che-update] - techinfo.honda.com, read through search
  summaries only.
- [Honda store, i-HDS subscriptions][store] - read through search summaries only (site returned HTTP 503).
- [Honda Europe RMI page][eu-rmi] - HDS purchase route in Europe, SERMI.

2002 / 2005 Civic service information (Operation CHARM mirror):

- [How to use the PGM Tester or a scan tool][cm-pgmfi-use], [reading codes without a scan tool][cm-pgmfi-noscan],
  [clearing codes / ECM reset][cm-pgmfi-clear].
- [DLC circuit troubleshooting][cm-dlc-test], [DLC connector view image][img-dlc], [circuit F28 image][img-f28],
  [circuit F29 image][img-f29], [circuit F55 image][img-f55].
- [ABS with PGM Tester][cm-abs-pgm], [ABS indicator and DTC behaviour][cm-abs-noscan],
  [ABS generic scan tool note][cm-abs-generic], [ABS DTC clearing][cm-abs-clear], [ABS unit terminal table image][img-abs].
- [SRS DTC reading][cm-srs-read], [SRS DTC erasing][cm-srs-clear], [OPDS initialization][cm-srs-opds],
  [SRS unit connector image][img-srs].
- [Immobilizer troubleshooting][cm-immo-test], [immobilizer description][cm-immo-desc],
  [immobilizer circuit diagram image][img-immo].
- [A/T flash codes in SCS mode][cm-at-scs], [PCM reset][cm-at-reset], [CVT start clutch calibration][cm-cvt].
- [Multiplex control system troubleshooting][cm-mpx], [keyless transmitter programming][cm-keyless].
- [Updating the ECM/PCM (2002)][cm-ecm-update], [SB 02-011][cm-sb02011], [SB 03-002][cm-sb03002].
- [2002 Civic Si steering DTC reading][cm-si-eps].
- [2005 ECM index][cm-05-ecm], [2005 Updating the ECM/PCM][cm-05-update], [2005 P1456 chart][cm-05-p1456],
  [2005 DTC index][cm-05-dtc].

Tool catalogues, standards, datasheets:

- [Snap-on / Honda tool programme: Bosch MVCI][so-mvci], [Denso DST-i][so-dsti], [Honda Antares Capsule][so-hac].
- [SAE J2534-2 (2010-10)][sae-j2534-2], [SAE J2534-2/4_0500 HONDA_DIAGH][sae-diagh], [SAE J2809][sae-j2809] -
  abstracts only.
- [J2534-Sharp Protocol.cs][j2534sharp] - J2534-2 protocol ID constants.
- [Chipsoft J2534 Lite datasheet][chipsoft] - pin 12 programming voltage for Honda.
- [TopDon RLink Lite OEM software list][topdon], [MongoosePro Honda description][vsp-mongoose],
  [cnautotool mirror of Honda's J2534 page text][cnauto].

Training and trade press:

- [Tech-2-Tech module 27 "Scan Tool"][tt27], [module 28 "Scan Tool - Advanced"][tt28] - Grant Swaim, October 2000.
- [AutoInc scan tool review, 1 May 2014][autoinc].
- [Import Service "Antitheft Gone Wild", 31 July 2011][antitheft].

Community and vendor pages:

- [civicx thread on buying i-HDS in the UK/EU, August 2026][civicx].
- [Blog teardown of HIM and GNA600 clones, July 2015][blog-mk9]; [clone vendor blog, 2017][blog-hds].
- [Honda-Tech "Honda HDS and cheap MVCI cables"][ht-mvci] - search summary only.
- [Xhorse MVCI Pro + HDS setup review][vvdi].
- [HondaReflashTool README][hrt], [HondaCalibFiles repository][hcf].
- [HonDash FAQ][hondash-faq], [HonDash compatibility][hondash-compat], [OBDLink enhanced add-on article][obdlink],
  [unbranded Honda function list PDF][fnlist], [Foxwell NT510 Elite page][foxwell].
- [pinoutguide Honda OBD-II page][pinoutguide] (Ridgeline-derived, not used for the Civic),
  [rusEFI "Honda SEFMJ protocol" thread][rusefi] (cross-cutting).

[tt27]: https://www.ej9.ru/civic/book/chpt_27.pdf
[tt28]: https://www.ej9.ru/civic/book/chpt_28.pdf
[autoinc]: https://www.autoinc.org/scan-tool-review-toyota-lexus/
[antitheft]: https://automotivetechinfo.com/wp-content/uploads/2011/09/Antitheft-Gone-Wild.pdf
[sb00098a]: http://www.geocities.ws/pfeifferperformance/00-098CivicProductUpdate-PCM.pdf
[sb00098b]: https://static.nhtsa.gov/odi/tsbs/2014/MC-10129307-9999.pdf
[sb01023]: https://www.civicx.com/forum/attachments/june-10-2016-updating-control-units-modules-pdf.31388/
[ti-legacy]: https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_Legacy_ESS.htm
[ti-gna600]: https://techinfo.honda.com/Rjanisis/pubs/web/ToolSaleP.htm
[ti-j2534]: https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_J2534.htm
[ti-faq]: https://techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_FAQ.htm
[ti-che-update]: https://techinfo.honda.com/rjanisis/pubs/web/RJAAI001_CHE_TOOLS_CONTROL_MODULE_UPDATE.htm
[store]: https://store.honda.com/hondaacura-i-hds-and-vsp
[eu-rmi]: https://www.techinfo.honda-eu.com/en/automobile/diagnostics-special-tools-and-rmi-information
[so-mvci]: https://acura.snapon.com/HondaAcura/Catalog/Product/OTCHON3828AF
[so-dsti]: https://honda.snapon.com/HondaAcura/Catalog/Product/DSCWIFIAF
[so-hac]: https://honda.snapon.com/HondaAcura/Catalog/Product/BSEHAC001
[sae-j2534-2]: https://saemobilus.sae.org/content/J2534/2_201010/
[sae-diagh]: https://saemobilus.sae.org/content/j2534-2/4_5_00
[sae-j2809]: https://saemobilus.sae.org/content/J2809_200710
[j2534sharp]: https://raw.githubusercontent.com/BrianHumlicek/J2534-Sharp/master/Src/Definitions/Protocol.cs
[chipsoft]: https://chipsoft.com.ua/download/CHIPSOFT_J2534_Lite_en.pdf
[topdon]: https://web-file.topdon.com/topdon-web/information_download/RLink-Lite-Compatible-OEM-Software-List.pdf
[vsp-mongoose]: https://www.vehicleservicepros.com/12193989
[cnauto]: https://www.cnautotool.com/blog/instructions-for-i-hds-diagnostic-j2534-reprogramming-software/
[civicx]: https://www.civicx.com/forum/threads/anyone-actually-downlaod-hds-i-hds-in-the-uk-eu.110183/
[blog-mk9]: https://honda-civic-hatchback-mk9.blogspot.com/2015/07
[blog-hds]: https://honda-hds.blogspot.com/2017/04/
[ht-mvci]: https://honda-tech.com/forums/engine-management-tuning-124/honda-hds-cheap-mvci-cables-3312290/
[vvdi]: https://www.vvdishop.com/service/xhorse-mvci-pro-honda-hds-win7-setup-review.html
[hrt]: https://raw.githubusercontent.com/bouletmarc/HondaReflashTool/master/README.md
[hcf]: https://github.com/bouletmarc/HondaCalibFiles
[hondash-faq]: https://www.hondash.net/p/faq.html
[hondash-compat]: https://www.hondash.net/2022/06/compatibility.html
[obdlink]: https://support.obdlink.com/support/solutions/articles/43000711154
[fnlist]: https://cdn.shopify.com/s/files/1/0085/7699/1298/files/HONDA_V01.20_Diagnostic_Function_List_FULL.pdf
[foxwell]: https://www.foxwelldiag.com/products/foxwell-nt510-elite-obd-scan-tool
[pinoutguide]: https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml
[rusefi]: https://www.rusefi.com/forum/viewtopic.php?p=47371
[img-dlc]: https://charm.li/images/IMP02Q313/asian140/673344130/
[img-f28]: https://charm.li/images/IMP02Q313/asian140/1070665479/
[img-f29]: https://charm.li/images/IMP02Q313/asian140/1070799004/
[img-f55]: https://charm.li/images/IMP02Q313/asian140/1070803624/
[img-abs]: https://charm.li/images/IMP02Q313/asian140/171497837/
[img-srs]: https://charm.li/images/IMP02Q313/asian140/278422372/
[img-immo]: https://charm.li/images/IMP02Q313/asian140/1244434416/
[cm-pgmfi-use]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Testing%20and%20Inspection/Scan%20Tool%20Testing%20and%20Procedures/How%20to%20Use%20the%20Honda%20PGM%20Tester%20or%20A%20Scan%20Tool/
[cm-pgmfi-noscan]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/Without%20Scan%20Tool/
[cm-pgmfi-clear]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/
[cm-dlc-test]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Data%20Link%20Connector/Testing%20and%20Inspection/
[cm-abs-pgm]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/Description%20of%20On-Board%20Diagnostics/With%20Honda%20PGM%20Tester/
[cm-abs-noscan]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/Description%20of%20On-Board%20Diagnostics/Without%20Scan%20Tool/
[cm-abs-generic]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Initial%20Inspection%20and%20Diagnostic%20Overview/Description%20of%20On-Board%20Diagnostics/With%20Generic%20Scan%20Tool/
[cm-abs-clear]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Brakes%20and%20Traction%20Control/Antilock%20Brakes%20%2F%20Traction%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/With%20Manufacturer%27s%20Scan%20Tool/
[cm-srs-read]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/With%20Manufacturer%27s%20Scan%20Tool/
[cm-srs-clear]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Clearing%20Diagnostic%20Trouble%20Codes/
[cm-srs-opds]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Restraints%20and%20Safety%20Systems/Air%20Bag%20Systems/Testing%20and%20Inspection/Programming%20and%20Relearning/
[cm-immo-test]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Accessories%20and%20Optional%20Equipment/Antitheft%20and%20Alarm%20Systems/Testing%20and%20Inspection/
[cm-immo-desc]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Accessories%20and%20Optional%20Equipment/Antitheft%20and%20Alarm%20Systems/Description%20and%20Operation/Immobilizer%20System/
[cm-at-scs]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Transmission%20and%20Drivetrain/Transmission%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Automatic%20Transmission/Reading%20Diagnostic%20Trouble%20Codes/How%20to%20Check%20For%20DTCs%20With%20the%20SCS%20Mode%20%28Retrieving%20the%20Flash%20Codes%29/
[cm-at-reset]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Transmission%20and%20Drivetrain/Transmission%20Control%20Systems/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Automatic%20Transmission/Clearing%20Diagnostic%20Trouble%20Codes/PCM%20Reset%20Procedures/
[cm-cvt]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Transmission%20and%20Drivetrain/Transmission%20Control%20Systems/Testing%20and%20Inspection/Component%20Tests%20and%20General%20Diagnostics/Continuously%20Variable%20Transmission/Component%20Tests/Start%20Clutch%20Calibration%20Procedures/
[cm-mpx]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Body%20and%20Frame/Body%20Control%20Systems/Testing%20and%20Inspection/Multiplex%20Control%20System%20Troubleshooting/
[cm-keyless]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Body%20and%20Frame/Locks/Keyless%20Entry/Keyless%20Entry%20Transmitter/Testing%20and%20Inspection/Programming%20and%20Relearning/
[cm-ecm-update]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Engine%20Control%20Module/Testing%20and%20Inspection/Programming%20and%20Relearning/Updating%20the%20ECM%2FPCM/
[cm-sb02011]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Engine%20Control%20Module/Technical%20Service%20Bulletins/All%20Technical%20Service%20Bulletins/Engine%20Controls%20-%20ECM%20Software%20Update/
[cm-sb03002]: https://charm.li/Honda/2002/Civic%20EX%20Sedan%20L4-1668cc%201.7L%20SOHC%20%28VTEC%29%20MFI/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Engine%20Control%20Module/Technical%20Service%20Bulletins/All%20Technical%20Service%20Bulletins/Engine%20Controls%20-%20MIL%20ON%2FDTC%20P0128%20Is%20Stored%20In%20Memory/
[cm-si-eps]: https://charm.li/Honda/2002/Civic%20Si%20L4-1998cc%202.0L%20DOHC%20%28i-VTEC%29%20PGM%20FI/Repair%20and%20Diagnosis/Steering%20and%20Suspension/Steering/Testing%20and%20Inspection/Reading%20and%20Clearing%20Diagnostic%20Trouble%20Codes/Reading%20Diagnostic%20Trouble%20Codes/
[cm-05-ecm]: https://charm.li/Honda/2005/Civic%20L4-1.7L%20SOHC/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Engine%20Control%20Module/
[cm-05-update]: https://charm.li/Honda/2005/Civic%20L4-1.7L%20SOHC/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Engine%20Control%20Module/Testing%20and%20Inspection/Programming%20and%20Relearning/Updating%20the%20ECM%2FPCM/
[cm-05-p1456]: https://charm.li/Honda/2005/Civic%20L4-1.7L%20SOHC/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/Testing%20and%20Inspection/P%20Code%20Charts/P1456/
[cm-05-dtc]: https://charm.li/Honda/2005/Civic%20L4-1.7L%20SOHC/Repair%20and%20Diagnosis/A%20L%20L%20%20Diagnostic%20Trouble%20Codes%20%28%20DTC%20%29/
