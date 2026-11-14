# 07 - Firmware and software stack: options and prior art

Research dimension 7 of 9 for the Honda Civic 2001-2005 (7th generation) OBD2 device project. Date: 2026-11-14.
Web research only: nothing was cloned, installed or executed. Source code was read through GitHub pages and raw files; three PDFs (ELM327 datasheet, OBDLink command sheet, SAE J2534-1) were read page by page.

Confidence tags: **HIGH** = primary source, or two independent sources agree. **MEDIUM** = one credible source. **LOW** = single forum post, my own inference or arithmetic, or conflicting reports.

Method limit: the WebSearch budget shared by all researchers of this run ran out after about 25 of my searches. Everything after that was fetched from URLs already known. Items that stayed unverified because of this are marked and repeated under "Open questions".

## Summary

1. No open-source project was found that speaks Honda's dealer-level K-line protocol to a 2001-2005 Civic. The two closest pieces of prior art are kerpz/ArduinoHondaOBD (Honda 9600-baud DLC protocol, README says ECUs "before 2002", no licence) and HondaECU + eculib (GPL-3.0, Python over an FTDI cable, Honda/Keihin **motorcycle** ECUs, 10400 baud, 70 ms wake-up pulse, full flash read/write). Which framing the 7th-generation Civic uses is not settled by this dimension.
2. For legislated OBD (ISO 9141-2, ISO 14230-4) on a microcontroller, iwanders/OBD9141 (MIT) is the best reusable base: 5-baud init, fast init, echo read-back, DTC read/clear, and an ECU simulator class. The other K-line libraries (aster94, muki01, freediag) are GPL-3.0.
3. A genuine ELM327 (v1.4b/v2.0 datasheet) offers 9600 baud (`AT IB 96`), a custom 5-baud address (`AT IIA`), custom headers and keep-alives, and programmable fast-init pulse widths (PP 1A/1B, 2.5 ms steps). It still appends its own sum checksum and wraps fast init around its own start message, so it cannot cleanly produce Honda frames (length byte plus two's-complement checksum).
4. STN chips (OBDLink) come much closer: `STP 21` (ISO 9141, no header, no autoinit), `STPBR`/`STIBR` (free baud rate) and `STIMCS 1` (no checksum added or checked). No command for a custom wake-up pulse was found in the documents read. No verified Honda recipe exists for either chip family.
5. SAE J2534-1 (04.04) describes exactly the K-line primitives a firmware needs: free baud rate, parity, checksum off, P1/P3/P4, W0-W5, TIDLE/TINIL/TWUP in 1 ms steps, `FIVE_BAUD_INIT`, and `FAST_INIT` with any first message or none. Honda's own interfaces add J2534-2 features (`HONDA_DIAGH_PS` = 0x800B; the Honda MVCI lists "K-Line, Honda Diag H, Honda Diag X").
6. No complete open-source J2534 device with K-line support was found (Macchina-J2534: K-line "WIP"; panda: ISO 9141 unsupported; taskmanager-j2534: CAN only, not open source). Logging shim DLLs do exist (joeyoravec/j2534-logger GPL-3.0, MEAT-Inc/Fulcrum) and are the cheapest way to learn what HDS asks a pass-through device to do.
7. Data sources are thin. OBDb/Honda-Civic has nothing usable for 2001-2005. Generic DTC lists exist under MIT. No Torque or ELM327 custom-init recipe for pre-CAN Honda was found; OBD Fusion sells no Honda enhanced add-on.
8. Tooling gaps: sigrok has no K-line/KWP2000 decoder and pcap has no K-line link type. The workable path is UART decode, then pcap with `LINKTYPE_USER0` plus a Lua dissector (or the MIT-licensed jared52005/Monitor approach), which TShark and therefore REA can read.
9. Test tooling exists for the legislated protocols (OBD9141 simulator, freediag CARSIM files, Ircama ELM327-emulator with a K-line mode, commercial ECUsim 5100). A Honda ECM emulator has to be built from captured request/response pairs.
10. Recommended stack: firmware owns all byte and pulse timing behind J2534-shaped primitives; the host (C#, the owner's language) owns protocol logic behind one channel interface with two back-ends (our device, and any commercial J2534 interface through J2534-Sharp, MIT); binary framing over USB CDC first; an ELM327 personality and a J2534 DLL are optional later layers.

## Findings

### 1. Open-source firmware and libraries for K-line protocols

#### Overview

| Project | Protocols | Target | Licence | Reuse verdict |
|---|---|---|---|---|
| [iwanders/OBD9141](https://github.com/iwanders/OBD9141) | ISO 9141-2, ISO 14230-2 slow and fast init | Arduino, Teensy 3.x/LC, ESP32 | MIT | Reuse or port directly (init, echo, timing, DTC, simulator) |
| [aster94/Keyword-Protocol-2000](https://github.com/aster94/Keyword-Protocol-2000) | KWP2000 fast init (Suzuki, Kawasaki tested; Yamaha, Honda untested) | Arduino framework | GPL-3.0 | Reference only unless the project is GPL |
| [muki01/OBD2_KLine_Library](https://github.com/muki01/OBD2_KLine_Library) | ISO 9141-2, ISO 14230-4, VAG KW1281, BMW DS2, Opel KW82 | Arduino, ESP32 | GPL-3.0 (commercial licence offered) | Reference only |
| [PowerBroker2/ELMduino](https://github.com/PowerBroker2/ELMduino) | none itself; client for an ELM327 | Arduino, ESP32 | MIT | Not a K-line driver; useful only as an ELM327 client example |
| [kerpz/ArduinoHondaOBD](https://github.com/kerpz/ArduinoHondaOBD) | Honda proprietary 9600-baud DLC protocol, ELM327 emulation towards Torque | Arduino Uno/Nano | none stated | Protocol facts only; code cannot be copied |
| HondaECU ([fork](https://github.com/Adeen117/HondaECU)) + [eculib](https://github.com/Pongmi04/eculib) | Honda/Keihin motorcycle K-line: diagnostics and flashing | Python on a PC, FTDI cable | GPL-3.0 | Best reference for Honda framing, wake-up, flash sequence |
| [AutotronicCommunity/Honda_Keihin_KLine_Protocol](https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol) | Documentation of the same motorcycle protocol | README + capture text | none stated | Protocol facts |
| sabsteef Honda_K-line ([fork](https://github.com/K9CTec/Honda_K-line)) | Honda K-line (Arduino sketch + Python panel) | Arduino + Python | BSD 2-Clause | Small; permissive; contents not inspected |
| [fenugrec/freediag](https://github.com/fenugrec/freediag) | L2: ISO 9141, ISO 14230, SAE J1850, VAG, MB1, D2, raw, CAN; L3: J1979, KWP | C, Linux and Windows, dumb serial cables | GPL-3.0 | Use as a ready PC-side tool with a dumb K-line cable; CARSIM test files |
| [HonOBDapt](https://sourceforge.net/projects/honobdapt/) | "Adapts your 92-95 Honda diagnostic port to OBD II" | C++ | GPLv3 | Older generation; not inspected further |
| [brendan-w/python-OBD](https://github.com/brendan-w/python-OBD) | ELM327 client | Python | GPL-2.0 | Generic OBD only, through an ELM327 |
| [rnd-ash/ecu_diagnostics](https://github.com/rnd-ash/ecu_diagnostics) | UDS, KWP2000, OBD2 over CAN/ISO-TP hardware APIs | Rust | GPL-3.0 | No K-line physical layer listed |

Confidence for the licence and scope columns: HIGH where the repository page or file header was read (OBD9141, aster94, muki01, ELMduino, HondaECU fork, freediag, python-OBD, ecu_diagnostics, HonOBDapt), MEDIUM for the sabsteef fork (licence text seen on a fork page only).

#### iwanders/OBD9141 (MIT)

- Supports ISO 9141-2 and ISO 14230-2 with slow and fast init; boards: Teensy 3.x/LC, Arduino (AltSoftSerial), ESP32; transceivers named: MC33290, SN65HVDA100, SN65HVDA195. HIGH ([README](https://github.com/iwanders/OBD9141)).
- 5-baud init is bit-banged on the TX pin, address 0x33 sent as level pairs: `kline(false); delay(200);` start bit, then `delay(400)` for each pair of equal bits, `kline(true); delay(200);` stop bit. It then expects 0x55, reads key bytes v1 and v2, requires v1 == v2 for ISO 9141-2, waits `delay(30)`, writes `~v2`, and expects 0xCC. HIGH ([OBD9141.cpp](https://raw.githubusercontent.com/iwanders/OBD9141/master/src/OBD9141.cpp)).
- Fast init: `kline(false); delay(25);` then `kline(true); delay(25);` then the message `{0xC1, 0x33, 0xF1, 0x81}` plus checksum; success if the reply's fourth byte is 0xC1. HIGH (same file).
- KWP slow init: same 5-baud routine without the v1 == v2 check (code comment: "KWP slow init, v1 == 233, v2 = 143"). HIGH (same file).
- Echo: each request byte is written, then `readBytes(tmp, request_len+1)` reads the echo back and compares it byte by byte with what was sent. HIGH (same file).
- Request headers: ISO 9141-2 `{0x68, 0x6A, 0xF1, mode, pid}`; KWP first byte `(0b11<<6) | length`, second byte 0x33. Checksum is the 8-bit sum. HIGH (same file).
- Timing constants ([OBD9141.h](https://raw.githubusercontent.com/iwanders/OBD9141/master/src/OBD9141.h)), HIGH:

| Constant | Value | Meaning in the source comments |
|---|---|---|
| `OBD9141_KLINE_BAUD` | 10400 | "as per spec" |
| `OBD9141_INTERSYMBOL_WAIT` | 5 ms | delay between written bytes |
| `OBD9141_REQUEST_ECHO_MS_PER_BYTE` | 3 ms | echo wait per byte |
| `OBD9141_WAIT_FOR_ECHO_TIMEOUT` | 5 ms | added to the echo timeout |
| `OBD9141_REQUEST_ANSWER_MS_PER_BYTE` | 3 ms | allowed per answer byte |
| `OBD9141_WAIT_FOR_REQUEST_ANSWER_TIMEOUT` | 30 + 20 ms | includes the 30 ms between request and answer |
| `OBD9141_INIT_IDLE_BUS_BEFORE` | 3000 ms | bus idle before init |
| `OBD9141_INIT_POST_INIT_DELAY` | 50 ms | not in the spec; settle time after init |
| `OBD9141_BUFFER_SIZE` | 16 | must be raised for long DTC answers |

- API includes `readTroubleCodes()`, `readPendingTroubleCodes()`, `clearTroubleCodes()`, `requestKWP()`, and a second class that simulates ECU responses (a `simulator` example). HIGH (README and header).

#### aster94/Keyword-Protocol-2000 (GPL-3.0)

- KWP2000 for motorcycles; "works on Suzuki and Kawasaki", Yamaha and Honda untested; transceivers suggested: L9637, MC33660, MC33199; includes a Python ECU emulator in `extras`. HIGH ([README](https://github.com/aster94/Keyword-Protocol-2000)).
- Fast init is a non-blocking state machine on the TX pin: `ISO_T_IDLE` 1000 ms high, `ISO_T_INIL` 25 ms low, `ISO_T_WUP` 50 ms total, then `begin(ISO_BAUDRATE)` with 10400. Echo bytes are read during the send loop and compared (`if (echo != request[i] && echo != 0)` sets an echo error). Inter-byte wait uses `ISO_T_P4_MIN`; checksum is the 8-bit sum. HIGH ([KWP2000.cpp](https://raw.githubusercontent.com/aster94/Keyword-Protocol-2000/master/src/KWP2000.cpp), [ISO.h](https://raw.githubusercontent.com/aster94/Keyword-Protocol-2000/master/src/ISO.h)).

#### muki01/OBD2_KLine_Library (GPL-3.0, commercial licence offered)

- ISO 9141-2 and ISO 14230-4 plus VAG KW1281, BMW DS2, Opel KW82; init methods 5-baud, fast, "ping" or none; `setInitAddress()` changes the 5-baud address; raw access through `writeData`, `writeRawData`, `sendBytes`; live data, freeze frame, DTC read/clear, VIN; transceivers L9637D, MC33290, Si9241, SN65HVDA195, plus transistor and LM393 circuits; echo removal is automatic (method not documented). No Honda in the documentation; "ECU definition files are licensed separately". HIGH ([README](https://github.com/muki01/OBD2_KLine_Library)).
- muki01/OBD2_K-line_Reader is the companion reader application from the same author. LOW (seen only as a search result; not opened).

#### PowerBroker2/ELMduino (MIT)

- A client library that lets an Arduino or ESP32 talk to an ELM327 adapter (Serial, ESP32 BluetoothSerial) with a non-blocking query model. It contains no K-line driver; the ELM327 does the bus work. HIGH ([README](https://github.com/PowerBroker2/ELMduino)).

#### kerpz/ArduinoHondaOBD (no licence)

- Purpose: reads the Honda OBD/DLC protocol and "translates it to ELM327" so Torque can read it over an HC-05 Bluetooth module, or shows it on an LCD. README: supported vehicles are Honda ECUs from before 2002; tested on a P2T OBD2 stock ECU and a P30 OBD1 chipped ECU; K-line from the Honda 3-pin DLC goes to Arduino pin 12. MEDIUM ([README](https://raw.githubusercontent.com/kerpz/ArduinoHondaOBD/master/README.md), read through a summariser).
- No licence file and no licence statement in the file headers (author Philip Bordado). HIGH (file tree and headers).
- Serial: `SoftwareSerialWithHalfDuplex dlcSerial(12, 12, false, false);` and `dlcSerial.begin(9600);`. HIGH ([hobd_uni2.ino](https://raw.githubusercontent.com/kerpz/ArduinoHondaOBD/master/hobd_uni2/hobd_uni2.ino), [hobd_elm.ino](https://raw.githubusercontent.com/kerpz/ArduinoHondaOBD/master/hobd_elm/hobd_elm.ino); both files agree).
- Init: no wake-up pulse and no pin toggling. `dlcInit()` writes the 11 bytes `68 6A F5 AF BF B3 B2 C1 DB B3 E9`, then `delay(300)`. HIGH (both files agree).
- Observation: the last byte of that init message equals the 8-bit sum of the ten bytes before it (0x6E9, low byte 0xE9). The message is therefore framed like ISO 9141-2 (three header bytes `68 6A F5`, sum checksum), unlike the commands that follow. LOW (my arithmetic on the bytes above).
- Command: `dlcCommand(cmd, num, loc, len)` sends `cmd num loc len crc` with `crc = 0xFF - (cmd + num + loc + len - 0x01)`; the reply is `00 len+3 data... crc`; timeout 200 ms (hobd_uni2) or 250 ms (hobd_elm); hobd_uni2 verifies the reply checksum the same way. HIGH (both files).
- Read memory: `dlcCommand(0x20, 0x05, loc, len)`. DTC scan: `dlcCommand(0x20, 0x05, 0x40, 0x10)` with nibble-coded codes. Reset/clear: `dlcCommand(0x21, 0x04, 0x01, 0x00)`. HIGH (hobd_uni2.ino).
- ECU variants: `obd_select` 1 = OBD1, 2 = OBD2; RPM is `1875000 / (value + 1)` for OBD1 and `value / 4` for OBD2. HIGH (hobd_uni2.ino).
- Echo: none in the sketch. The half-duplex library keeps the pin as an input with pull-up and "temporarily switches to an output until the byte is sent, then flips back to input", so the sketch never sees its own bytes. HIGH ([SoftwareSerialWithHalfDuplex](https://github.com/nickstedman/SoftwareSerialWithHalfDuplex), LGPL-2.1 or later).
- ELM327 emulation: handles `ATD, ATI, ATZ, ATE, ATL, ATM, ATS, ATH, ATSP`, maps mode 01 PIDs to ECU memory offsets (for example 010C -> offset 0x00, 2 bytes; 0105 -> 0x10), maps mode 04 to the reset command, and exposes raw Honda reads as custom PIDs. HIGH (hobd_elm.ino). A Torque custom PID file is shipped (`torque/HOBD.csv`). HIGH (file tree).

#### HondaECU and eculib (GPL-3.0, motorcycles)

- The lead "RyanHope/HondaECU" is partly wrong today: that repository returns 404. Forks remain (Adeen117/HondaECU with 756 commits and the full source; roccofz1/HondaECU). One fork's README says "HondaECU development has been taken over by MCU Innovations, Inc." HIGH (fetch result, [fork](https://github.com/Adeen117/HondaECU), [fork](https://github.com/ekomook/HondaECU)).
- Scope: "A diagnostic and ECU flashing tool for Honda motorcycles"; the repository carries Keihin motorcycle ECU images (`38770-xxx.bin`, 49 kB to 1 MB) and TunerPro `.xdf` files. It is not a car tool. HIGH (fork page and file tree).
- Hardware and drivers: FTDI USB-to-serial converter plus a K-line level converter (optocoupler design recommended); on Windows the libusbK driver installed with Zadig; a bench harness needs a 2.5 A supply ("a 2.0 A supply powers the ECU, but transfers are slow and produce CRC errors"). HIGH (fork README).
- Link: `KlineAdapter(Device)` from pylibftdi, default `baudrate=10400`, `ftdi_set_line_property(8, 1, 0)`. HIGH ([base.py](https://raw.githubusercontent.com/Pongmi04/eculib/master/eculib/base.py)).
- Wake-up: the FTDI chip is switched to bit-bang mode, TX is driven low for 70 ms, then high, the chip returns to UART mode and the code waits 130 ms. HIGH for the 70 ms low time (corroborated below), MEDIUM for the 130 ms wait ([honda.py](https://raw.githubusercontent.com/Pongmi04/eculib/master/eculib/honda.py), read through a summariser).
- Framing: `header bytes + [total length] + data + [checksum]`; checksum is `((sum ^ 0xFF) + 1) & 0xFF` (two's complement, so the 8-bit sum of a whole frame is zero). HIGH (honda.py).
- Echo: after writing a frame the code reads back `len(msg)` bytes and discards them. Reply reading: header and length byte first, then the remaining bytes given by the length byte. HIGH (honda.py).
- Commands seen: ping `[0xFE] [0x72]`; ECM id `[0x72] [0x71, 0x00]`; table read `[0x72] [0x71, table]`; faults `[0x72] [0x74, i]` (current) and `[0x72] [0x73, i]` (stored); clear `[0x72] [0x60, 0x03]`; flash control with headers 0x7B (recover), 0x7D (write init) and 0x7E (erase, write block `[0x01, 0x06]`, end `[0x01, 0x07]`); a password message with header 0x27 and `0xE0` plus 7 bytes. Flash read chunk 12 bytes, write chunk 128 (fallback 64). HIGH ([kline.py](https://raw.githubusercontent.com/Adeen117/HondaECU/master/src/threads/kline.py), honda.py).
- Independent documentation of the same protocol: 10400 baud; pull TX low 70 ms, high 120 ms; send `FE 04 72 8C`, ECU replies `0E 04 72 7C`; send `72 05 00 F0 99`, reply `02 04 00 FA`; ECM id request `72 05 71 00 18`; table 11 `72 05 71 11 07`; table 17 `72 05 71 17 01`; request format `72 AA BB CC CS`. HIGH for the motorcycle protocol ([AutotronicCommunity README](https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol) agrees with eculib; I also checked that every quoted frame sums to zero modulo 256).
- Whether Honda cars of 2001-2005 use this same framing on the DLC K-line is not shown by any source in this dimension. LOW.

#### freediag (GPL-3.0)

- "Free diagnostic software for OBD-II compliant motor vehicles": a command-line scan tool plus `libdiag`; Linux and Windows. HIGH ([repository](https://github.com/fenugrec/freediag)).
- Source tree: L0 drivers `diag_l0_dumb.c`, `diag_l0_dumbtest.c`, `diag_l0_br.c`, `diag_l0_elm.c`, `diag_l0_me.c`, `diag_l0_sim.c`; L2 `iso9141`, `iso14230`, `saej1850`, `vag`, `mb1`, `d2`, `raw`, `can`; L3 `saej1979`, `iso14230`, `vag`; L7 `d2`, `kwp71`. HIGH ([tree listing](https://api.github.com/repos/fenugrec/freediag/git/trees/HEAD?recursive=1)).
- Dumb-cable options (added together, `set dumbopts`): 1 USE_LLINE, 2 CLEAR_DTR, 4 SET_RTS, 8 MAN_BREAK ("use to force manual 5bps transmission. Essential for most USB-serial bridges"), 16 LLINE_INV, 32 FAST_BREAK (instead of a 25 ms break and 25 ms wait, "send 0x00 at 360bps (==25ms) and wait a total of 50ms"), 64 BLOCKDUPLEX ("message-based half duplex removal (if P4==0)"). HIGH ([dumb_interfaces.txt](https://raw.githubusercontent.com/fenugrec/freediag/master/doc/dumb_interfaces.txt)).
- Settings `initmode`, `testerid`, `destaddr`, `speed` exist, so non-standard addresses and baud rates can be tried from the CLI. HIGH ([Scantool manual](https://freediag.sourceforge.io/Scantool-Manual.html)).

#### What to take from these projects

| Technique | Proven implementation | Note for our firmware |
|---|---|---|
| 5-baud address | GPIO bit-bang with 200 ms bit time (OBD9141); "manual break" on USB-serial bridges (freediag) | Use a hardware timer for edges; do not rely on a 5-baud UART mode |
| Fast init 25 ms low + 25 ms high | GPIO (OBD9141, aster94); `0x00` at 360 bps (freediag) | Make low and high times parameters (J2534 TINIL/TWUP) |
| Honda motorcycle wake-up 70 ms low | FTDI bit-bang (eculib) | Same pulse primitive with other numbers |
| Echo on the single wire | read back and compare each byte (OBD9141, aster94); read back and discard (eculib); pin turned to input while idle (kerpz) | Read back and compare: it also detects a stuck or contended bus |
| Inter-byte gap P4 | 5 ms (OBD9141) | Parameter; eculib sends frames without gaps and the motorcycle ECUs accept it |
| Reply framing | fixed expected length (OBD9141), length byte (eculib, kerpz), inter-byte timeout (freediag) | Support both length-byte and timeout framing |

### 2. ELM327 and STN chips as early probes

#### ELM327 commands that matter for K-line work

Source for the version column: Elm Electronics, "ELM327 AT Commands", October 2010 ([PDF](https://cdn.sparkfun.com/assets/4/e/5/0/2/ELM327_AT_Commands.pdf)). Source for behaviour: datasheet ELM327DSI, 82 pages, identification string 'ELM327 v2.0' ([PDF](https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf), pages 16-18, 34, 38-39, 61). All rows HIGH.

| Command | Since | What the datasheet says |
|---|---|---|
| `AT IB 10` / `IB 48` / `IB 96` | 1.0 / 1.4 / 1.0 | ISO baud rate 10400 / 4800 / 9600. IB 96 applies to "protocols 3, 4 and 5 ... while relaxing some of the requirements for the initiation byte transfers". No other rates exist. |
| `AT IIA hh` | 1.2 | 5-baud init address for protocols 3 or 4; "the full eight bit value is used exactly as provided"; default $33. |
| `AT KW0` / `KW1`, `AT KW` | 1.2, 1.3 | Key word checking off/on; display the key words. |
| `AT SI`, `AT FI` | 1.4 | Perform a slow or fast initiation now. FI requires protocol 5. |
| `AT BI` | 1.0 | Bypass the initialisation sequence. |
| `AT SH xx yy zz` | 1.0 | Three header bytes. For ISO 14230 the ELM327 "always determines the number of bytes ... and inserts that length for you"; if the length bits are 0 it creates a fourth header byte. |
| `AT WM [1-6 bytes]` | 1.2 | Wake-up (keep-alive) message; "You do not provide the checksum byte - it is automatically added for you". Defaults: `68 6A F1 01 00` (ISO 9141) and `C1 33 F1 3E` (KWP). |
| `AT SW hh` | 1.0 | Wake-up interval, hh x 20 ms; default about 3 s. |
| `AT ST hh`, `AT AT0/1/2` | 1.0, 1.2 | Response timeout hh x 4 ms; adaptive timing. |
| `AT TA hh`, `AT SR hh`, `AT RA hh` | 1.4, 1.2, 1.3 | Tester address; receive address filter. |
| `AT AL` / `NL` | 1.0 | Allow long (more than 7 byte) messages / normal length. |

The datasheet's own trick for a fast init followed by ISO 9141 framing is the sequence `AT SP 5`, `AT FI`, `AT SP 3`, `AT BI`. HIGH (datasheet page 17).

Programmable parameters for ISO/KWP timing (datasheet page 61), all HIGH:

| PP | Meaning | Scale | Default |
|---|---|---|---|
| 14 | Final stop bit width (P4 inter-byte time) | 98 + value x 64 us | 50 (5.2 ms) |
| 15 | Maximum inter-byte time P1 and minimum inter-message time P2 | value x 2.112 ms | 0A (21 ms) |
| 16 | Default ISO/KWP baud rate | 00 = 9600, FF = 10400 | FF |
| 17 | Wake-up message rate | value x 20.48 ms | 92 (2.99 s) |
| 1A | Protocol 5 fast initiation active time (TiniL) | value x 2.5 ms | 0A (25 ms) |
| 1B | Protocol 5 fast initiation passive time (TiniH) | value x 2.5 ms | 0A (25 ms) |
| 1C | Outputs used for initiation (bit 0 K line, bit 1 L line) | bit field | 03 |
| 1D | P3 time before sending requests | (value - 0.5) x 4.096 ms | 0F (59 ms) |

#### STN (OBDLink) commands

Sources: "OBDLink MX Protocol Commands", 29 December 2011 ([PDF](https://www.scantool.net/downloads/118/mx_protocol_commands.pdf)), and the STN1100 Family Reference and Programming Manual revision A ([page 12](https://www.manualsdir.com/manuals/710473/scantool-stn1100-frpm.html?page=12)). These are old revisions; newer manuals may add commands.

| Command | Meaning | Confidence |
|---|---|---|
| `STP 21` | ISO 9141 (no header, no autoinit) | HIGH |
| `STP 22` | ISO 9141-2 (5 baud autoinit) | HIGH |
| `STP 23` / `24` / `25` | ISO 14230 (no autoinit) / (5 baud autoinit) / (fast autoinit) | HIGH |
| note | "Presets without autoinit do not send automatic keep-alives." | HIGH |
| `STPBR baud`, `STPBRR` | Set / report the OBD protocol baud rate; an unsupported rate gives the closest possible | HIGH |
| `STIBR baud` | Set the ISO baud rate in bps | MEDIUM (range printed on that page looks like a typo) |
| `STIMCS 0/1` | Manual checksum: when on, no checksum byte is appended to transmitted messages and none is verified on reception | MEDIUM |
| `STIP1X ms` | Maximum inter-byte time when receiving (P1max), default 20 ms | MEDIUM |
| `STIP4 ms` | Inter-byte time when transmitting (P4), default 5 ms | MEDIUM |
| `STIAT 0/1` | Adaptive P1max timing off/on | MEDIUM |

#### Can these chips speak Honda's proprietary protocol or issue its wake-up pulse?

- 9600 baud is available on a genuine ELM327 (`AT IB 96`) and any baud rate on an STN chip (`STPBR`). HIGH (documents above).
- A genuine ELM327 always adds its own checksum to K-line messages. The datasheet states this explicitly for wake-up messages (HIGH); for ordinary requests it is the chip's well-known behaviour, which I did not re-read on a specific page this round (MEDIUM). Both Honda framings found in this research use a checksum that makes the 8-bit sum of the whole frame zero (kerpz: `0xFF - (sum - 1)`; eculib: two's complement). HIGH (source code).
- Consequence: if the user supplies the Honda checksum as the last data byte, the checksum appended by the ELM327 is always 0x00, so the ECU would see a correct Honda frame followed by one stray 0x00 byte. Whether a Honda ECU tolerates that is unknown. LOW (my arithmetic; needs a bench test).
- The ELM327 fast-init pulse can be stretched: PP 1A = 1C gives 70 ms low, PP 1B = 30 gives 120 ms high (34 gives 130 ms). The pulse is still followed by the chip's own start message and a wait for a KWP-style answer, and `AT FI` works only in protocol 5. So a pulse of Honda-motorcycle shape is possible in principle, but a clean Honda session is not. LOW (derived from the PP table; untested).
- An STN chip in `STP 21` with `STIMCS 1` sends exactly the bytes given, at any baud rate, without init and without keep-alives. This is enough for a protocol that needs no wake-up pulse (the kerpz style). No STN command for a custom wake-up pulse was found in the two documents read. MEDIUM for the capability, LOW for "it will work on a Honda" (untested).
- No published Honda recipe was found. The Torque wiki "Custom Init" page lists only Toyota and Nissan Japanese-market strings (for example Toyota Celica ZZT230: `ATIB 96`, `ATIIA 13`, `ATSH8113F1`, `ATSP A4`, `ATSW00`) and has no Honda entry. HIGH ([Torque wiki](https://wiki.torque-bhp.com/view/Custom_Init)). A forum suggestion to use `ATIB 96 ATIIA 13 ATSH8213F0 ATSP4` on a Honda is the Toyota recipe reused, described by its own poster as not guaranteed. LOW ([PakWheels](https://www.pakwheels.com/forums/t/help-required-regarding-elm-327/242479), search snippet only).
- A Russian write-up confirms the Toyota case in detail (2ZZ-GE ECU, ISO 9141-2 style, 9600 baud, `ATIIA13`), showing that `IB 96` + `IIA` + `SH` is a working pattern for non-standard K-line ECUs that keep the ISO frame format. MEDIUM ([drive2.ru](https://www.drive2.ru/b/535600977143662337/)).

Hypothetical probe sequences to try on the bench (not verified anywhere; placeholders instead of invented bytes):

| Goal | Adapter | Sequence | Expected weakness |
|---|---|---|---|
| Honda frame at 9600 baud, no pulse | genuine ELM327 | `AT SP 3`, `AT IB 96`, `AT BI`, `AT H1`, `AT SH <b1> <b2> <b3>`, then `<remaining bytes incl. Honda checksum>` | trailing 0x00 checksum byte; reply flagged as a checksum error |
| Same | STN | `STP 21`, `STPBR 9600`, `STIMCS 1`, then the whole frame as hex | none known; no keep-alive (host must send it) |
| Wake-up pulse 70 ms low | genuine ELM327 | PP 1A and 1B changed, `AT SP 5`, `AT FI`, `AT SP 3`, `AT BI` | chip sends its own start message after the pulse |

Clone ELM327 adapters often implement only part of the command set; that is common knowledge and was not verified this round. LOW.

Alternative early probe with full control: an FTDI-based "dumb" K-line cable driven from Python (the HondaECU method) or from freediag's dumb driver. HondaECU shows that host-side timing through an FTDI chip is good enough for a 70 ms wake-up pulse, diagnostics and flashing of Honda motorcycle ECUs. HIGH for motorcycles (source code), MEDIUM as a prediction for the Civic.

#### Is an ELM327-compatible command set on our device worthwhile?

- Precedent: kerpz does exactly this, bridging Honda proprietary data to Torque through emulated ELM327 commands and custom PIDs. HIGH (source code).
- For the generic part alone the value is small: a 2001-2005 Civic already answers legislated OBD over ISO 9141-2 to any ELM327 (OBDb test cases for 2004 and 2005 Civic carry `686A...|p=9141-2` entries). MEDIUM ([2005 test case](https://raw.githubusercontent.com/OBDb/Honda-Civic/main/tests/test_cases/2005/command_support.yaml)).
- The value lies in exposing Honda-only data as custom PIDs to existing dashboards. Cost: apps probe many AT commands and expect exact reply strings; phone apps also expect a specific Bluetooth profile. Recommendation: optional late personality, not the core protocol. MEDIUM (judgement).
- OBD Fusion lists enhanced add-ons for FCA, Ford, Mazda, Toyota, Nissan and Mitsubishi, none for Honda, so there is no commercial phone-app implementation of Honda enhanced diagnostics over ELM/STN to learn from there. MEDIUM ([OBD Fusion](https://www.obdsoftware.net/software/obdfusion)).

### 3. SAE J2534 pass-through as an architecture

#### What J2534-1 (04.04) requires for K-line

Source: SAE J2534-1, revised December 2004, read directly ([copy](https://netcult.ch/elmue/HUD%20ECU%20Hacker/J2534%20PassThrough.pdf), pages 9-10, 18-19, 43-48). All HIGH.

- API: 14 functions - PassThruOpen, Close, Connect, Disconnect, ReadMsgs, WriteMsgs, StartPeriodicMsg, StopPeriodicMsg, StartMsgFilter, StopMsgFilter, SetProgrammingVoltage, ReadVersion, GetLastError, Ioctl.
- PC link: "RS-232, USB, Ethernet, IEEE1394, Bluetooth or any other connection that allows the pass-thru device to meet all other requirements of this document, including timing requirements."
- ISO 9141 (section 6.5.1): baud rates 10400 (within 0.5 %), 10000 (1 %), and within 2 %: 4800, 9600, 9615, 9800, 10870, 11905, 12500, 13158, 13889, 14706, 15625, 19200, plus any other rate the device can hit within 2 %; "the baud rate shall be set by the application"; odd, even or no parity with 7 or 8 data bits; ability to disable checksum verification "to allow vehicle manufacturer specific error detection"; both 5-baud and fast initialisation; the interface "shall not adjust timer parameters based on keyword values".
- ISO 14230 (6.5.2): "The pass-thru interface will not automatically handle tester present messages."
- Vehicle side (6.4): SAE J1962 connector, operate from 8.0 to 18.0 V, survive 24 V and reverse 24 V for 10 minutes; K-line sink current up to 100 mA.

| Item | Value | Notes |
|---|---|---|
| Protocol ID ISO9141 | 0x03 | "ISO 9141 and ISO 9141-2" |
| Protocol ID ISO14230 | 0x04 | |
| Flag ISO9141_NO_CHECKSUM | bit 9 (0x200) | "the entire message will be treated as data by the interface" |
| Flag ISO9141_K_LINE_ONLY | bit 12 (0x1000) | K-line only for the init address |
| DATA_RATE | 0x01 | 5 to 500000 |
| LOOPBACK | 0x03 | echo transmitted messages into the receive queue |
| P1_MAX | 0x07 | default 40 = 20 ms (0.5 ms per bit) |
| P3_MIN | 0x0A | default 110 = 55 ms |
| P4_MIN | 0x0C | default 10 = 5 ms |
| W0 / W1 / W2 / W3 / W4 / W5 | 0x19 / 0x0E / 0x0F / 0x10 / 0x11 / 0x12 | defaults 300 / 300 / 20 / 20 / 50 / 300 ms |
| TIDLE | 0x13 | default 300 ms; bus idle before fast init |
| TINIL | 0x14 | default 25 ms; low pulse of fast init; 1 ms per bit |
| TWUP | 0x15 | default 50 ms; "duration of the wake-up pulse in fast initialization" (the low time plus the high time that follows); 1 ms per bit |
| PARITY | 0x16 | 0 none, 1 odd, 2 even |
| DATA_BITS | 0x20 | 0 = 8 bits, 1 = 7 bits |
| FIVE_BAUD_MOD | 0x21 | 0 = ISO 9141-2/14230-4 sequence, 1 = interface sends inverted key byte 2 only, 2 = ECU sends inverted address only, 3 = plain ISO 9141 |
| Ioctl FIVE_BAUD_INIT | - | input: one address byte; output: two key words |
| Ioctl FAST_INIT | - | input: any message "or any other ISO 9141 or ISO 14230 message specified by the application. If NULL, no message is transmitted"; output may be NULL; "For the ISO9141 Protocol, no verification of message format will be applied" |

The numeric protocol IDs and flag values are confirmed by an independent implementation (`ISO9141 = 3`, `ISO9141_NO_CHECKSUM = 0x200`, `ISO9141_K_LINE_ONLY = 0x1000`). HIGH ([j2534 crate source](https://docs.rs/j2534/0.1.2/src/j2534/lib.rs.html)).

Inference: the standard ISO9141 channel can express a Honda-style session without any extension - 9600 or 10400 baud, checksum off, TINIL = 70 and a longer TWUP for the motorcycle-style pulse, FAST_INIT with the wake-up frame or with no frame. MEDIUM (follows from the specification text; what HDS really requests is unknown).

#### J2534-2 and Honda

| Fact | Confidence | Source |
|---|---|---|
| `HONDA_DIAGH_PS` = 0x800B | HIGH | [J2534-Sharp discussion with a vendor header](https://github.com/BrianHumlicek/J2534-Sharp/discussions/13), [Quantex table](https://quantexlab.com/el/develop/j2534/pt_secconnect.html) |
| `ISO9141_PS` = 0x8002, `ISO14230_PS` = 0x8003 (pin-selectable variants); `J1962_PINS` config parameter ID 0x8001 | MEDIUM | same discussion |
| Name of 0x800A differs: `UART_ECHO_BYTE_PS` in vendor headers, `TP1_6_PS` elsewhere | LOW (conflict) | discussion vs. Quantex search snippet |
| SAE J2534-2/4_0500 (January 2022) is a separate document "Pass-Thru Extended Feature - HONDA_DIAGH" | HIGH | [SAE](https://saemobilus.sae.org/content/J2534-2/4_5_00/) |
| SAE J2809 (issued October 2007, stabilised October 2017), "Honda Diagnostic Serial Data Link Protocol - ABS/VSA System", defines the proprietary protocol for ABS or VSA ECUs "implemented on some Honda vehicles" and is to be used with J2534-2 | HIGH | [SAE](https://saemobilus.sae.org/standards/j2809_201710-honda-diagnostic-serial-data-link-protocol-abs-vsa-system) |
| Honda's MVCI (part OTCHON3828AF) supports "Two Pin Selectable Dual Wire CAN channels, Single Wire CAN, K-Line, Honda Diag H, Honda Diag X" with API "J-2534-1, J-2534-2" | MEDIUM | [dealer tool catalogue](https://acura.snapon.com/HondaAcura/Catalog/Product/OTCHON3828AF) |
| Honda Australia (February 2024): i-HDS needs "a pass-thru Vehicle Communication Device (VCI) that is compliant with SAE J2534-1 and J2534-2 protocols"; a generic J2534 VCI can read codes and, with security validation, program keys, PCM and immobiliser; a generic scan tool can only read codes; Windows 10 only | HIGH | [Honda document](https://aasra.com.au/wp-content/uploads/2024/02/Honda-diagnostic-updated-12.02.2024.pdf) |
| A generic pass-through cable (GODIAG GD101) lists for HDS: "Does not support non-#7 K-line" | LOW | vendor listings ([example](https://www.godiagshop.com/wholesale/godiag-gd101-j2534.html), search snippets) |
| A USD 30 "HDS cable" clone was found to contain an FT232R and an NXP LPC2148 and to copy the Drew Technologies Mongoose | LOW | [owner blog, 2015](https://honda-civic-hatchback-mk9.blogspot.com/2015/07/honda-hds-and-j2534-adapter-clone.html) |

Which protocol IDs, pins, baud rates and init calls HDS actually issues for a pre-CAN Civic could not be found in any public source. Candidates are ISO9141 (0x03) on pin 7, `ISO9141_PS` with `J1962_PINS` for another pin, and `HONDA_DIAGH_PS`. LOW (inference from the table above).

#### What implementing J2534 for our device would involve

1. A 32-bit Windows DLL exporting the 14 functions, registered under `PassThruSupport.04.04` in the 32-bit registry view. A recent hobby project does this in C# (.NET Framework 4.7.2, x86, DllExport) and registers under `HKLM\SOFTWARE\WOW6432Node\PassThruSupport.04.04\<name>` with a `FunctionLibrary` value; it notes that AnyCPU and x64 builds "will not work with 32-bit diagnostic tools". MEDIUM ([taskmanager-j2534](https://github.com/gorevyoneticisi/taskmanager-j2534)).
2. Firmware support for every row of the parameter table above, message filters, periodic messages on firmware timers, receive timestamps, transmit-done and receive-start indications, and battery voltage readout.
3. Behaviour that OEM applications check: the same project reports that applications poll battery voltage often and may abort on low readings, are sensitive to version strings, and expect `ERR_BUFFER_EMPTY` when nothing is queued. MEDIUM (same README, CAN applications).
4. For Honda: possibly J2534-2 pin switching and `HONDA_DIAGH_PS`, whose details sit in paid SAE documents (J2534-2/4, J2809).

#### Open-source J2534 projects

| Project | What it is | K-line status | Licence |
|---|---|---|---|
| [rnd-ash/Macchina-J2534](https://github.com/rnd-ash/Macchina-J2534) | Rust driver + Arduino-loaded firmware for Macchina M2 and A0; Windows, Linux, macOS | ISO9141 and ISO14230-4 "WIP" on M2, unsupported on A0 | not stated on the page |
| commaai/panda J2534 driver | Windows J2534 DLL for the panda | ISO 9141 listed as unsupported until 5-baud init exists; README no longer at the old path (404). LOW: mirror text via search; [forum pointer](https://www.macheforum.com/site/threads/use-comma-ai-panda-for-fdrs.27114/) | licence not checked |
| [gorevyoneticisi/taskmanager-j2534](https://github.com/gorevyoneticisi/taskmanager-j2534) | C# DLL + STM32F407 firmware, FT232H at 921600 baud; confirmed with Techstream, ODIS, IDS | CAN only; "Honda pre-2008 K-line ECUs with i-HDS" explicitly not covered | proprietary, "source-available, not open source" |
| [hackingvolvo/SardineCAN-Arduino](https://github.com/hackingvolvo/SardineCAN-Arduino) | Arduino firmware + separate Win32 DLL, aimed at Volvo VIDA | CAN only | GPL-3.0 |
| [BrianHumlicek/J2534-Sharp](https://github.com/BrianHumlicek/J2534-Sharp) | .NET wrapper to call any vendor's J2534 DLL (v2.0 targets .NET 10) | client side; supports whatever the DLL supports | MIT |
| [bouletmarc/HondaReflashTool](https://github.com/bouletmarc/HondaReflashTool) | C# Honda/Acura reflash tool over J2534 using J2534-Sharp | CAN only (2006 and later ECUs); nothing for K-line cars | not stated |
| [jakka351/OpenJ2534](https://github.com/jakka351/OpenJ2534) | Curated list of J2534 resources | - | list |

All rows HIGH for "what it is" (repository pages), except where marked. No finished open-source K-line J2534 device was found. MEDIUM (search was cut short).

#### Logging shim DLLs

| Project | How it works | Licence |
|---|---|---|
| [joeyoravec/j2534-logger](https://github.com/joeyoravec/j2534-logger) | `ptshim32.dll` sits between the application and the vendor DLL; can be registered under `PassThruSupport.04.04` like a normal interface; on first use a dialog selects the real device; records to a memory buffer saved as UTF-8 text | GPL-3.0 |
| [MEAT-Inc/Fulcrum](https://github.com/MEAT-Inc/Fulcrum) | C++ shim plus "FulcrumInjector" application that launches the OE software, selects "FulcrumShim" as the device and writes `.shimLog` files; mentions replayable simulations; early-stage by its own description | licence file not read |
| [rollsch/j2534-logger](https://github.com/rollsch/j2534-logger) | another logger listed in OpenJ2534 | not inspected |

HIGH for the first two (repository pages). A shim log shows the API-level truth - protocol ID, flags, every SET_CONFIG value, init ioctl, message boundaries - which a logic-analyser trace cannot show. It still needs a working combination of HDS, a pass-through interface HDS accepts, and the car.

### 4. Host-side stack options and trade-offs

| Option | Strengths | Weaknesses | Verdict |
|---|---|---|---|
| C# / .NET | Owner's language; J2534-Sharp (MIT) gives access to commercial interfaces; HondaReflashTool and taskmanager-j2534 show the ecosystem; a 32-bit J2534 DLL can be written in C# | Vendor J2534 DLLs are 32-bit, so the process loading them must be x86 | Main host library, CLI and later GUI |
| Python | Fastest for experiments; pyserial/pyftdi; HondaECU and eculib are Python; good for capture conversion scripts feeding REA | python-OBD is ELM327-only and GPL-2.0; distribution to end users is clumsy | Lab scripting, converters, test harness |
| Rust | ecu_diagnostics and Macchina-J2534 exist | both GPL-3.0 or unlicensed, no K-line layer; no advantage for this owner | Not recommended |
| Browser (Web Serial) | Zero install UI | MDN: "not Baseline because it does not work in some of the most widely-used browsers"; secure context required | Possible later front end only |
| Android / iOS apps | Convenience in the car | Needs BLE or Wi-Fi transport and a second code base | Later, through the ELM327 personality or a small custom app |

Confidence: the rows are judgement based on facts cited elsewhere in this file; the Web Serial statement is HIGH ([MDN](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API)).

Transport latency against K-line timing:

| Quantity | Value | Confidence and source |
|---|---|---|
| Tester inter-byte gap P4 | 5 ms default | HIGH (J2534-1; OBD9141 uses 5 ms) |
| ECU inter-byte maximum P1 | 20 ms default | HIGH (J2534-1) |
| Gap before the next request P3 | 55 ms minimum default | HIGH (J2534-1) |
| Fast-init pulse | 25 ms low inside a 50 ms pattern | HIGH (J2534-1, OBD9141, aster94 agree) |
| Key byte to inverted key byte W4 | 50 ms default maximum | HIGH (J2534-1) |
| Keep-alive | "at least once every five seconds"; ELM327 default 3 s | HIGH (ELM327 datasheet page 34) |
| USB-UART bridge round trip | "about 1-2 ms per flow-control round trip" with an FT232H | MEDIUM (taskmanager-j2534 README) |
| Wireless links | "Some modules require a minimum delay between sending a request and receiving a response from the ECU. This can only be achieved by USB connection. Wireless connection may reduce read performance significantly." | MEDIUM ([Scanmatik guide](https://scanmatik.ru/downloads/SM_J2534_RP1210_DPDU_Guide_EN.pdf), page 6) |
| FTDI latency timer default, BLE connection intervals | not verified this round | LOW (see open questions) |

Conclusions:
- Millisecond-level items (pulse edges, 5-baud bits, P4 gaps, W4, echo comparison) belong in firmware. Then the transport only carries whole frames and any of USB CDC, BLE or Wi-Fi is adequate for diagnostics. MEDIUM (design reasoning supported by the J2534 model, where the device owns timing).
- Host-side timing over a USB-UART is nevertheless proven workable for Honda motorcycle ECUs (HondaECU), which makes a dumb cable a legitimate first experiment. HIGH (source code).
- Keep-alive must run in firmware or as a periodic message, because J2534-style devices do not send tester-present by themselves and a stalled host would drop the session. HIGH (J2534-1 text).

### 5. Data sources

| Source | Content | Licence | Use for a 2001-2005 Civic | Confidence |
|---|---|---|---|---|
| [OBDb/Honda-Civic](https://github.com/OBDb/Honda-Civic) | one `signalsets/v3/default.json` (158 kB), `generations.yaml` (seventh generation 2001-2005 listed), per-year test cases | CC-BY-SA-4.0 | None in practice: the 2004 and 2005 test cases list every probed command as unsupported; the Honda-specific commands are CAN-era (`DAxx` ECUs, service 22) | HIGH ([2004](https://raw.githubusercontent.com/OBDb/Honda-Civic/main/tests/test_cases/2004/command_support.yaml), [2005](https://raw.githubusercontent.com/OBDb/Honda-Civic/main/tests/test_cases/2005/command_support.yaml)) |
| [mytrile/obd-trouble-codes](https://github.com/mytrile/obd-trouble-codes) | generic DTC text in JSON, CSV, SQLite | MIT | Generic P0xxx descriptions | MEDIUM (count and source not stated) |
| [foerbsnavi/OBDex](https://github.com/foerbsnavi/OBDex) | generic codes only; manufacturer codes "deliberately out of scope" | unclear | Generic only | LOW (search summary) |
| [peyo/dtc-and-vin-data](https://www.github.com/peyo/dtc-and-vin-data) | DTC categories including Honda | unknown | Possible Honda P1xxx seed; not inspected | LOW |
| kerpz `torque/HOBD.csv` | Torque custom PIDs for his emulator | none | Shows how to present Honda data to Torque | HIGH (file tree) |
| Torque wiki custom init | Toyota, Nissan JDM only | - | No Honda entry | HIGH |
| ScanGauge X-Gauge repertory | could not be read (bot protection) | - | Unknown | - |

Honda-specific DTC tables (P1xxx, ABS, SRS, immobiliser), live-data definitions and memory maps were not found in any open repository. They have to come from service literature (other dimensions) and from reverse engineering. MEDIUM (negative result with a truncated search).

### 6. Capture and analysis tooling

- sigrok decoders relevant here: `uart`, `can`, `lin`, `flexray`, `sae_j1850_vpw`, plus utilities `timing`, `counter`, `jitter`, `guess_bitrate`. There is no K-line, ISO 9141, KWP2000 or OBD decoder. HIGH ([sigrok decoder list](https://sigrok.org/wiki/Protocol_decoders)).
- The pcap link-type registry has no K-line, ISO 9141 or ISO 14230 type and no generic UART type. Available: `LINKTYPE_USER0` to `USER15` (147-162, private use), `LINKTYPE_RTAC_SERIAL` (250, a vendor's serial format), `LINKTYPE_LIN` (212), `LINKTYPE_CAN_SOCKETCAN` (227). HIGH ([tcpdump.org](https://www.tcpdump.org/linktypes.html)).
- A 2004 developer thread discussed adding ISO 9141/14230 capture support to Ethereal; I found no evidence of a K-line dissector in current Wireshark (two guessed source file names returned 404). LOW ([thread](https://lists.wireshark.org/archives/ethereal-dev/200410/msg00290.html), search snippet).
- `text2pcap -l <typenum>` sets the link type of the output file, `-t` parses timestamps, `-D` reads an I/O direction marker; output is pcapng by default. HIGH ([text2pcap manual](https://www.wireshark.org/docs/man-pages/text2pcap.html)).
- [jared52005/Monitor](https://github.com/jared52005/Monitor) (MIT): ESP32/STM32 firmware and PC tools that stream K-line (ISO 9141, ISO 14230, KW1281) and CAN traffic to Wireshark over a TCP pipe (port 19000 for K-line), wrapping frames in a dummy IPv4 header with a custom protocol number and decoding them with Lua dissectors; includes a passive K-line hardware folder and an FT232 + MC33660 adapter interface. HIGH (README).
- Quantex publishes a compact text format for J2534 call logs (delta time, level, device, command such as `conn`, `wr`, `rd`, `io`, then data lines). Useful as a model for our own log format. MEDIUM ([Quantex](https://quantexlab.com/en/develop/j2534/log_format.html)).

Recommended pipeline (design proposal, not a sourced fact):

1. Capture with a logic analyser (sigrok UART decoder at the right baud rate) or, better, with our own device in passive mode.
2. Group bytes into frames by inter-byte gap or length byte; keep wake-up pulses and 5-baud addresses as events with their durations.
3. Write pcapng with `LINKTYPE_USER0` and a small fixed pseudo-header (direction, baud rate, event type, flags).
4. Dissect with a Lua script (generic K-line frame, then ISO 9141-2/KWP2000 and Honda sub-dissectors).
5. Convert J2534 shim logs into the same pcap format, so that all evidence - sniffed traffic, shim logs, our own sessions - is searchable with TShark and REA in one way.

### 7. Testing without the car

| Tool | What it simulates | Licence | Use |
|---|---|---|---|
| OBD9141 simulator class | ISO 9141-2 ECU responses on a microcontroller | MIT | Second board with a K-line transceiver as a bench ECU |
| aster94 ECU emulator (Python, `extras`) | KWP2000 ECU over a serial adapter | GPL-3.0 | External test tool |
| freediag CARSIM (`diag_l0_sim.c`, `.db` files) | File-driven car for freediag's own stack: lines `RQ <bytes>` / `RP <bytes>`, checksum token `cks1`; the ISO 9141 file begins `RQ 0x33`, `RP 0x55`, `RP 0x08`, `RP 0x08`, `RQ 0xF7`, `RP 0xCC` | GPL-3.0 | The file format is a good model for replay tables |
| [Ircama/ELM327-emulator](https://github.com/Ircama/ELM327-emulator) | An ELM327 with multiple ECUs; K-line option `-K` echoes each byte, answers the 5-baud address with `55 08 08` and StartCommunication with `83 F1 33 C1 8F E9 E0`; scenario dictionaries and plugins; a helper builds a dictionary by scanning a real car | CC BY-NC-SA 4.0 (badge) | Tests for host software that talks to ELM327 adapters; non-commercial terms, so keep it external |
| [OBDSim](https://icculus.org/obdgpslogger/obdsim.html) | ELM327 device with generator plugins (random, cycle, log replay) | GPLv2 or later | Simple app-level tests |
| [ECUsim 5100](https://www.obdsol.com/solutions/development-tools/obd-simulators/ecusim-5100) | Commercial hardware simulator: J1850, ISO 9141-2, ISO 14230-4, CAN; three virtual ECUs (ECM, TCM, ABS) per plug-in module | commercial | Reference-grade legislated OBD target; no Honda proprietary support |

HIGH for each row's description (project pages or source files), MEDIUM for the ELM327-emulator licence (badge only).

Proposed test ladder (design proposal):

1. Unit tests on the host protocol code with recorded request/response tables in an RQ/RP-style text format.
2. Loopback bench: two boards on a 12 V K-line, one running our firmware as tester and one as emulator replaying tables, including init pulses and deliberate faults (bad checksum, late byte, bus held low).
3. Replay of J2534 shim logs: the logged call sequence is run against our firmware and the emulator answers with the logged ECU replies.
4. Hardware in the loop with a spare ECM on a bench supply. The only sourced hint is from the motorcycle tool: 2.5 A supply recommended, slower transfers and CRC errors at 2.0 A. MEDIUM (HondaECU fork README). What a Civic ECM needs to wake up on a bench belongs to another dimension.

### 8. Recommended layered architecture

```
+-------------------------------------------------------------+
| Host applications: CLI, GUI, scripts, optional J2534 DLL     |
+-------------------------------------------------------------+
| Service layer (host): DTC read/clear per ECU, live data,     |
| freeze frame, service functions, flash - data-driven tables  |
+-------------------------------------------------------------+
| Protocol engines (host first, firmware later if stable):     |
|   ISO 9141-2 / ISO 14230-4 legislated OBD (J1979)            |
|   Honda proprietary framings (length byte + zero-sum cks)    |
+-------------------------------------------------------------+
| Channel interface: one abstraction, two back-ends            |
|   (a) our device   (b) any J2534 interface via J2534-Sharp   |
+-------------------------------------------------------------+
| Transport: framed binary over USB CDC (BLE / Wi-Fi later)    |
+-------------------------------------------------------------+
| Link and timing layer (firmware): baud, parity, P1/P3/P4,    |
| 5-baud init, pulse init, echo compare, framing, timestamps,  |
| periodic messages, passive sniff mode                        |
+-------------------------------------------------------------+
| Physical driver: K-line transceiver, UART RX always on,      |
| GPIO control of TX for pulses, battery voltage measurement   |
+-------------------------------------------------------------+
```

Reasons:

1. Firmware owns timing. Every project that works at byte level puts 25 ms and 200 ms edges and the 5 ms gaps close to the wire, and J2534 defines the device as the owner of P1/P3/P4/W-timers. This makes the transport choice uncritical. Based on HIGH facts in sections 1, 3 and 4.
2. Shape the firmware primitives after the J2534-1 ISO9141 channel (the parameter table in section 3) plus one generic "pulse low x ms, high y ms" primitive. It is a published, complete vocabulary for K-line variants; a J2534 DLL can be added later without redesign; and calls captured from HDS with a shim can be replayed one to one.
3. Keep Honda protocol logic on the host at first. The framing is still unknown for this car, host code is quicker to change, and the same code can run over a commercial J2534 interface before our hardware exists.
4. One channel interface with two back-ends removes the hardware from the critical path and gives a reference to compare our device against.
5. Binary framing with length and CRC on USB CDC, not a text protocol, for the native interface: timestamps, events (pulse seen, framing error) and exact bytes are needed for captures. The ELM327 text personality, if built, is a separate mode on top of the legislated OBD engine.
6. Passive sniff mode in the same firmware turns the device into the capture tool for the pcap pipeline in section 6.
7. Echo handling by read-back and compare, because it doubles as bus-fault detection (short to ground or battery, a second tester on the line).

Licences of candidates for reuse:

| Item | Licence | Consequence |
|---|---|---|
| OBD9141 | MIT | Code may be ported into firmware of any licence, with attribution |
| ELMduino, J2534-Sharp, jared52005/Monitor, j2534 Rust crate, mytrile DTC data | MIT | Free to reuse with attribution |
| sabsteef Honda_K-line | BSD 2-Clause | Free to reuse with attribution |
| SoftwareSerialWithHalfDuplex | LGPL-2.1 or later | Usable as a library; not needed on a hardware UART |
| HondaECU, eculib, aster94, muki01, freediag, j2534-logger, ecu_diagnostics, SardineCAN, HonOBDapt | GPL-3.0 | Copying code makes our code GPL-3.0; reading them for protocol facts and running them as separate tools is fine |
| python-OBD, OBDSim | GPL-2.0 / GPLv2 or later | Same, as separate tools |
| OBDb data | CC-BY-SA-4.0 | Attribution and share-alike for derived data |
| Ircama ELM327-emulator | CC BY-NC-SA 4.0 | Non-commercial; external test tool only |
| kerpz/ArduinoHondaOBD, AutotronicCommunity documentation | no licence | Facts may be used; code and text may not be copied |
| taskmanager-j2534 | proprietary, source-available | Study only |

The project needs a licence decision early: a permissive licence rules out copying from the GPL projects; a GPL-3.0 project could reuse HondaECU and freediag code directly.

### Targets for the REA toolkit noted along the way

| Target | Tool | Why |
|---|---|---|
| HDS / i-HDS installation (executables, per-ECU data files) | Ghidra, Binwalk | The definitive source of Honda request and reply tables for every control unit |
| J2534 DLLs of Honda-capable interfaces (HIM / GNA600-type, Mongoose-type clones) | Ghidra | Shows how `HONDA_DIAGH_PS` and pin selection are implemented |
| J2534 shim logs of an HDS session on the car | converted to pcap, TShark | Exact protocol IDs, baud rates, init parameters, frames |
| K-line captures from sigrok or our sniff mode | TShark with a Lua dissector | Wire-level truth including pulse widths |
| Keihin motorcycle ECU images shipped in HondaECU forks (`bins/`) and their `.xdf` files | Ghidra | Same vendor family; the K-line command handler in those images may resemble the car ECM's (LOW, inference) |
| Civic ECM firmware dump, once obtained | Ghidra | Command handler, security check, flash routines |
| Android applications that claim Honda support | JADX | Possible ELM/STN command recipes; none identified yet |

## Conflicts and uncertainties

1. Two different Honda framings exist in open code: kerpz (9600 baud, no pulse, `cmd len addr count cks`, "before 2002" ECUs, 3-pin DLC) and the Keihin motorcycle protocol (10400 baud, 70 ms pulse, `72 len ...`). Neither source covers a 2001-2005 Civic. Both share the length byte and the zero-sum checksum.
2. Pulse timing in the motorcycle protocol: 120 ms high in the AutotronicCommunity text, 130 ms wait in eculib. Both are probably inside the ECU's tolerance; the exact window is unknown.
3. The kerpz README wording on supported vehicles reached me through a summariser ("before 2002"; one search summary said "pre-2002 non-US"). The exact sentence should be re-read before relying on it.
4. `HONDA_DIAGH_PS` = 0x800B is consistent across sources, but neighbouring J2534-2 IDs are named differently in different headers (0x800A, 0x800D-0x8010). Treat any J2534-2 constant other than 0x800B, 0x8002, 0x8003 as unconfirmed.
5. The panda J2534 statement comes from a mirrored old README; the driver is no longer at the path where it used to be, so its present status is unknown.
6. STN command facts come from 2011 documents (STN1100 manual revision A, OBDLink MX sheet). Newer manuals may add K-line commands, including pulse control.
7. Whether a genuine ELM327 appends its checksum to every K-line request was not re-read on a datasheet page this round (it was read for wake-up messages); the "trailing 0x00" consequence is my own arithmetic.
8. The clone "HDS cable" teardown (FT232R + LPC2148, Mongoose copy) is one owner's blog from 2015; current cables may differ.
9. ELM327-emulator's licence was taken from a badge, and Fulcrum's and Macchina-J2534's licences were not visible; check the licence files before any reuse.

## Open questions

| Question | How to answer it |
|---|---|
| Which J2534 protocol ID, pins, baud rate, parity, init ioctl and timing values does HDS use for each control unit of a 2001-2005 Civic? | Run HDS through a logging shim (j2534-logger or Fulcrum) with an interface HDS accepts, on the car; or reverse engineer HDS with Ghidra |
| Does the Civic ECM answer the kerpz-style 9600-baud frames, the motorcycle-style frames, or something else? | Passive sniff of a commercial scan tool or HDS on the car; bench test with a dumb K-line cable |
| What is in SAE J2534-2/4 (HONDA_DIAGH) and SAE J2809 (ABS/VSA protocol): baud rate, signalling, framing? | Buy and read the two SAE documents; or reverse engineer a vendor J2534 DLL |
| Can an STN chip generate a custom wake-up pulse, and do current manuals add K-line commands? | Read the current OBDLink Family Reference and Programming Manual |
| Does a Honda ECU tolerate a trailing extra byte after a valid frame (the ELM327 hack)? | Bench test with a genuine ELM327 |
| Does current Wireshark contain any K-line or KWP2000-over-K-line dissector? | Inspect the Wireshark source tree and release notes |
| FTDI latency-timer default and BLE connection-interval limits (to size the transport) | Read the FTDI application note and Bluetooth platform guidelines; the fetch of the FTDI note was refused this round |
| Do ScanGauge X-Gauge or Torque PID lists for pre-CAN Honda exist? | Read the ecomodder X-Gauge repertory in a normal browser; search Honda forums |
| Which Bluetooth profile and reply strings do Torque, OBD Fusion and Car Scanner require from an "ELM327"? | Test with an ELM327 emulator and the apps; JADX on the apps |
| Is HDS (and are the vendor DLLs) 32-bit, which fixes the bitness of any DLL we write? | Inspect the installed binaries |
| Is the history of RyanHope/HondaECU preserved completely in the forks, and what changed after MCU Innovations took over? | Compare fork histories |
| What is the project's licence (permissive or GPL)? | Owner decision; it determines whether GPL code can be reused |

## Sources

Honda-specific code and documentation
- https://github.com/kerpz/ArduinoHondaOBD - Arduino reader for the Honda 9600-baud DLC protocol with ELM327 emulation (README, `hobd_uni2.ino`, `hobd_elm.ino` read as raw files)
- https://github.com/Adeen117/HondaECU - fork of HondaECU with full source (GPL-3.0), motorcycle diagnostics and flashing
- https://github.com/ekomook/HondaECU - fork whose README notes the takeover by MCU Innovations
- https://github.com/Pongmi04/eculib - copy of the eculib K-line library used by HondaECU (`base.py`, `honda.py`)
- https://github.com/AutotronicCommunity/Honda_Keihin_KLine_Protocol - written description of the Honda/Keihin motorcycle K-line protocol
- https://github.com/K9CTec/Honda_K-line - resolves to a fork of sabsteef's BSD-licensed Honda K-line sketch
- https://github.com/bouletmarc/HondaReflashTool - C# J2534 reflash tool for CAN-era Honda ECUs
- https://sourceforge.net/projects/honobdapt/ - adapter software for 1992-1995 Honda diagnostic ports (GPLv3)

Generic K-line libraries and tools
- https://github.com/iwanders/OBD9141 - MIT library for ISO 9141-2 and ISO 14230 (`src/OBD9141.h`, `src/OBD9141.cpp`)
- https://github.com/aster94/Keyword-Protocol-2000 - GPL-3.0 KWP2000 library (`src/KWP2000.cpp`, `src/ISO.h`)
- https://github.com/muki01/OBD2_KLine_Library - GPL-3.0 multi-protocol K-line library
- https://github.com/PowerBroker2/ELMduino - MIT ELM327 client library
- https://github.com/nickstedman/SoftwareSerialWithHalfDuplex - single-pin half-duplex software serial (LGPL)
- https://github.com/fenugrec/freediag - GPL-3.0 scan tool; https://freediag.sourceforge.io/Scantool-Manual.html ; raw `doc/dumb_interfaces.txt` ; raw `scantool/freediag_carsim_iso9141.db`
- https://github.com/brendan-w/python-OBD - GPL-2.0 Python ELM327 client
- https://github.com/rnd-ash/ecu_diagnostics - GPL-3.0 Rust diagnostics crate

ELM327 and STN
- https://cdn.sparkfun.com/assets/4/e/5/0/2/ELM327_AT_Commands.pdf - Elm Electronics AT command list with first-appearance versions (October 2010)
- https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf - ELM327 datasheet ELM327DSI (v2.0), 82 pages
- https://www.scantool.net/downloads/118/mx_protocol_commands.pdf - OBDLink MX protocol commands and ST protocol presets (December 2011)
- https://www.manualsdir.com/manuals/710473/scantool-stn1100-frpm.html?page=12 - STN1100 manual revision A, ISO-specific ST commands
- https://wiki.torque-bhp.com/view/Custom_Init - Torque custom init strings (Toyota, Nissan; no Honda)
- https://www.drive2.ru/b/535600977143662337/ - Russian analysis of a Toyota K-line ECU with ELM327 (9600 baud, ATIIA13)
- https://www.pakwheels.com/forums/t/help-required-regarding-elm-327/242479 - forum thread with a reused Toyota init string (snippet only)
- https://www.obdsoftware.net/software/obdfusion - OBD Fusion product page (no Honda enhanced add-on)

J2534
- https://netcult.ch/elmue/HUD%20ECU%20Hacker/J2534%20PassThrough.pdf - SAE J2534-1, revised December 2004 (primary text)
- https://docs.rs/j2534/0.1.2/src/j2534/lib.rs.html - Rust J2534 bindings with protocol and flag constants (MIT)
- https://github.com/BrianHumlicek/J2534-Sharp - MIT .NET wrapper; https://github.com/BrianHumlicek/J2534-Sharp/discussions/13 - J2534-2 protocol IDs and pin switching
- https://quantexlab.com/el/develop/j2534/pt_secconnect.html - Quantex table of extended protocol IDs including HONDA_DIAGH_PS
- https://www.quantexlab.eu/en/develop/j2534/pt_connect.html - Quantex PassThruConnect reference
- https://quantexlab.com/en/develop/j2534/log_format.html - Quantex J2534 log format
- https://saemobilus.sae.org/standards/j2809_201710-honda-diagnostic-serial-data-link-protocol-abs-vsa-system - SAE J2809 abstract
- https://saemobilus.sae.org/content/J2534-2/4_5_00/ - SAE J2534-2/4 HONDA_DIAGH extended feature
- https://github.com/joeyoravec/j2534-logger - GPL-3.0 logging shim
- https://github.com/MEAT-Inc/Fulcrum - C++ shim and injector application
- https://github.com/rnd-ash/Macchina-J2534 - Rust J2534 driver for Macchina hardware
- https://github.com/gorevyoneticisi/taskmanager-j2534 - source-available C# J2534 DLL with STM32 firmware (CAN only)
- https://github.com/hackingvolvo/SardineCAN-Arduino - GPL-3.0 Arduino J2534 device firmware (CAN)
- https://github.com/jakka351/OpenJ2534 - list of J2534 resources
- https://www.macheforum.com/site/threads/use-comma-ai-panda-for-fdrs.27114/ - forum pointer to the panda J2534 driver
- https://scanmatik.ru/downloads/SM_J2534_RP1210_DPDU_Guide_EN.pdf - Scanmatik J2534 guide (USB versus wireless remark)

Honda tools
- https://aasra.com.au/wp-content/uploads/2024/02/Honda-diagnostic-updated-12.02.2024.pdf - Honda Australia: i-HDS and J2534 technical specifications (February 2024)
- https://acura.snapon.com/HondaAcura/Catalog/Product/OTCHON3828AF - Honda MVCI catalogue entry with protocol list
- https://www.godiagshop.com/wholesale/godiag-gd101-j2534.html - GODIAG GD101 listing (HDS limitation on non-pin-7 K-line)
- https://honda-civic-hatchback-mk9.blogspot.com/2015/07/honda-hds-and-j2534-adapter-clone.html - teardown note on a clone HDS adapter

Data
- https://github.com/OBDb/Honda-Civic - OBDb signal set for the Civic; raw test cases for 2004 and 2005
- https://github.com/mytrile/obd-trouble-codes - MIT generic DTC list
- https://github.com/foerbsnavi/OBDex - generic DTC list
- https://www.github.com/peyo/dtc-and-vin-data - DTC data with a Honda category (not inspected)
- https://ecomodder.com/wiki/X-Gauge_Repertory - ScanGauge X-Gauge list (blocked for automated access)

Capture, analysis, simulation
- https://sigrok.org/wiki/Protocol_decoders - sigrok decoder list
- https://www.tcpdump.org/linktypes.html - pcap link-layer type registry
- https://www.wireshark.org/docs/man-pages/text2pcap.html - text2pcap manual
- https://lists.wireshark.org/archives/ethereal-dev/200410/msg00290.html - 2004 discussion of ISO 9141/14230 capture support
- https://github.com/jared52005/Monitor - MIT K-line and CAN monitor feeding Wireshark with Lua dissectors
- https://github.com/Ircama/ELM327-emulator - ELM327 and ECU emulator with K-line mode
- https://icculus.org/obdgpslogger/obdsim.html - OBDSim
- https://www.obdsol.com/solutions/development-tools/obd-simulators/ecusim-5100 - commercial multi-protocol ECU simulator
- https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API - Web Serial availability
