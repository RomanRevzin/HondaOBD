# 02 - Data link connector (DLC) pinout and the legislated OBD-II side: 2001-2005 Honda Civic (7th generation)

Research date: 2026-11-14. Scope: the 16-pin connector itself, the standard (legislated) OBD-II / EOBD protocol, the electrical layer, the SCS line and the other non-standard pins. Honda's proprietary dealer protocol, the control units themselves and reflashing belong to other notes and are only touched under "cross-cutting".

Confidence tags: **HIGH** = primary source, or two independent sources agree; **MEDIUM** = one credible source (or a sister-model factory manual applied to the Civic); **LOW** = single forum post, inference, or conflicting reports.

Method caveats (read before trusting a tag):

- The shared web-search budget ran out part-way through. Everything after that was done by fetching known URLs only.
- honda-tech.com, manualslib.com, justanswer.com, civicforums.com, s2ki.com and web.archive.org refused the fetch tool. Facts from those sites were seen **only through search-engine result summaries** and are marked "(via search summary)". They are never rated above MEDIUM on their own.
- No page of the actual 2001-2005 Civic factory service manual or Electrical Troubleshooting Manual (ETM) could be opened. The factory-manual facts below come from HTML mirrors of the **2002-2006 CR-V** shop manual (same platform generation, same ECM connector layout A/B/E, K20/K24 engines like the EP3) and the **2002-2008 Fit/Jazz** manual. Where they are applied to the Civic that is an inference and is said so.
- Standards and datasheets (ISO 9141-2:1994, ISO 14230-4:2000, ELM327, MC33660, Honda job aid) were read page by page from the original PDFs, so those numbers are first-hand.

## Summary

1. The 7th-generation Civic is a **K-line-only** car. American Honda's own job aid "DLC Circuit Troubleshooting" (June 2017) lists "2001-05 Civic - ALL (includes GX and Si)" under "Equipped with K-Line Only" and the 2006-14 Civic under "both F-CAN and K-Line". No CAN, no J1850 on this car. The coordinator's lead "CAN from model year 2006" is confirmed.
2. Best-evidence DLC pinout: 4 BLK body ground; 5 BRN/YEL ECM logic ground (LG3, ECM E3); 7 LT BLU K-line (ECM E23, plus ABS/SRS/EPS units); 9 BRN SCS (ECM E29); 12 RED/WHT write-enable WEN (ECM E30, documented for K-series cars only); 14 GRY, a 5 V line from the immobilizer control unit; 16 WHT/RED battery via the No. 9 BACK UP fuse. Pins 6 and 15 are empty: no CAN, no L-line.
3. Leads checked: pin 7, 9, 4, 5 confirmed. Pin 16 is **white/red, not white/green**. Pin 14 is grey, but the only description found ties it to the immobilizer control unit, and the name "DIAG-H" could not be verified for this car. Pin 12 = write enable is supported by two sources but not confirmed for D17 cars.
4. Market difference that matters: on "EU model" cars of this Honda generation the SCS line is on a **separate 2P service check connector** (tool 07PAZ-0010100) and not on DLC pin 9; "except EU" cars use DLC pins 4 and 9 (DLC pin box 07WAJ-0010100). One 2002 Civic owner reports a DLC with only cavities 4, 5, 7, 14, 16 populated.
5. Legislated protocol: **ISO 9141-2 on pin 7** (5-baud initialisation with address 0x33, then 10.4 kbit/s). Evidence: Honda's K-line-only statement plus scan-tool compatibility databases that report "ISO 9141" for the Civic VII 1.7 petrol, 1.7 CTDi and 2.0 Type R (2002-2005). No raw ELM327 log of a 7th-gen Civic was found, so key bytes and ECU address are still unknown.
6. All ISO 9141-2 initialisation, timing and electrical numbers were taken from the text of the standard itself (tables below).
7. KWP2000 fast init: no evidence the Civic accepts it. ISO 14230-4 says an OBD ECU supports only one initialisation method, so an ISO 9141-2 ECM is not expected to. Cheap to test.
8. Supported PIDs: **no real 0100/0120/0140 bitmap of a 2001-2005 Civic was found.** The nearest relative with a verifiable log is a 2007 EU Accord K20A6 on ISO 9141-2: `41 00 BE 3E A8 11`, response header `48 6B 11`, "only the engine ECU answers generic OBD". This must be measured on the car.
9. Sampling rate over ISO 9141-2: the standard limits a compliant tester to roughly 7-8.5 single-PID requests per second with key words 08 08; an ELM327 with default settings does about 4-7; custom firmware reached about 20 per second on another ISO 9141-2 car by undercutting the timings.
10. Europe: 2002-2005 EU Civics (including the 1.7 CTDi and the K20A2 Type R) are reported as answering generic OBD on ISO 9141; EOBD was mandatory for petrol cars first registered from 1 January 2001. Japan: unclear, two user-list entries claim ISO 9141-2 on 2001-2002 JDM Hondas, nothing Civic-specific.
11. Electrical: tester pull-up 510 ohm +-5 % to battery on K, at most 2 nF, receiver thresholds 30 % / 70 % of battery voltage, 10.4 kbit/s +-1 %. On the Honda the K-line idles at battery level (factory test: "8.5 V or more" between DLC 5 and 7). SCS is an ECM input that sits at about 5 V and is simply shorted to ground by the tool.
12. Hardware consequence: one 12 V K-line transceiver on pin 7 with a switchable pull-up, a software-controlled switch to ground on pin 9, and for the stretch goals a controllable driver on pin 12 and a separate 5 V single-wire channel on pin 14. Pins 12 and 14 need bench verification before anything is driven onto them.

## Findings

### 1. The 16-pin DLC: location, populated cavities, wires, connected units

#### 1.1 Location

| Fact | Confidence | Source |
|---|---|---|
| Honda's manual wording for this platform generation: "16P Data Link Connector (DLC) ... located under the driver's side of the dashboard" | MEDIUM for the Civic (text is from the CR-V 2 manual) | [CR-V 2 manual, section 11](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_2o.htm), [section 77](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2ak.htm) |
| 2003 Civic LX coupe (US): DLC "mounted in the driver's footwell, a little inboard and below where the driver's right knee would be" | LOW (one owner, via search summary; the summary did not say which thread, most likely one of the honda-tech 2001-2005 Civic threads listed under Sources) | e.g. [honda-tech thread](https://honda-tech.com/forums/honda-civic-2001-2005-78/02-civic-service-check-connector-2386746/) |
| The same CR-V manual also describes the DLC as at the "right side of the under-dash fuse/relay box" in its ABS section, next to the 2P service check connector on EU models | MEDIUM (CR-V, not Civic) | [CR-V 2 manual, ABS](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2aa.htm) |
| EP3 hatchback and right-hand-drive cars: exact position not found | unknown | - |

#### 1.2 Pin table

Connector: SAE J1962 type A, 16 cavities. "E3", "E23" etc. are terminals of ECM/PCM connector E (31P), which has the same assignments on the 2001 Civic D17, the K-series cars and the CR-V 2 as far as the sources go.

| Pin | Wire colour | Honda name | Function | Other end | Confidence | Sources |
|---|---|---|---|---|---|---|
| 4 | BLK | (body ground) | Chassis ground | Body ground | HIGH (function), MEDIUM (colour) | [rywire](https://blog.rywire.com/k-series-dlc-connector-pinout/); CR-V manual: "Repair open in the wire between DLC terminal No. 4 and body ground" ([group_3k](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_3k.htm)) |
| 5 | BRN/YEL | LG3 (logic ground) | Signal ground of the ECM/PCM | ECM/PCM E3 | HIGH | rywire ("pin 5 (Brown/Yellow) is LG3 logic ground from ECU pin E3"); CR-V manual E3 "LG3 (LOGIC GROUND)" BRN/YEL ([group_2o](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_2o.htm)); 2001 Civic procedure "terminal 5 to E3" (via search summary, [justanswer](https://www.justanswer.com/honda/1bezu-no-communication-data-link.html)) |
| 6 | empty | - | No CAN on this car | - | HIGH | [Honda job aid](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf) |
| 7 | LT BLU | K-LINE | Bidirectional serial line: legislated OBD and the dealer tool | ECM/PCM E23; also ABS modulator-control unit, and by inference SRS unit and EPS control unit | HIGH (pin, name, ECM terminal, colour) | rywire ("pin 7 (Light Blue) is K-Line ... to ECU E23"); CR-V manual E23 "K-LINE", LT BLU, "Sends and receives scan tool signal"; 2001-2003 Civic ECM pinout "23-lt blu-K line" (via search summary, [civicforums](https://www.civicforums.com/forums/109-7th-gen-do-yourself-articles/307197-classic-diy-2001-2003-civic-ecu-wiring.html)); 2001 Civic procedure "DLC terminal 7 (ECM terminal E23)" (via search summary, [justanswer](https://www.justanswer.com/honda/3ha2t-2001-honda-civic-sedan-ex-always-ran-great-until.html)) |
| 9 | BRN | SCS (service check signal) | Grounding it puts the units into blink-code mode | ECM/PCM E29 (and the SCS inputs of SRS, ABS, EPS units) | HIGH for US-type cars; see 1.4 for EU-type cars | rywire ("pin 9 (Brown) = SCS ... to ECU pin E29"); CR-V manual E29 "SCS (SERVICE CHECK SIGNAL)" BRN and "Connect the DLC pin box terminals No. 4 and No. 9 with a jumper wire"; Civic ECM pinout "29-brn-Service check signal" (via search summary); US owners: jumping 4 and 9 works on an '03 Civic (via search summary) |
| 12 | RED/WHT (K-series source) | WEN (write enable signal) | Tells the ECM a rewrite is allowed | ECM/PCM E30 | MEDIUM that the function exists on this ECM family; LOW that the cavity is populated on D17 cars; LOW on colour | rywire ("pin 12 (Red/white) = write-enable signal to ECU pin E30 (for rewriting the ECU)"); [pinoutguide](https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml) row "12 - Programming out"; Fit manual E30 "WEN (WRITE ENABLE SIGNAL)", RED/BLU, "Detects write enable signal", "With ignition switch ON (II): about 0 V" ([hondafitjazz](http://www.hondafitjazz.com/manual/a00/html/f6/saa4ef6a20300000000caat10.html)) |
| 14 | GRY | name not verified | A line that sits at about 5 V and comes from the immobilizer control unit | Immobilizer control unit pin 3 | MEDIUM | rywire ("pin 14 (Gray) = +5V from immobilizer control unit pin 3 (probably for rewriting immobilizer codes)"); DMM reading "Pin 14 5V" on a 2002 Civic (via search summary, [honda-tech](https://honda-tech.com/forums/honda-civic-2001-2005-78/honda-civic-2002-strange-obdii-pinout-3366115/)); pinoutguide row "13 or 14? - Input/Output for VTM-4 and immobilizer control" |
| 15 | empty | - | No L-line | - | MEDIUM | Not listed by rywire; not present on the 2002 Civic described on honda-tech |
| 16 | WHT/RED | +B | Unswitched battery | No. 9 BACK UP fuse in the under-hood fuse/relay box | HIGH (function), MEDIUM (colour and fuse rating) | rywire ("pin 16 (White/Red) = +12V battery voltage (from #9 BACKUP fuse 7.5A)"); EP3 manual, DLC terminal No. 16, WHT/RED (via search summary, [manualslib p.334](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=334)); Civic forum "fuse #9 ... white/red wire" (via search summary); CR-V: "No. 9 BACK UP (10A) fuse" |

Notes:

- The rywire page gives no model years. It is a K-series swap vendor, so the pinout is most likely the RSX / Civic Si (EP3) one. Its remarks "ties in with E7 / E9" are swap-harness notes and are not used here.
- The pinoutguide table is a generic Honda page. It labels pin 14 "CAN Low - J-2284 (WEN)" and adds a row "13 or 14?". It is internally inconsistent and is used only as weak corroboration.
- Fuse rating: 7.5 A for the Civic/RSX per rywire and a forum post; 10 A on the CR-V; "No. 8 (15A)" on the Fit. Check the fuse-box lid of the actual car.

#### 1.3 Which control units are on which wire

| Wire | Units | Confidence | Evidence |
|---|---|---|---|
| K-line (pin 7) | ECM/PCM (E23) | HIGH | See pin table |
| K-line (pin 7) | ABS modulator-control unit | LOW-MEDIUM | 2002 Civic EX owner traced the K-line to "the ABS plug under the hood" and to the ECU (via search summary, [honda-tech](https://honda-tech.com/forums/honda-civic-2001-2005-78/new-guy-dlc-pin-7-communication-issues-3093207/)); CR-V manual: "The ABS can be diagnosed with the Honda PGM Tester" ([group2aa](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2aa.htm)) |
| K-line (pin 7) | SRS unit | MEDIUM (inference) | Honda SRS procedure of this generation: "Connect the Honda PGM Tester to the 16P DLC, then follow the prompts in the SRS menu" ([Jazz manual](https://hondziarze.duckdns.org/jazz/A00/HTML/00/SAA2E00K79100000000BBAT00.HTML)); the job aid says every HDS link on this car is K-line |
| K-line (pin 7) | EPS control unit (EP3 / Si only) | MEDIUM (inference) | EP3 manual has "How To Retrieve EPS DTCs; PGM Tester Method; Service Check Signal Circuit Method" (page title, via search summary, [manualslib p.542](https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=542)) |
| K-line (pin 7) | Gauge assembly | LOW | Job aid generic step: "Check the K-Line circuit for continuity between the PCM and gauge control module" (written for all K-line-only models) |
| SCS (pin 9) | ECM/PCM (E29); SRS, ABS (and EPS) react by blinking their indicators | HIGH (ECM), MEDIUM (others) | See section 6 |
| Pin 14 | Immobilizer control unit | MEDIUM | See pin table |
| Pin 12 | ECM/PCM E30 | MEDIUM-LOW | See pin table |
| Not on the DLC | Multiplex control unit: self-diagnosis is entered through its own "multiplex control inspection connector" with "MPCS Service Connector 07WAZ-0010100"; it talks to the ECM over a YEL wire to ECM E24 | MEDIUM (CR-V manual; Civic ECM pinout also lists "24-yel-Multiplex control unit comm line") | [CR-V manual, multiplex](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group31c.htm) |

#### 1.4 Market and engine differences

| Topic | US / Canada | Europe (EU models) | Japan | Confidence |
|---|---|---|---|---|
| Connector | 16-pin J1962 | 16-pin J1962 | 16-pin presumed | HIGH (US), MEDIUM (EU, tool reports), LOW (JDM) |
| K-line on pin 7 | yes | yes | presumed | HIGH / MEDIUM / LOW |
| SCS access | DLC pin 9; "There is no separate SCS connector in the '03 Civic like on the older models. You must jump two terminals in the DLC" (owner, via search summary) | Separate "service check connector (2P)" shorted with "SCS Short Connector 07PAZ-0010100 (EU model)"; "DLC Pin Box 07WAJ-0010100 (Except EU model)". The CR-V manual names the model codes: "KG, KS, KE, KR, KU (Hong Kong) models" use the 2P connector, all others jump DLC "No. 4 and No. 9" | unknown | MEDIUM for the Civic (CR-V manual is explicit; one 2002 Civic owner reports no pin 9, market not stated) |
| D17 (sedan/coupe) versus K20 (EP3) | Same ECM connector E functions (E3, E23, E29) in every source seen. EP3 and Si add the EPS control unit as a diagnosable unit. Pin 12 / pin 14 wiring is documented only by a K-series source. | | | MEDIUM |

Honda-specialist vendor Hondash states that Hondas with "standard OBD2 socket (US/CA '96-, others '99-)" are not served by its native-protocol scanner and "require a ELM327 scanner", while other markets kept the 3-pin (some European models 5-pin) Honda connector on OBD1/2A/2B cars until '01, "some markets until '03" ([hondash FAQ](https://www.hondash.net/p/faq.html), [compatibility check](https://www.hondash.net/2018/07/how-to-check-if-hondash-will-work-with.html)). MEDIUM. Reading: the 7th-gen Civic, with its 16-pin socket, falls on the generic-OBD side in every market according to this vendor.

### 2. Which legislated protocol the car speaks

#### 2.1 Evidence

| # | Evidence | What it shows | Confidence |
|---|---|---|---|
| 1 | American Honda job aid "DLC Circuit Troubleshooting", June 2017: "all models are equipped with a K-Line to DLC circuit, and some are equipped with an additional F-CAN to DLC circuit"; "Equipped with K-Line Only: ... 2001-05 Civic - ALL (includes GX and Si) ... 2003-05 Civic Hybrid"; "Equipped with both F-CAN and K-Line: ... 2006-14 Civic" ([Honda](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf), [Acura twin](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116738-9999.pdf)) | Only a K-line reaches the DLC. The only legislated K-line protocols are ISO 9141-2 and ISO 14230-4. | HIGH (primary) |
| 2 | klavkarr / outilsobdfacile database, "Honda Civic VII (2001 - 2005)", vehicles tested with their ELM327-based tool: "1.7 CTDI Protocol: ISO 9141, 2002 2004, Diesel, 100"; "1.7 mfi Protocol: ISO 9141, 2003, Gasoline, 115"; "2.0 16V Protocol: ISO 9141, 2002 2005, Gasoline, 200"; "2.0 Type-R Protocol: ISO 9141, 2003, Gasoline, 200" ([klavkarr](https://www.klavkarr.com/compatible/honda-civicvii-21000116)) | Tool-detected protocol on real cars. The same database reports "CAN 29bit 500kb" for every Civic VIII, so the field is real detection output. | MEDIUM-HIGH |
| 3 | pinoutguide Honda list: "Civic coupe em2, 2001, Gasoline (120 HP): ISO 9141-2"; other Civic 2001-2005 rows "ISO 14230-4, ISO 9141-2" or "ISO 9141"; summary line "1996 - 2006 : ISO 9141" ([pinoutguide](https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml)) | Agrees, but several rows are the same cars as in #2, so not fully independent | MEDIUM |
| 4 | elm327rus.ru compatibility list: "Honda Civic, gasoline, 2001, United States: ISO9141-2/ISO14230-4" ([elm327rus](https://elm327rus.ru/sovmestimost/)) | K-line family, does not separate 9141 from KWP | LOW |
| 5 | Hondash: US OBD2 ECUs "boot and initialize in a standard ISO-compatible mode at 10.4K baud", unlike non-US OBD2A/B ECUs with "native 9.6K PGM-FI communication" ([hondash wiring](https://www.hondash.net/2018/03/dlc-interface-power-management.html)) | Honda US ECMs of the preceding generation already used the ISO mode at 10.4 kbit/s | MEDIUM |
| 6 | Related Honda K-line car with raw log: 2007 EU Accord K20A6, `ATDPN` = `A3` (ELM327 protocol 3 = ISO 9141-2) ([wican-fw issue 1035](https://github.com/meatpiHQ/wican-fw/issues/1035)) | Honda K-series ECMs of the K-line era do ISO 9141-2 with 5-baud init | MEDIUM (other model) |
| 7 | OBDb community repo tags its generic commands for the Civic 1997-2005 test cases as `686A...|p=9141-2` ([OBDb/Honda-Civic](https://github.com/OBDb/Honda-Civic)) | Community expectation only; those files contain no confirmed-supported command | LOW |

Conclusion: **ISO 9141-2 on pin 7 for US cars: HIGH.** Not found: a raw ELM327 session from a 2001-2005 Civic showing `ATDPN` -> `3`. That is the first thing to capture on the real car.

#### 2.2 Does it also accept ISO 14230 (KWP2000) fast init?

- ISO 14230-4:2000, 4.4: "ECU(s) (OBD related) shall only support one of the two following methods of initialisation: 5 baud initialisation; fast initialisation. The scan tool shall support both methods." "Keywords received by the scan tool can be 2025, 2027, 2029 and 2031." "In the case where 5 baud initialisation is used, then the 5 baud address shall be 33H and subsequent communication shall take place at 10 400 baud." ([ISO 14230-4](https://law.resource.org/pub/us/cfr/ibr/inc/iso/iso.14230-4.2000.pdf)) HIGH.
- Same document, clause 3: "ISO 14230-1 and ISO 9141-2 physical layers are fully compatible. The only difference between these two standards is that ISO 14230-1 additionally supports 24 V systems." HIGH. One transceiver serves both.
- For the Civic: no report of a successful fast init was found. The databases that say "ISO 14230-4, ISO 9141-2" for a Civic do not say which initialisation worked. **Unknown, expected "no" (LOW).** Honda's own dealer protocol on the same wire is a separate question (other dimension).

#### 2.3 ISO 9141-2 initialisation, framing and timing (from the standard)

All rows HIGH: read directly in [ISO 9141-2:1994(E)](https://law.resource.org/pub/us/cfr/ibr/inc/iso/iso.9142-2.1994.pdf).

| Item | Value | Clause |
|---|---|---|
| Initialisation address | 33 hex, sent by the tester at 5 bit/s, on K and L simultaneously ("providing an initialization, simultaneously on the K and L lines") | 6, 8.2 |
| 5 bit/s tolerance (tester) | +-0.5 % | 8.2 c |
| Address byte format | start bit (0), 8 data bits LSB first, stop bit (1) | 6 |
| Idle before address (W0) | K at logic 1 for at least 2 ms | Fig. 6, A.1 |
| ECU synchronisation byte | 55 hex at 10.4 kbit/s ("8 alternate bits starting with a logic 1 bit") | 7.2 |
| Key words | two bytes, 7 data bits + odd parity: **08 08** or **94 94** (hex including parity) | 7.3, Fig. 6 |
| Meaning of the key words | 08 08: message separation by timing only, P2 min = 25 ms. 94 94: "another type of message synchronization", P2 min = 0 | 12 |
| Tester acknowledgement | logic inversion of key word 2: F7 (for 08) or 6B (for 94) | 7.3.1, Fig. 6 |
| ECU final byte | logic inversion of the address: CC | 7.3.1, Fig. 6 |
| W1 (end of address to sync) | 60-300 ms | A.1 |
| W2 (sync to key word 1) | 5-20 ms | A.1 |
| W3 (key word 1 to key word 2) | 0-20 ms | A.1 |
| W4 (key word 2 to inverted key word; inverted key word to inverted address) | 25-50 ms | A.1 |
| W5 (bus idle before the tester retransmits the address) | at least 300 ms | A.1 |
| Worst-case initialisation time | 2.44 s | A.1 note |
| Data rate | 10.4 kbit/s; tester +-1 %, ECU +-2 % | 8.3.2, 9.3 |
| Byte format after init | start bit, 8 data bits LSB first, 1 stop bit | 11.1 |
| Request frame | 68 6A F1 + up to 7 data bytes + checksum | Table 1 |
| Response frame | 48 6B <ECU address> + up to 7 data bytes + checksum | Table 1 |
| Message length | 5 to 11 bytes | 11.1 |
| Checksum | 8-bit sum of all bytes before it, modulo 256 | 11.2 |
| P1 (ECU inter-byte) | 0-20 ms | A.2 |
| P2 (request to response, and between responses) | 25-50 ms with key words 08; 0-50 ms with key words 94 | A.2 |
| P3 (end of responses to next request) | 55 ms to **5 s** | A.2 |
| P4 (tester inter-byte) | 5-20 ms | A.2 |
| Idle timeout | "If this period [P3] is too long, then the vehicle shall assume that communication is over and shall subsequently require complete reinitialization"; the tester "should periodically transmit dummy messages" | 13.2.5 |
| Retry rule | on errors the tester retries "until 1 min has elapsed since the last valid response", then re-initialises | 13.2 |

Example request computed from Table 1 (my arithmetic, not a capture): `68 6A F1 01 00` has checksum C4, so mode 01 PID 00 goes out as `68 6A F1 01 00 C4`.

How a widely used tester sets these (ELM327 v2.0 datasheet, read first-hand, HIGH; [ELM327DSI](https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf)):

| ELM327 parameter | Default | Meaning |
|---|---|---|
| Protocol numbers | 3 = ISO 9141-2, 4 = KWP 5-baud, 5 = KWP fast | `AT SP h`, `AT DPN` |
| `AT IIA` | 33 | slow-init address |
| `AT IB` | 10 (10400 baud); 48 and 96 selectable | ISO baud rate |
| PP 14 | 5.2 ms | P4 inter-byte time |
| PP 15 | 21 ms | "maximum inter-byte time (P1), and minimum inter-message time (P2)" |
| PP 1D | 59 ms | P3, delay before sending requests |
| PP 17 / `AT SW` | 92 hex = 2.99 s | wake-up (keep-alive) interval |
| `AT WM` default | "68 6A F1 01 00 for ISO 9141, and C1 33 F1 3E for KWP" | keep-alive message |
| PP 1A / 1B | 25 ms / 25 ms | fast-init low / high times |
| PP 1C | 03 | initiation is driven on both K and L outputs |
| PP 03 / `AT ST` | 32 hex, about 205 ms | NO DATA timeout |
| PP 06 | F1 | tester address |

#### 2.4 Honda-specific values

| Item | Value | Confidence |
|---|---|---|
| Key words sent by a 7th-gen Civic ECM | not found | unknown |
| ECU address in responses | not found for the Civic. 2007 EU Accord K20A6: `48 6B 11 41 05 7D 87` and four more frames; I recomputed all five checksums and they are correct, so the source address 0x11 is genuine for that car ([wican-fw issue 1035](https://github.com/meatpiHQ/wican-fw/issues/1035)) | LOW for the Civic. Firmware must accept any address in byte 3. |
| L-line needed | no, pin 15 is not populated | MEDIUM |
| Number of responders to functional requests | "Only the engine ECU answers generic OBD" (Accord report); "generic OBD readers will not read beyond the engine stuff on a Honda. You need a Honda specific scanner to talk to ABS and SRS" (honda-tech, via search summary) | MEDIUM |

### 3. SAE J1979 services and PIDs actually supported

| Question | Finding | Confidence |
|---|---|---|
| 0100 bitmap of a 2001-2005 Civic | **Not found in any source.** | unknown |
| Nearest verified relative | 2007 EU Accord 2.0 K20A6, ISO 9141-2: `0100` -> `41 00 BE 3E A8 11`. Decoded: PIDs 01, 03, 04, 05, 06, 07, 0B, 0C, 0D, 0E, 0F, 11, 13, 15, 1C, and bit for PID 20 set (a 0120 block exists). Live frames seen for 04, 05, 06, 0C, 11. | MEDIUM for that car, LOW as a prediction for the Civic |
| What a community database assumes for older Civics | OBDb's generic ISO 9141-2 command set for the Civic contains 0101, 0103, 0104, 0105, 0106, 0107, 010B, 010C, 010D, 010E, 010F, 0111, 011C, 0134. Its 1997, 1998, 2004 and 2005 test files list these as not answered and have no confirmed-supported entries, so they prove nothing either way ([2004](https://raw.githubusercontent.com/OBDb/Honda-Civic/main/tests/test_cases/2004/command_support.yaml), [2005](https://raw.githubusercontent.com/OBDb/Honda-Civic/main/tests/test_cases/2005/command_support.yaml)) | LOW |
| Mode 02 freeze frame, 03, 04, 07 | Honda manual of this generation: "The scan tool and the Honda PGM Tester can read the DTC, freeze frame data, current data" ([CR-V manual](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_2o.htm)). No raw log. | MEDIUM that they work, details unknown |
| Mode 05, 06, 09 (VIN) | No report found. Whether a 2002 ECM returns a VIN is unknown. | unknown |
| Automatic transmission codes | The PCM contains the A/T control on automatic cars ("ECM" on manual, "PCM" on automatic in the manuals), so powertrain codes for the transmission come from the same node. Dealer-level A/T data needs the Honda protocol. | MEDIUM (inference) |

Sampling rate over ISO 9141-2:

| Basis | Rate | Confidence |
|---|---|---|
| Arithmetic from the standard, single-PID request with a 2-byte value, key words 08 08: request 6 bytes (about 5.8 ms on the wire + 5 x 5 ms P4 = about 31 ms) + P2 25-50 ms + response 8 bytes (about 7.7 ms) + P3 55 ms = about 119-144 ms per cycle | about 7.0-8.4 requests/s for a strictly compliant tester; about 10.7/s if the ECM uses key words 94 94 and answers immediately | MEDIUM (derived) |
| ELM327 datasheet: "If a typical vehicle query response time were about 50 msec, and the timeout were set to 200 msec, the fastest scan rate possible would only be about 4 queries per second. Changing the ST time to about 100 msec would almost double that rate, giving about 7 queries per second." Adaptive timing is on by default; appending the expected response count (for example `01 0C 1`) removes the final wait. | about 4-7/s, more with tuning | HIGH (datasheet), not Honda-specific |
| OBD9141 Arduino library on "one Kia car build in 2004": "up to 20 requests per second", against "a maximum of 6 readings per second" for a Bluetooth OBD-II dongle on the same car ([OBD9141](https://github.com/iwanders/OBD9141)) | about 20/s by undercutting P3/P4 | MEDIUM, other make |
| Measured on a 7th-gen Civic | not found | unknown |

### 4. European and Japanese cars

| Market | Finding | Confidence |
|---|---|---|
| EU, legal background | EOBD applies to M1 cars "first registered within EU member states since January 1, 2001 for petrol-engined cars" and since 1 January 2004 for diesel; one year earlier for new models ([Wikipedia](https://en.wikipedia.org/wiki/On-board_diagnostics)) | MEDIUM (secondary source) |
| EU petrol, 2002 | Legally an EOBD car. The 2.0 Type R (K20A2, 200 hp) of 2002, 2003 and 2005 is listed as "ISO 9141" by the klavkarr database. The 1.4 / 1.6 (D14Z6, D16V1, D16W7) were not found in any list; one Russian list names a "Honda Civic 5D, petrol, 2001, Germany, 1.4, 90 hp" as ELM327-compatible without a protocol ([elm327rus](https://elm327rus.ru/sovmestimost/)) | MEDIUM-HIGH (Type R), LOW (1.4/1.6) |
| EU diesel 1.7 CTDi | Listed as "ISO 9141" for 2002 and 2004, although diesel EOBD only became mandatory in 2004 | MEDIUM |
| EU, SCS | Separate 2P service check connector expected, see 1.4 | MEDIUM |
| Japan | Nothing Civic-specific with a protocol. The Russian user list has "Honda Accord wagon, 2001, Japan: ISO 9141-2 (5 baud init, 10.4 kbaud)", "Honda fit, 2002, japan: ISO 9141-2 (5 baud init, 10.4 kbaud)" and a row "Honda Civic 7th Gen, petrol, 2001-05, USA, CAN, JAP, EU, SIN, ENG" without protocol. A Japanese OBD app FAQ warns in general terms that cars using "a protocol specific to Japanese cars" are not OBD-II and cannot be diagnosed generically ([ganchi](https://ganchi.rdy.jp/AppObdInfoSan/qa.html)). | LOW. Treat a JDM car as "may or may not answer 0100". |

### 5. Electrical layer

From ISO 9141-2:1994 (HIGH, first-hand):

| Parameter | Value | Clause |
|---|---|---|
| Idle state | logic 1, line high (pulled to battery voltage VB) | 6, 9.2.1 |
| Transmitter levels | logic 0 below 20 % VB, logic 1 above 80 % VB | 5.1 |
| Receiver thresholds | logic 0 below 30 % VB, logic 1 above 70 % VB; between 30 % and 70 % is undefined | 5.1 |
| Edge slope | less than 10 % of the bit time (20 %-80 %) | 5.1 |
| Tester pull-up | K and L "internally pulled up to VB via a nominal 510 ohm resistor", 510 ohm +-5 % in both transmit-high and receive state | 8.3.1, A.4 |
| Tester low level | below 10 % VB "at a maximum sink current of 2 A" | 8.3.1 |
| Tester supply range | must work from 8 V to 16 V battery voltage | 8.3 |
| Tester must not put on the lines | anything above VB or below -1 V | 8.3.4 |
| What the tester may expect from the vehicle | at least 5 kohm to ground and at least 10 kohm to VB on K and L | 8.3.5 |
| Tester + cable capacitance | at most 2 nF | 8.3.6, A.3 |
| On-board wiring capacitance | at most 2 nF | 10, A.3 |
| All ECUs together | at most 5.6 nF; on-board wiring plus all ECUs at most 7.6 nF; 500 pF recommended per ECU | 5.2.2, A.3 |
| ECU pull-up, if fitted | at least 100 kohm to VB | 9.2.1, A.4 |
| ECU pull-down | at least 50 kohm to ground | 9.2.1, A.4 |
| ECU low-side driver | equivalent sink resistance at most 110 ohm | 9.2.1 |
| Overvoltage the ECU pins must survive | 20 V permanently, 24 V for 30 min, 30 V for 1 min | 9.2.3 |
| L-line | "unidirectional line and is only used during initialization"; an ECU must accept the address "on either or both K and L lines" | 4.1, 9.3 |

Honda-side observations:

| Observation | Confidence | Source |
|---|---|---|
| Factory test: ignition ON, "Measure voltage between DLC terminals No. 5 and No. 7. Is there 8.5 V or more?" is the pass condition for the K-line. With ECM connector E unplugged the same measurement must read 0 V, otherwise "Repair short to power in the wire between the DLC terminal No. 7 and the ECM/PCM (E23)". So the pull-up to battery level is inside the ECM/PCM. | MEDIUM-HIGH (identical wording in the CR-V 2 and Fit/Jazz manuals; the 2001 Civic procedure is reported to follow the same steps) | [CR-V group_3k](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_3k.htm), [Fit/Jazz](http://www.hondafitjazz.com/manual/a00/html/97/saa2e97a20326919461faat00.html) |
| ECM terminal E23 signal: "With ignition switch ON (II): pulses or battery voltage" | MEDIUM-HIGH | [CR-V group_2o](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_2o.htm) |
| DMM readings at DLC pin 7 on 2002 Civics: "Pin 7 12V" and "11.9 volts" | LOW each, consistent with the manual (via search summary) | honda-tech threads 3366115 and 3093207 |
| Value of the ECM-side pull-up resistor | not published anywhere found | unknown, measure |
| L-line | not used | MEDIUM |
| A failed 5 V sensor reference can stop K-line communication: the first vehicle-side measurement in the job aid's K-line procedure (step 4) is "Measure the voltage on the MAP sensor VCC circuit", pass band "between 4.5 and 5.5 V" | HIGH | [Honda job aid](https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf) |

Interface parts (first-hand datasheet facts unless marked):

| Option | Facts | Source / status |
|---|---|---|
| Discrete, ELM327 reference circuit | NPN transistor (base resistor 2.2 kohm) pulls K low; 510 ohm pull-up to battery on K and on L; receive through a 47 kohm / 33 kohm divider into a Schmitt input giving "typical threshold levels of 7.0V (rising) and 3.6V (falling)"; "a short at 13.8V causes about 0.4W dissipation", hence 1/2 W resistors; L-line parts may be left out "If your vehicle does not require the L line" | [ELM327DSI p.71-73](https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf), HIGH |
| NXP MC33660 (ISO K-line serial link interface) | VBB 8.0-18 V; ISO input low threshold max 0.4 x VBB, high threshold min 0.7 x VBB, hysteresis 0.05-0.1 x VBB; internal pull-up current only 5-140 microamp, so the external 510 ohm is still required (characterised with RISO = 510 ohm); short-circuit current limit 50-200 mA; ISO pin load-dump 40 V; thermal shutdown | [MC33660 datasheet](https://www.nxp.com/docs/en/data-sheet/MC33660.pdf), HIGH |
| MC33290, SN65HVDA100, SN65HVDA195 | "all three worked with the typical application circuit from the datasheet" in the OBD9141 project | [OBD9141](https://github.com/iwanders/OBD9141), MEDIUM; datasheets not read here |
| ST L9637 | datasheet download timed out | not verified here |

Design warning from the ELM327 datasheet (HIGH): when listening to another tester, do not add a second pull-up. "Both the ELM327 and the other scan tool would present 510 ohm resistors, so the vehicle would see 255 ohm. This would very likely cause data errors, and might even damage some circuitry." It suggests switching 10 kohm in series for monitoring. The pull-up on the planned device should therefore be switchable (510 ohm for tester mode, off or 10 kohm and above for sniffing the dealer tool).

### 6. Pin 9, the SCS (service check signal) line

| Fact | Confidence | Source |
|---|---|---|
| ECM/PCM terminal E29, BRN, "SCS (SERVICE CHECK SIGNAL)", "Detects service check signal". "With the service check signal shorted with the PGM Tester: about 0 V"; "With the service check signal opened: about 5 V". The MIL troubleshooting flow accepts "about 5 V (or battery voltage)" at E29. | HIGH for the level (CR-V 2 and Fit manuals agree); Civic colour and terminal confirmed by the ECM pinout post | [CR-V group_2o](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_2o.htm), [CR-V group2ak](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2ak.htm), [Fit](http://www.hondafitjazz.com/manual/a00/html/f6/saa4ef6a20300000000caat10.html) |
| The dealer tool itself shorts the line: later manual wording is "shorted with the HDS or the scan tool" | MEDIUM-HIGH | Fit manual |
| Electrical nature: an input pulled up inside the ECM to about 5 V (other units on the wire have their own pull-ups); the tool only has to connect it to ground (DLC pin 4). Required sink current: **not published**; expect milliamps. | MEDIUM (inference), current unknown | - |
| Honda tools: "SCS Short Connector 07PAZ-0010100 (EU model)" for the 2P service check connector; "DLC Pin Box 07WAJ-0010100 (Except EU model)": "Connect the DLC pin box terminals No. 4 and No. 9 with a jumper wire (B), then push the switch (C)" | MEDIUM-HIGH (CR-V 2 manual, two sections) | [group2aa](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2aa.htm), [group2ak](https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2ak.htm) |
| A third part number, "07WAZ-002010A", is given by one site as the SCS connector for an Accord 16-pin DLC | LOW, uncorroborated | [alflash](https://alflash.com.ua/Honda/) |
| 7th-gen Civic, owner reports: with 4 and 9 jumped and ignition on, "the MIL, airbag and ABS light will blink DTC's"; "The pin trick 4 & 9 does work on Civic" | MEDIUM (two forums, via search summary) | [honda-tech](https://honda-tech.com/forums/honda-civic-2001-2005-78/get-engine-code-without-scanner-2900091/), [civicforums](https://www.civicforums.com/forums/36-mechanical-problems-vehicle-issues-fix-forum/352527-possible-pull-check-engine-light-codes-no-scanner.html) |
| Engine: MIL blinks the Honda flash codes | HIGH | manuals above |
| ABS: "The blinking frequency indicates the DTC"; clearing with the SCS line shorted uses a brake-pedal sequence, "After a few seconds, the ABS indicator blinks twice and the DTC is cleared" | MEDIUM (CR-V) | group2aa |
| SRS: PGM Tester "SRS" menu, or DLC pin box 4-9 jumper, then "the SRS indicator will indicate the diagnostic trouble code (DTC) by the number of blinks". Erasing uses a different connector: "SCS short connector (07PAZ-0010100)" on the "MES connector (2P)", with remove/reconnect steps "within 4 seconds" | MEDIUM (Jazz manual; Civic owners also say the reset connector differs from the code-reading one) | [Jazz SRS](https://hondziarze.duckdns.org/jazz/A00/HTML/00/SAA2E00K79100000000BBAT00.HTML) |
| EPS (EP3 / Si): a "Service Check Signal Circuit Method" exists in the EP3 manual | MEDIUM (page title only) | manualslib p.542 |
| Immobilizer indicator reacting to SCS | no evidence found | unknown |

### 7. Other populated pins

| Pin | What is known | What is not known | Confidence |
|---|---|---|---|
| 12 | Write enable. K-series pinout: RED/WHT to ECM E30 "for rewriting the ECU". The ECM terminal exists on the same ECM family: "WEN (WRITE ENABLE SIGNAL) - Detects write enable signal - With ignition switch ON (II): about 0 V". So it is an ECM input that idles low and is presumably driven high by the tool during an update. | Whether the cavity is populated on D17 cars (one 2002 Civic owner lists only 4, 5, 7, 14, 16); the voltage the tool applies; timing relative to the K-line session | MEDIUM-LOW |
| 14 | GRY wire, about 5 V at rest, from the immobilizer control unit (pin 3). Not CAN on this car. | Signal name (the lead "DIAG-H" is not confirmed for this model), direction, protocol, whether any other unit is on it | MEDIUM |
| 1, 2, 3, 6, 8, 10, 11, 13, 15 | Not populated as far as any source says | A physical look at the car | MEDIUM |

Protocol-discovery session to run first on the real car (ELM327-style commands, all present in the datasheet's command list):

| Step | Command | Purpose |
|---|---|---|
| 1 | `ATZ`, `ATE0`, `ATL0`, `ATH1`, `ATS1` | reset, show headers |
| 2 | `ATSP3` then `0100` | force ISO 9141-2; expect "BUS INIT: OK" and `48 6B xx 41 00 ...` |
| 3 | `ATDPN`, `ATKW` | protocol number; key words (08 08 or 94 94) |
| 4 | `0100`, `0120`, `0140` | supported-PID bitmaps |
| 5 | `0101`, `03`, `07` | readiness, stored and pending DTCs |
| 6 | `02 00 00` and `02 02 00` (PID, then frame number 00) | freeze-frame support and the DTC that stored the frame |
| 7 | `0900`, `0902` | vehicle information / VIN support |
| 8 | `0600`, `05 00 01`-style requests | mode 06 / mode 05 support |
| 9 | `ATSP5` then `0100`; `ATSP4` then `0100` | test KWP fast init and KWP 5-baud |
| 10 | never send `04` during discovery | it clears DTCs and readiness |

### 8. Targets for the reverse-engineering toolkit noticed in this dimension

- ECM/PCM firmware dump: the K-line service routine (5-baud address 0x33 detection, key-word constants, the PID-support bitmaps, the handler for functional header `68 6A F1`), the SCS input and blink-code routine, and the WEN input that gates reprogramming. Finding the bitmap constants in the dump answers question 3 without a car.
- Dealer software (HDS / PGM Tester data) and its vehicle-interface driver: how it shorts pin 9, what it drives on pin 12 and how it uses pin 14 during immobilizer functions.
- Logic-analyser or serial captures on pins 7, 9, 12 and 14 while a Honda-capable tool is connected (use a high-impedance tap, see the pull-up warning in section 5).
- The CR-V 2 manual mirror refers to an "ECM/PCM update" procedure at its page 11-5, and the Fit manual documents the WEN terminal: evidence that dealer reflashing through the DLC existed for this ECM generation.

## Conflicts and uncertainties

1. **Pin 16 colour.** Lead said white/green. EP3 manual (search summary), rywire and a Civic forum post all say WHT/RED. No source said white/green. The lead is most likely wrong.
2. **Pin 14.** rywire: grey, 5 V from the immobilizer control unit. pinoutguide: "CAN Low - J-2284 (WEN)" plus an uncertain row "13 or 14? - Input/Output for VTM-4 and immobilizer control". The generic J1962 meaning (CAN low) does not apply to this car (HIGH, job aid). The immobilizer reading is preferred; the name is unverified.
3. **Pin 12 and pin 9 presence.** A 2002 Civic owner (market unknown) found only 4, 5, 7, 14, 16 populated: no pin 9, no pin 12. US owners of '03 cars use pin 9 successfully. Most likely explanation: EU-type car with the separate 2P service check connector. Not proven.
4. **Separate SCS connector on a 2002 Civic.** One thread says a 2002 owner was told by Honda that only Civics up to 2001 have a separate connector; another 2002 thread describes two connectors below the glove box (a 2-pin SCS and a 3-pin one). Both via search summary; unresolved, possibly a non-US car or a misidentified plug.
5. **Voltage on pin 7 with the ECM unplugged.** The factory procedure expects 0 V. For a 2002 Civic Si, one summary of a technician thread gives "4.2 volts", another gives "about 11 volts". Either other units also pull the line up on that car, or the car had a wiring fault. Unresolved.
6. **"ISO 14230-4, ISO 9141-2" in compatibility lists.** Two lists print both names for some Civic rows; the klavkarr database prints only "ISO 9141". The lists do not say which initialisation succeeded. Treated as ISO 9141-2.
7. **Response address 0x11.** Seen on one 2007 EU Accord. Many ISO 9141-2 cars use 0x10. Nothing known for the Civic.
8. **Fuse rating for pin 16.** 7.5 A (rywire, forum) against 10 A (CR-V manual). Model-dependent.
9. **Wire colours differ between models.** The K-line on the Fit is reported as RED/WHT, not LT BLU, and its WEN wire as RED/BLU where the K-series source says RED/WHT (the Fit colours were extracted by the fetch tool's summariser and the WEN and IMOCD rows came out with the same colour, so treat them with caution). Colours must not be carried over between models without checking; the LT BLU for the Civic comes from a Civic ECM pinout post and from K-series sources.
10. **Sister-model inference.** Every factory-manual quotation here is from the CR-V 2, Fit/Jazz or (titles only) EP3 manuals. The Civic sedan/coupe manual may differ in details, especially pins 12 and 14 and the SCS arrangement by market.

## Open questions

| # | Question | How to answer |
|---|---|---|
| 1 | Which cavities are populated on the owner's car, and what voltage is on each (ignition off / on)? | Look into the DLC with a torch; DMM on pins 4, 5, 7, 9, 12, 14, 16. Expected: 7 near battery voltage, 9 about 5 V, 14 about 5 V, 12 about 0 V. |
| 2 | Market, engine, transmission, ECM part number | VIN plate and ECM label; decides the EU/US/JDM branch above |
| 3 | Does the ECM answer ISO 9141-2, with which key words, which response address, and what are the real W1-W4 / P2 times? | Session in section 7 with any ELM327, plus a logic analyser on pin 7 for the timings |
| 4 | Does it accept KWP2000 fast init or 5-baud KWP? | Steps 9 of the session |
| 5 | 0100 / 0120 / 0140 bitmaps, mode 02, 05, 06, 09 support | Same session; or find the bitmap constants in an ECM dump |
| 6 | Achievable polling rate | Measure P2 on the car, then tune P3/P4 in own firmware |
| 7 | Value of the vehicle-side K-line pull-up | Measure the idle voltage unloaded, then with 10 kohm and with 1 kohm to ground, ignition on, no tester connected |
| 8 | SCS: open-circuit voltage at pin 9, short-circuit current, which indicators blink, whether the immobilizer lamp reacts | Milliammeter between pins 9 and 4; observe the cluster. Do this before choosing the switch transistor. |
| 9 | Pin 14: signal name, protocol, which units | Civic ETM "Data Link Connector" and "Immobilizer System" circuit pages; sniff during an immobilizer function of a dealer-level tool; reverse-engineer the dealer software |
| 10 | Pin 12: is it wired on D17 cars, what level and timing does the tool apply? | ETM page; service manual "ECM/PCM update" page; sniff a real update; firmware analysis of the WEN input |
| 11 | Civic-specific confirmation of every colour in the pin table | 2001-2005 Civic ETM (connector view of the DLC) and service manual section 11 "DLC Circuit Troubleshooting" and "ECM/PCM Inputs and Outputs at Connector E (31P)". The EP3 manual pages 247, 334-335 and 542 on manualslib open in a normal browser but not with the fetch tool. |
| 12 | Which units share the K-line on the Civic (ABS, SRS, EPS, gauge, immobilizer) | ETM K-line circuit page; or unplug units one at a time as in the job aid |
| 13 | Do JDM 7th-gen Civics answer generic OBD? | Only relevant if the car is JDM; test with the session above; Japanese owner blogs were not conclusive |
| 14 | EU cars: where is the 2P service check connector on the Civic (EP1/EP2/EP3/EU7...) | EP3 / European Civic shop manual; the CR-V has it at the "right side of the under-dash fuse/relay box" |

## Sources

Primary documents read first-hand:

- https://law.resource.org/pub/us/cfr/ibr/inc/iso/iso.9142-2.1994.pdf - ISO 9141-2:1994(E), full text (the file name really is "9142-2"): initialisation, timing tables A.1/A.2, electrical tables A.3/A.4.
- https://law.resource.org/pub/us/cfr/ibr/inc/iso/iso.14230-4.2000.pdf - ISO 14230-4:2000(E), KWP2000 requirements for emission-related systems.
- https://static.nhtsa.gov/odi/tsbs/2017/MC-10116735-9999.pdf - American Honda job aid "DLC Circuit Troubleshooting", June 2017: K-line-only versus F-CAN vehicle lists, K-line fault procedure.
- https://static.nhtsa.gov/odi/tsbs/2017/MC-10116738-9999.pdf - Acura edition of the same job aid.
- https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf - ELM327 v2.0 datasheet (ELM327DSI): AT commands, programmable-parameter defaults, ISO interface circuit, timing discussion.
- https://www.nxp.com/docs/en/data-sheet/MC33660.pdf - MC33660 K-line transceiver datasheet: thresholds, current limit, ratings.

Factory-manual mirrors (sister models, HTML):

- https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_2o.htm - CR-V (2002-2006) shop manual, section 11: general troubleshooting, ECM/PCM inputs and outputs at connector E.
- https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_3k.htm - same manual, PGM-FI: MIL circuit and "DLC Circuit Troubleshooting" (pages 11-114/115; in the last part of the page).
- https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2ak.htm - same manual, section 77 supplement: DLC pin box / SCS short connector part numbers, model codes, E29 voltage.
- https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group2aa.htm - same manual, ABS: tools for EU / except-EU models, SCS method, DTC clearing.
- https://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group31c.htm - same manual, multiplex control system: MPCS service connector, ECM E24 wire.
- http://www.hondafitjazz.com/manual/a00/html/97/saa2e97a20326919461faat00.html - Fit/Jazz manual, "DLC Circuit Troubleshooting" (8.5 V test).
- http://www.hondafitjazz.com/manual/a00/html/f6/saa4ef6a20300000000caat10.html - Fit manual, ECM/PCM connector E table including WEN (E30) and SCS (E29).
- https://www.hondafitjazz.com/manual/A00/HTML/97/SAA2E97A20300000000CAAT10.HTML - Fit/Jazz manual, another connector E table.
- https://hondziarze.duckdns.org/jazz/A00/HTML/00/SAA2E00K79100000000BBAT00.HTML - Jazz manual, "How to Troubleshoot the SRS" (PGM Tester, DLC 4-9, MES connector).

Community and vendor references fetched directly:

- https://blog.rywire.com/k-series-dlc-connector-pinout/ - K-series DLC pinout with wire colours and ECM terminals (pins 4, 5, 7, 9, 12, 14, 16).
- https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml - generic Honda OBD-II pinout and user-reported protocol list.
- https://www.klavkarr.com/compatible/honda-civicvii-21000116 - outilsobdfacile/klavkarr tested-vehicle table for the Civic VII with detected protocol.
- https://elm327rus.ru/sovmestimost/ - Russian ELM327 compatibility list (US Civic 2001, JDM Fit and Accord entries).
- https://github.com/meatpiHQ/wican-fw/issues/1035 - raw ISO 9141-2 frames from a 2007 EU Accord K20A6.
- https://github.com/iwanders/OBD9141 - Arduino ISO 9141-2 / KWP library: transceivers used, request rates.
- https://github.com/OBDb/Honda-Civic - OBDb Civic signal set and per-model-year test files.
- https://www.hondash.net/2018/03/dlc-interface-power-management.html , https://www.hondash.net/p/faq.html , https://www.hondash.net/2018/07/how-to-check-if-hondash-will-work-with.html - Hondash: native Honda protocol versus ISO mode, which markets and years use which connector.
- https://github.com/kerpz/ArduinoHondaOBD , https://github.com/mr-sneezy/ArduinoHondaOBD1_to_OBD2_BT - projects for the older proprietary Honda protocol on the 3-pin DLC ("Honda ECU's before 2002 in countries other than the USA").
- https://alflash.com.ua/Honda/ - Honda service-connector notes and tool part numbers.
- https://en.wikipedia.org/wiki/On-board_diagnostics - EOBD dates, protocol overview.
- https://ganchi.rdy.jp/AppObdInfoSan/qa.html - Japanese OBD app FAQ on Japan-specific protocols.

Seen only through search-engine summaries (site refused the fetch tool):

- https://honda-tech.com/forums/honda-civic-2001-2005-78/honda-civic-2002-strange-obdii-pinout-3366115/ - 2002 Civic with cavities 4, 5, 7, 14, 16 and DMM readings.
- https://honda-tech.com/forums/honda-civic-2001-2005-78/new-guy-dlc-pin-7-communication-issues-3093207/ - 2002 Civic EX, pin 7 at 11.9 V, K-line traced to ABS plug and ECU.
- https://honda-tech.com/forums/honda-civic-2001-2005-78/02-civic-service-check-connector-2386746/ , https://honda-tech.com/forums/honda-civic-2001-2005-78/get-engine-code-without-scanner-2900091/ , https://honda-tech.com/forums/honda-civic-2001-2005-78/abs-computer-jumper-location-2117354/ - SCS via DLC pins 4 and 9 on 2001-2005 Civics.
- https://www.civicforums.com/forums/109-7th-gen-do-yourself-articles/307197-classic-diy-2001-2003-civic-ecu-wiring.html - 2001-2003 Civic ECM connector pinout (E23 lt blu K-line, E29 brn SCS, E24 yel multiplex, E27 red/blu immobilizer code).
- https://www.civicforums.com/forums/36-mechanical-problems-vehicle-issues-fix-forum/352527-possible-pull-check-engine-light-codes-no-scanner.html - "The pin trick 4 & 9 does work on Civic".
- https://www.justanswer.com/honda/1bezu-no-communication-data-link.html , https://www.justanswer.com/honda/3ha2t-2001-honda-civic-sedan-ex-always-ran-great-until.html - quotations of the "2001 CIVIC - DLC Circuit Troubleshooting" procedure (terminal 5 to E3, terminal 7 to E23).
- https://www.justanswer.com/honda/sg6sz-i-m-working-2002-honda-civic-si-2-0l-k20a3.html - 2002 Civic Si K-line fault case.
- https://www.manualslib.com/manual/804994/Honda-Ep3.html?page=334 (DLC circuit troubleshooting), ?page=247 (ECM connector E), ?page=542 (EPS DTC retrieval) - Honda EP3 service manual.
- https://www.d-series.org/threads/abs-and-srs-faults.172153/page-2 - forum copy of the pin 12 / pin 14 description (bot paywall, not opened).
