# 06 - Interface hardware: design options and prior art

Research date: 2026-11-14. Scope: the electrical/hardware side of a home-built diagnostic interface for a 2002 Honda Civic (7th generation) DLC. Web research only; nothing was installed or executed.

Confidence tags: **HIGH** = primary source read directly (datasheet, schematic, vendor page) or two independent sources agree; **MEDIUM** = one credible source, or well-established general knowledge not re-verified in this session; **LOW** = single forum post, inference, or conflicting reports. Paragraphs marked *Design note* are this researcher's engineering proposals, not sourced facts.

Research limitation: the shared web-search budget for the whole research round ran out before this dimension was finished. The Honda HIM teardown, comma panda schematics, KKL-cable teardown, bench-harness products and several 2026 prices are therefore thinner than planned and are tagged accordingly.

## Summary

1. A K-line channel that must send 5-baud init and 25-200 ms low pulses needs a transceiver **without a TXD dominant time-out**. Parts that qualify and are active in 2026: ST **L9637D** (true ISO 9141 interface), TI **TLIN1027-Q1**, TI SN65HVDA195-Q1, NXP **TJA1027**, Atmel/Microchip ATA6664. Parts that do NOT qualify (time-out present): SN65HVDA100, TJA1021, TJA1029, ATA6663 (55 ms typ), MCP2003B, TLE7259-3. NXP MC33290 and MC33660 are discontinued/end-of-life.
2. Recommended: first choice **L9637D** on pin 7 (4.5-36 V, no time-out, >= 50 kHz, 3.3 V logic, spare comparator); second choice **TLIN1027-Q1** (or pin-compatible **TJA1027**, which both Macchina M2 and OpenDiag use for K-line). LIN-type parts are transmit-limited to 20 kbit/s.
3. The tester must supply a **510 ohm pull-up to battery voltage** on the K-line (ISO 9141; ELM327, ScanTool/SparkFun and L9637 datasheets all show it). It must be switchable: two testers in parallel give 255 ohm, which is why a sniffer must present high impedance.
4. Honda's second diagnostic line (DIAG-H, reported on DLC pin 14) is electrically unresolved: one forum source says it is a **5 V bus**, another reports that bridging pin 7 to pin 14 on a 12 V K-line cable works. The second channel must therefore be a *universal* single-wire channel (open-drain driver, selectable pull-up to none/5 V/Vbat, programmable receive threshold, analogue read-back), not a fixed 12 V K-line transceiver.
5. SCS (pin 9) idles near 5 V and is grounded by the dealer tool; a protected open-drain low-side switch with read-back is sufficient. Nothing public describes the electrical side of the write-enable line.
6. Microcontroller: **RP2350 (Raspberry Pi Pico 2 W, about US$7)** fits best: GNU Arm toolchain already installed, 2 UARTs + 12 PIO state machines for exact bit timing and edge capture, USB device, BLE and Wi-Fi. ESP32-S3 is the alternative when the wireless/software ecosystem matters more than hard timing (separate toolchain).
7. Power: fuse/PTC, TVS, reverse-polarity element, then a 40-60 V-rated buck; design for ISO 16750-2 "Test B" (35 V suppressed load dump), 14 V reverse, 24 V jump start; target under 1 mA asleep (ELM327 reference circuit: about 30 mA awake, under 1 mA with modifications).
8. Protocol work can start before any PCB exists with: an FTDI-based KKL 409.1 cable, a Pico + L9637D on a breadboard, a full 16-pin OBD breakout/Y-cable and a logic analyser. ELM327/STN-based boards (SparkFun OBD-II UART, WiCAN Pro) only help with legislated OBD.
9. No open design covers Honda's pins out of the box: Macchina M2 wires only pins 7 and 15 to its two TJA1027 channels (pins 9 and 12 are test points) and fits no 510 ohm pull-up; OpenDiag (2025, ESP32-S3, US$60) has high-side drivers on pins 9/12/14 but cannot ground pin 9.
10. The common "OBD-II to DB9" cable carries only nine conductors and therefore cannot carry SCS/pin 9 and pin 12; use a fully populated 16-pin extension or breakout.

## Findings

### 1. K-line transceiver choices in 2026

**Why the time-out matters.** A 5-baud address byte holds the line low for 200 ms per bit (up to 400 ms for two consecutive zero bits), and the brief requires arbitrary low pulses of 25-200 ms. LIN transceivers with "TXD dominant time-out" release the bus after a few tens of milliseconds (ATA6663: typ 55 ms), so they cannot produce these waveforms. TI says of its K-line capable part that "it does not have dominant state time-out" and supports "speeds down to 0 Baud"; Atmel says the ATA6664 differs from the ATA6663 only in that the time-out is disabled "so the device is able to send a static low signal" (HIGH, [TI SN65HVDA195-Q1](https://www.ti.com/product/SN65HVDA195-Q1); [Atmel ATA6663/ATA6664 datasheet](https://www.mouser.com/datasheet/2/268/atmel-9146-lin-networking-ata6663-ata6664_datashee-1368894.pdf)).

**Tester pull-up.** "The standard specifies 510 ohm"; the ELM327 reference uses 510 ohm 1/2 W on both K and L lines because "a short at 13.8V causes about 0.4W dissipation" (HIGH, [ELM327 datasheet p.79](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf)). The L9637 application circuit draws the diagnostic tester with a 510 ohm pull-up, and the ScanTool/SparkFun reference uses 510 ohm 0.5 W (HIGH, [L9637 datasheet](https://www.st.com/resource/en/datasheet/l9637.pdf), [SparkFun OBD-II UART schematic](https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf)).

| Part | Maker | Lifecycle / availability (Nov 2026) | Operating supply | Bus pin rating | TXD dominant time-out | Speed limit | Pull-up and other notes | Conf. / source |
|---|---|---|---|---|---|---|---|---|
| **L9637D** | ST | Active (ST lists E-L9637D013TR as Active; Octopart "Production", about US$1.1; listed by LCSC as C153038, stock not shown) | VS 4.5-36 V (40 V transients); VCC 3-7 V | K and LI: -24 V to VS; reverse battery to -24 V; ESD only +/-2 kV HBM | **None** (no timer in feature list or block diagram) | fTX-K min 50 kHz, typ 100 kHz with 510 ohm / 1.3 nF; rise/fall 2 us typ | External K pull-up must be <= 5 kohm; internal pull-ups only on TX (20 k), RX and LO (10 k) to VCC; thresholds 0.45/0.55 x VS; K current limit 60 mA typ; ISOFF 120 uA, standby < 1 uA when VCC is removed; separate L-line comparator LI -> LO; K input current 4 uA typ when not transmitting | HIGH datasheet facts, MEDIUM status: [datasheet](https://www.st.com/resource/en/datasheet/l9637.pdf), [ST product page](https://www.st.com/en/automotive-analog-and-power/l9637.html), [Octopart](https://octopart.com/e-l9637d013tr-stmicroelectronics-712929) |
| **TLIN1027-Q1** | TI | Active; SOIC-8 and VSON-8 | 4-36 V | LIN +/-45 V; HBM +/-8 kV on LIN; rated against ISO 7637-2 pulses 1/2/3a/3b (-100/75/-150/100 V) | **None** ("LIN Transceiver without Dominant State Timeout", "Supports ISO 9141 (K-Line)") | Transmit 20 kbit/s (10.4 kbit/s duty cycles specified); receiver to 100 kbit/s | Internal pull-up 45 kohm (20-60) through a diode to VSUP; tester needs external resistor + series diode (datasheet shows 1 kohm for LIN commander; use 510 ohm for K-line); thresholds 0.4/0.6 x VSUP; bus current limit 40-200 mA; sleep 8 uA typ, normal recessive 300 uA typ; RXD open-drain; TXD and EN have internal pull-downs; logic VIH 2 V | HIGH: [product page](https://www.ti.com/product/TLIN1027-Q1), [datasheet SLLSF58B](https://www.ti.com/lit/ds/symlink/tlin1027-q1.pdf) |
| SN65HVDA195-Q1 | TI | Active, but TI flags "a newer version ... TLIN1027-Q1 ... same functionality with different pin-out" | 7-27 V | LIN -40 to +40 V | **None** ("it does not have dominant state time-out"; "speeds down to 0 Baud") | 20 kbit/s LIN | "Supports ISO9141 (k-line)"; external pull-up + diode for master | HIGH: [TI](https://www.ti.com/product/SN65HVDA195-Q1) |
| **TJA1027** | NXP | Active; SO8 (T/20) and HVSON8 (TK/20) | VBAT 5-18 V operating; limiting -0.3 to +42 V | LIN +/-42 V; +/-8 kV IEC 61000-4-2 | **None** (only an "initial TXD dominant check"; NXP AH1103: the time-out "is not provided by the TJA1027") | Up to 20 kBd | "K-line compatible"; integrated slave termination, master needs external resistor + diode; sleep 7 uA typ; 3.3 V/5 V logic; same pinout as TJA1020/1021/1029 | HIGH: [datasheet rev.3](https://www.nxp.com/docs/en/data-sheet/TJA1027.pdf), [NXP page](https://www.nxp.com/products/interfaces/automotive-lin-solutions/lin-2-2a-sae-j2602-transceiver:TJA1027), [AH1103](https://community.nxp.com/pwmxy87654/attachments/pwmxy87654/mpc5xxx/3714/1/AH1103_v3%200_TJA1021_22_24_27_29%20LIN%20transceiver.pdf) |
| TJA1029 | NXP | Listed by NXP | n/c | n/c | **Yes** ("with TXD Dominant Timeout") | 20 kBd | Same footprint as TJA1027 - easy to buy the wrong one | HIGH: [NXP](https://www.nxp.com/pip/TJA1029) |
| TJA1021 | NXP | not checked | n/c | n/c | **Yes** | 20 kBd | | MEDIUM: [Mouser overview](https://www.mouser.co.za/new/nxp-semiconductors/nxp-tja1021-transceivers) |
| SN65HVDA100-Q1 | TI | Active (successor TLIN1021A-Q1) | 5-27 V | LIN -27 to +45 V | **Yes** ("TXD dominant state time-out protection") | 20 kbit/s | Claims ISO 9141 support but unusable for 5-baud/long pulses. OBD9141 worked with it only for standard traffic | HIGH: [TI](https://www.ti.com/product/SN65HVDA100-Q1) |
| ATA6663 | Microchip (ex Atmel) | status not verified | n/c | n/c | **Yes**, typ 55 ms | 20 kBaud | | HIGH: [datasheet](https://www.mouser.com/datasheet/2/268/atmel-9146-lin-networking-ata6663-ata6664_datashee-1368894.pdf) |
| ATA6664 | Microchip (ex Atmel) | status not verified | n/c | n/c | **None** ("able to send a static low signal to the LIN bus") | 20 kBaud | Identical to ATA6663 otherwise | HIGH: same datasheet |
| MCP2003B | Microchip | listed at LCSC (status and stock not verified) | 5.5-30 V (distributor listing) | n/c | **Yes** ("TXD timer expiration"; duration not verified) | 20 kBaud | | MEDIUM: [datasheet listing](https://www.radiolocman.com/datasheet/data.html?di=479539), [LCSC](https://www.lcsc.com/product-detail/C635011.html) |
| MCP2021A | Microchip | not verified | n/c | n/c | not verified | n/c | LIN transceiver with on-chip regulator; no K-line evidence found | LOW |
| TLE7259-3 | Infineon | status not verified | not verified | n/c | **Yes** ("TxD ... dominant time-out function") | 20 kBaud | | MEDIUM: [Infineon](https://infineon.com/cms/en/product/interface/automotive-transceiver/automotive-lin-transceivers/tle7259-3ge/) |
| MC33290 | NXP | **Discontinued** (page archived: "no longer manufactured") | n/c | n/c | n/c | n/c | Only in legacy designs (OBD9141, muki01) | HIGH: [NXP](https://www.nxp.com/products/interfaces/in-vehicle-network/lin-transceivers-and-system-basis-chips/iso-k-line-serial-link-interface:MC33290) |
| MC33660 | NXP | **End of life**: NXP "No Longer Manufactured"; DigiKey obsolete / last-time-buy; Mouser end-of-life with residual stock | 8.0-18 V | n/c | not mentioned | n/c | Do not design in | HIGH (EOL), MEDIUM (stock): [NXP](https://www.nxp.com/products/interfaces/in-vehicle-network/lin-transceivers-and-system-basis-chips/iso-k-line-serial-link-interface:MC33660), [DigiKey](https://www.digikey.co.nz/en/products/detail/nxp-usa-inc/MC33660EF/3524281) |
| Si9241A / Si9243A | Vishay | Legacy: Si9243AEY "No Longer Manufactured" at Farnell; Si9241AEY unconfirmed; Vishay still lists both | n/c | n/c | n/c | "200kB typical" (Vishay table) | Avoid for a new design | LOW-MEDIUM: [Vishay](https://www.vishay.com/applications/automotive/diesel_ecuandfuelinjectordrivers/canbus/iso9141_bustransceiver), [Farnell](https://il.farnell.com/vishay/si9243aey/ic-transceiver/dp/1652279) |

n/c = not checked in this session.

**Discrete and comparator circuits (all proven, no time-out by construction):**

| Circuit | Transmit | Receive | Source / conf. |
|---|---|---|---|
| ELM327 reference (Fig. 9) | NPN 2N3904 (Q7 for K on pin 7, Q6 for L on pin 15), 2.2 kohm base resistors, 510 ohm 1/2 W pull-ups to Vbat | 47 kohm / 33 kohm divider into a Schmitt input; thresholds typ 7.0 V rising / 3.6 V falling | HIGH: [ELM327 datasheet p.79-81](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf) |
| ScanTool/SparkFun STN1110 reference (OBD-II UART v13) | MMBT2222 with 1 kohm base resistor per line, 510 ohm 0.5 W pull-ups to VBAT | LM339 comparator: K-line through 10 kohm against VBAT/2 (10 k/10 k divider), 10 kohm pull-up to 3.3 V on the output | HIGH: [schematic sheet 3](https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf) |
| muki01 reader | Offers transistor, LM393-comparator, L9637D, MC33290, Si9241 and SN65HVDA195 variants | - | HIGH: [repo](https://github.com/muki01/OBD2_K-line_Reader) |
| KKL cables | Reported to contain "LM339 or L9637" behind the USB-serial chip | - | LOW: [TunerPro forum](https://www.tunerpro.net/forum3/viewtopic.php?p=23029) |
| Optocoupler isolation | The HondaECU (motorcycle) project is reported to recommend an optocoupler schematic with an FTDI adapter; its README could not be fetched (404) | - | LOW |

*Design note - recommendation.* First choice for the pin-7 K-line channel: **L9637D** (purpose-built, widest baud margin if reflashing turns out to use more than 20 kbit/s, 3.3 V logic, spare comparator usable as a high-impedance monitor for another 12 V line, standby under 1 uA if VCC is switched). Add an external TVS because its ESD rating is only 2 kV. Second choice: **TLIN1027DRQ1**, with **TJA1027T/20** as an alternate on the same standard LIN SOIC-8 pinout (1 RXD, 2 EN/SLP_N, 4 TXD, 5 GND, 6 LIN, 7 VSUP; pin 8 is NC on the TI part and INH on the NXP part). Whatever the part, the 510 ohm pull-up goes through a P-MOSFET (plus series diode) so firmware can remove it for sniffing. Do not attach any 12 V-referenced LIN transceiver to an unknown or 5 V line: its internal 45 kohm pull-up to VSUP and its ratiometric thresholds (0.4/0.6 x VSUP) make it both intrusive and deaf there.

### 2. Microcontroller platform candidates

Requirements derived from the brief: at least two UART-like channels at 9600/10400 baud plus 5-baud bit-banging and millisecond pulses; edge/byte timestamps with resolution well under one bit time (96 us at 10.4 kbaud); USB device; BLE and preferably Wi-Fi.

| Criterion | RP2350 (Pico 2 W) / RP2040 (Pico W) | ESP32-S3 | ESP32-C6 | STM32 (e.g. G0B1, F411, WB55) + radio | nRF52840 |
|---|---|---|---|---|---|
| Hardware UARTs | 2, plus PIO: 12 state machines on RP2350 (8 on RP2040), each can be a UART TX or RX at any baud, with inversion (HIGH/MEDIUM: [Pimoroni listing](https://shop.pimoroni.com/en-us/products/raspberry-pi-pico-2-w), [Tom's Hardware review](https://www.tomshardware.com/raspberry-pi/raspberry-pi-pico-2-w-review)) | 3 (MEDIUM: [atomic14 comparison](https://esp32.atomic14.com/compare/esp32-c6-mini-1-vs-esp32-s3-mini-1/)) | 2 (+ low-power UART) (MEDIUM, same source) | 3 to 8 depending on part (MEDIUM, general knowledge) | 2 UARTE (MEDIUM, general knowledge) |
| Arbitrary baud, break, long pulses | Fractional dividers; pulses by switching the pin to GPIO or by a PIO program (MEDIUM) | Fractional divider; break API; RMT peripheral for exact pulses (MEDIUM) | Same as S3 (MEDIUM) | Fractional BRR; LIN break generation/detection; single-wire half-duplex mode; pin inversion (MEDIUM) | Programmable baud register (non-standard rates to be verified); pulses via GPIO (LOW-MEDIUM) |
| Microsecond timestamps | 1 us hardware timer; PIO + DMA raw edge capture at MHz rates independent of CPU load (MEDIUM) | Hardware capture via RMT/MCPWM; ISR-level timestamps jitter under Wi-Fi/BLE load (MEDIUM) | As S3 | Timer input capture, 1 us or better, deterministic (MEDIUM) | GPIOTE + PPI + TIMER capture, hardware-timed (MEDIUM) |
| USB device | Yes, full speed, TinyUSB composite (MEDIUM) | Yes, USB-OTG full speed plus built-in USB-Serial/JTAG (MEDIUM) | Only fixed-function USB Serial/JTAG, no custom device classes (MEDIUM) | Yes on most families; crystal-less on G0B1/F0x2/L4 (MEDIUM) | Yes, full speed (MEDIUM) |
| BLE / Wi-Fi | CYW43439: 2.4 GHz Wi-Fi 4 + Bluetooth 5.2 (HIGH: [CNX Software](https://www.cnx-software.com/2024/11/25/7-raspberry-pi-pico-2-w-board-2-4-ghz-wifi-4-bluetooth-5-2-wireless-module/)) | Wi-Fi 4 + BLE 5; **no Bluetooth Classic** (MEDIUM) | Wi-Fi 6 + BLE 5.3 + 802.15.4; no Classic (MEDIUM) | None on chip except WB series (BLE only); otherwise add a module (MEDIUM) | BLE 5 / Thread / Zigbee; **no Wi-Fi** (MEDIUM) |
| Toolchain | Pico SDK on **GNU Arm Embedded (already installed)**; UF2 drag-and-drop; SWD via a second Pico | ESP-IDF with its own Xtensa GCC (separate install); very large community, existing OBD firmware (WiCAN, OpenDiag) | ESP-IDF with RISC-V GCC (separate install) | **GNU Arm Embedded (already installed)**, STM32Cube/libopencm3; most mature debug tooling | nRF Connect SDK/Zephyr (heavier) or legacy nRF5 SDK with GNU Arm |
| Cost / availability | Pico 2 W US$7 launch price, about US$7-9 retail (HIGH: [CircuitDigest](https://circuitdigest.com/news/from-pico-2-to-pico-2-w-raspberry-pi-pico-2w-launched-at-7), [SparkFun](https://www.sparkfun.com/raspberry-pi-pico-2-w-with-headers.html)) | Dev board roughly US$8-15 (LOW, estimate) | Roughly US$6-10 (LOW, estimate) | Nucleo roughly US$11-20, "Black Pill" roughly US$5 (LOW, estimate) | Dongle/XIAO roughly US$10, DK roughly US$50 (LOW, estimate) |

*Design note - recommendation.* Use the **Raspberry Pi Pico 2 W (RP2350)** for the breadboard prototype and, as a castellated module, on the first custom PCB. Reasons: it is the only candidate that gives the installed GNU Arm toolchain, USB, BLE and Wi-Fi in one US$7 part, and PIO solves the two hard problems cleanly (bit-exact 5-baud/pulse generation and multi-channel passive edge capture with hardware timing). Keep **ESP32-S3** as the documented alternative: better wireless stack and reusable open firmware, at the cost of a second toolchain and softer real-time behaviour. An STM32 plus BLE module gives the best determinism and sleep current but the most integration work. The nRF52840 has no Wi-Fi and only two UARTs. If compatibility with legacy Android ELM327 apps (Bluetooth Classic serial port) ever matters, note that ESP32-S3/C6 cannot provide it.

### 3. Power from DLC pin 16

**Facts**

- Pin 16 is permanent battery power; on the sibling Jazz/Fit the manual traces it to "the No. 8 (15A) fuse in the under-hood fuse/relay box" (MEDIUM for the Civic - fuse number not verified for the 7th-gen Civic: [Jazz/Fit DLC circuit troubleshooting](https://www.hondafitjazz.com/manual/A00/HTML/97/SAA2E97A20326919461FAAT00.HTML)).
- Load dump moved from ISO 7637-2 to ISO 16750-2 in 2011; "Test A" (no central suppression) reaches about 100 V on 12 V systems, "Test B" (central suppression) is limited to 35 V, alternator source resistance 0.5-4 ohm; reverse-voltage test is 14 V for 60 s; an over-voltage test applies 18 V for 60 min (HIGH for the load-dump split and 35 V figure, MEDIUM for the rest: [Analog Devices LTC4380 article](https://www.analog.com/en/resources/technical-articles/low-quiescent-current-surge-stopper-robust-automotive-supply-protection.html), [Intertek ISO 16750-2 overview](https://w3inte.intertek.ch/automotive/standards/iso-16750-2/)). One overview also describes a supply dip to 4.5 V for 100 ms (LOW-MEDIUM, same overviews).
- 24 V jump-start for 60 s and cold-crank profiles dipping to roughly 3-6 V are also part of ISO 16750-2 (MEDIUM, general knowledge, not re-verified).
- ISO 7637-2 switching transients as applied to transceiver pins: pulse 1 -100 V, pulse 2 +75 V, pulse 3a -150 V, pulse 3b +100 V (HIGH as test levels quoted by TI: [TLIN1027-Q1 datasheet section 6.3](https://www.ti.com/lit/ds/symlink/tlin1027-q1.pdf)).
- ELM327 reference: 1N4001 series diode and a 7805; "base current = 29.8 mA", 17.6 mA in low-power mode, and with regulator/CAN changes "less than 1 mA"; the datasheet warns that a typical circuit "draws about as much current as a dome lamp, so can not be left on for long" (HIGH: [ELM327 datasheet p.85-86](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf)).
- Other reported currents: WiCAN Pro about 35 mA running / 2.8 mA asleep, 6.5-18 V input (MEDIUM, single review: [CNX Software](https://www.cnx-software.com/2024/08/22/wican-pro-esp32-s3-powered-obd-scanner-for-vehicle-diagnostics-with-smart-home-integration/)); Freematics ONE+ about 10 mA with radios off (MEDIUM: [Freematics](https://freematics.com/products/freematics-one-plus-model-b/)).

**Reference input stages seen in open hardware**

| Design | Chain from pin 16 | Source / conf. |
|---|---|---|
| Macchina M2 interface board | PTC 2.6 A hold -> SMBJ14A TVS -> 60 ohm ferrite -> 10 uF/0.1 uF/0.01 uF -> B340LA Schottky (reverse polarity) -> P-MOSFET IRFTS9342 switched off by a 16 V zener + PNP (over-voltage cut-off) -> TPS54328 buck to 5 V; separate MOSFET-switched 12 V/5 V rails for the bus circuits ("Low Power Control") | HIGH (schematic read): [M2 Interface Board Schematic sheet 3-4](https://github.com/macchina/m2-hardware/blob/master/M2/Interface%20Board/Interface%20Board%20Schematic.pdf) |
| OpenDiag rev 1.1 (2025) | PTC 500 mA / 30 V -> P-MOSFET reverse-polarity stage with gate zener -> buck to 5 V -> AMS1117-3.3; TVS arrays (NUP2105L) on bus pins | MEDIUM (small print; part numbers approximate): [opendiag.pdf](https://github.com/maximus64/opendiag-hardware/blob/main/kicad/opendiag.pdf) |
| ELM327 reference | 1N4001 -> 7805 | HIGH (above) |
| SparkFun OBD-II UART | MBRA140 Schottky -> linear 5 V and 3.3 V regulators | HIGH: [schematic sheet 2](https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf) |

*Design note - proposed power chain.*

1. Fuse: resettable PTC of 0.5-0.75 A hold rated at least 30 V (60 V preferred), or a 1 A SMD fuse.
2. Transient suppressor: SMBJ33A/SMBJ33CA class (600 W). This covers suppressed load dump, jump start and ISO 7637-2 pulses but not an unsuppressed 100 V "Test A" load dump (that needs an SM8S-class device). Acceptable for a tool that is not permanently installed; state it as a known limit. A 14 V stand-off TVS as on the M2 starts conducting near charging voltage and relies on the PTC, so it is not recommended.
3. Reverse polarity: series Schottky (simple, 0.4 V loss) or P-MOSFET with gate zener as on OpenDiag.
4. Regulator: a buck rated 40-60 V input with low quiescent current when Wi-Fi is used (current peaks of several hundred mA would burn roughly 1 W or more in an LDO from 14 V); an automotive low-Iq LDO (40-45 V class) is adequate for an STM32/nRF-only build.
5. Cranking: bulk capacitor after the reverse-polarity element; all bus drivers must default to OFF during brown-out and reset (pull-downs on every gate and on transceiver enable pins; the TLIN1027 already pulls EN low internally).
6. Sleep: budget under 1 mA total, ideally under 200 uA: switch off the 510 ohm pull-up paths and bus circuits, put the transceiver in sleep (TLIN1027 8 uA; L9637 under 1 uA with VCC removed, otherwise 120 uA), use megohm-range battery-sense dividers. A 30 mA always-on device consumes about 22 Ah per month, which is a large part of a small-car battery; 1 mA is about 0.7 Ah per month.
7. Battery sense: resistor divider to an ADC with a 0.1 uF filter (ELM327 uses 47 k/10 k; SparkFun 62 k/10 k; M2 analogue inputs 10 k/2.2 k with 16 V varistors).

### 4. Open hardware and commercial prior art

| Item | What it is (as verified) | K-line path | Honda pins 9 / 12 / 14 | Usable today for Honda protocol work? | Conf. / source |
|---|---|---|---|---|---|
| **FTDI-based KKL 409.1 cable** | USB-serial + level shifter ("LM339 or L9637" reported); sold with FT232RL or CH340 | Pin 7 (and L on 15) | None | **Yes, day one**: PC software controls baud, break and 5-baud by bit-banging; pulse timing depends on USB frames (1 ms) and operating-system scheduling jitter (several ms), and one forum user could not produce a 65-74 ms Honda wake-up pulse this way and drove the cable's level shifter from an Arduino instead. FTDI adapters are the usual recommendation for non-standard rates such as 10400 baud. Roughly US$10-15 (LOW, estimate) | LOW-MEDIUM: [TunerPro forum](https://www.tunerpro.net/forum3/viewtopic.php?p=23029), [kw1281test](https://github.com/gmenounos/kw1281test/issues/35) |
| **Pico 2 W + L9637D on breadboard** | Not a product; see BOM (a) | Any pin | Any, via discrete parts | **Yes, recommended first build** | *Design note* |
| iwanders/OBD9141 | Arduino/Teensy library; developed with "MC33290, SN65HVDA100 and SN65HVDA195" using datasheet circuits; Saleae captures in the repo | Pin 7 | None | Code and timing reference for ISO 9141-2/KWP init | HIGH: [README](https://github.com/iwanders/OBD9141) |
| muki01/OBD2_K-line_Reader | ESP32/Arduino reader with six interface schematics; 10.4 kbaud; pins 7, 16, 4/5; 47 k/10 k battery divider | Pin 7 | None | Schematic reference | HIGH: [README](https://github.com/muki01/OBD2_K-line_Reader) |
| ELM327 datasheet circuit | Discrete reference design, fully documented | Pins 7 and 15 | None | Circuit reference only; an ELM327 always wraps traffic in legislated ISO 9141/KWP framing (MEDIUM, general knowledge) | HIGH: [datasheet](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf) |
| SparkFun OBD-II UART (WIG-09555) | STN1110 + MCP2551, "Design by ScanTool.net and SparkFun", DE9 connector, US$62.50, in stock | Pins 7/15 via DE9 cable | None (DE9 cable has no conductors for them) | Only for legislated OBD sanity checks | HIGH: [product page](https://www.sparkfun.com/products/9555), [schematic](https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf) |
| STN2120 | OBD interpreter IC; vendor page: "This product is discontinued and no longer available for purchase" | - | - | No | HIGH: [OBD Solutions](https://www.obdsol.com/solutions/chips/stn2120/) |
| WiCAN Pro (MeatPi) | ESP32-S3-WROOM-1-N16R8 + separate closed OBD interpreter chip on a UART; US$89 | Through the interpreter chip | None found | Only legislated OBD; raw K-line timing is not exposed | MEDIUM: [CNX Software](https://www.cnx-software.com/2024/08/22/wican-pro-esp32-s3-powered-obd-scanner-for-vehicle-diagnostics-with-smart-home-integration/), [Crowd Supply](https://www.crowdsupply.com/meatpi-electronics/wican-pro) |
| Freematics ONE+ Model B | ESP32 + co-processor; lists CAN and "KWP2000 Fast and 5 Kbps" | Via co-processor | None found | Not suitable | MEDIUM: [Freematics](https://freematics.com/products/freematics-one-plus-model-b/) |
| **Macchina M2** (under-the-dash) | SAM3X processor board + interface board; CC BY-SA 4.0; BOM: 2x TJA1027TK/20, 2x TJA1051, TH8056, MCP2515, TPS54328 | TJA1027 channel 1 on pin 7, channel 2 on pin 15; **no external pull-up fitted** (R124/R125 "DNI") | Pins 8, 9, 12, 13 go only to test points; pin 14 is CAN-L of a TJA1051 | Partly: needs an added 510 ohm pull-up and bodge wires for pin 9; pin 14 is occupied by a CAN transceiver | HIGH: [schematic](https://github.com/macchina/m2-hardware/blob/master/M2/Interface%20Board/Interface%20Board%20Schematic.pdf), [BOM](https://raw.githubusercontent.com/macchina/m2-hardware/master/M2/Interface%20Board/BOM-PCB-01016%20R1.csv), [docs](https://docs.macchina.cc/m2-docs/detailed-reference/interfaces.md) |
| Macchina A0 | Not researched (search budget exhausted) | - | - | Unknown | - |
| **OpenDiag** (maximus64, rev 1.1, 2025) | ESP32-S3, 2x CAN, TJA1027 K-line, J1850, "six high-side drivers (OBD pins 6, 9, 11, 12, 13, 14) and one low-side driver (OBD pin 15)" fed by an adjustable boost converter; CERN-OHL-S hardware, GPLv3 firmware; US$60 on Tindie | TJA1027 on pin 7 | High-side (source) only on 9, 12, 14; cannot sink pin 9 to ground; no UART path to pin 14 | Closest ready-made board; firmware is open, but SCS grounding and a pin-14 data channel need hardware changes | HIGH (README), MEDIUM (schematic details): [hardware repo](https://github.com/maximus64/opendiag-hardware), [Tindie](https://www.tindie.com/products/opendiag/opendiag-open-source-obd2-interface/) |
| comma.ai panda | Current master Python library contains no K-line/LIN functions (checked: no `kline` identifiers), so present-day pandas are not a K-line platform; white/grey panda schematics could not be retrieved | - | - | No | MEDIUM: [panda python/__init__.py](https://github.com/commaai/panda/blob/master/python/__init__.py) |
| Tactrix Openport 2.0 | J2534 pass-thru; "As of 2026, the Openport 2.0 is no longer in production"; Openport 1.3U US$317 (Feb 2026); clones are risky | ISO 9141/14230 K-line | Not verified | Only if one is already owned; useful to run J2534 software while sniffing | MEDIUM: [HP Academy](https://www.hpacademy.com/technical-articles/tactrix-openport-2-0-alternatives/) |
| Honda HDS HIM (genuine or clone) | Dealer interface; RS-232 to the PC through an RJ45 jack (RJ45 4/5/7 to DE9 2/3/5); clones have two stacked boards, one teardown notes a "Revision A, date 2006-03" label; **no chip-level teardown found** | Unknown | Its 16-pin to 3-pin adapter is reported to use pin 14 as the data wire, pin 16 battery, pin 5 ground, with pins 1 and 3 bridged for cable identification | Best "reference tool" to sniff; must be opened and traced | LOW: [k20a.org](https://www.k20a.org/threads/honda-hds-him-serial-cable-pinout.208962/), [digital-kaos](https://www.digital-kaos.co.uk/forums/showthread.php/157964-Honda-HIM-3pin-cable), [Civic Mk9 blog](https://honda-civic-hatchback-mk9.blogspot.com/2015/07/honda-diagnostic-system-clone.html) |
| Cheap "HDS cable" / MVCI J2534 clones | USB dongles sold for about US$26-32; vendors claim K-line ISO 9141, KWP2000 and CAN and "ARM chip inside"; forum users bridge pin 7 to pin 14 inside to reach ABS/SRS/immobilizer | Pin 7 | Pin 14 only after the user modification | Cheap way to run HDS for sniffing; unknown quality | LOW: [Honda-Tech thread](https://honda-tech.com/forums/engine-management-tuning-124/honda-hds-cheap-mvci-cables-3312290/), [obd2tool listing](https://www.obd2tool.com/goods-707-HONDA+HDS+Cable+OBD2+Diagnostic+Cable.html) |

Order of usefulness before a custom PCB exists: (1) logic analyser + passive tap, (2) FTDI KKL cable, (3) Pico 2 W + L9637D breadboard, (4) a dealer-tool clone to generate reference traffic, (5) OpenDiag if a ready-made ESP32-S3 board with open firmware is wanted.

### 5. Driving Honda's extra lines (SCS, DIAG-H, write enable)

**What is publicly known about the electrical side**

| Line | Evidence | Conf. |
|---|---|---|
| K-line, pin 7 | Honda manual (Jazz/Fit, same ECM connector naming): "Measure the voltage between DLC terminals No. 5 and No. 7. Is there 8.5 V or more?"; the wire runs to ECM/PCM terminal E23. The car therefore pulls the idle K-line up towards battery voltage by itself (which unit provides that pull-up was not established). A 2004 Civic owner measured 14 V on pin 7 (which suggests a running engine). | MEDIUM-HIGH: [Jazz/Fit manual](https://www.hondafitjazz.com/manual/A00/HTML/97/SAA2E97A20326919461FAAT00.HTML), [JustAnswer transcription for 2001 Civic](https://www.justanswer.com/honda/1bezu-no-communication-data-link.html), [hondacivicforum](https://www.hondacivicforum.com/forum/mechanical-problems-technical-chat-8/2004-16p-data-link-connector-troubleshooting-98549/) |
| SCS, pin 9 | Same 2004 Civic owner measured 4.8 V DC on pin 9 (pulled up to about 5 V inside the control units). A CR-V (same platform) manual refers to "the SCS line ... and the No. 9 terminal (BRN wire) of the Data Link Connector (DLC) (16P)" and to DLC No. 4 as the body-ground return. K-series ECM lists E29 = SCS (brown). | MEDIUM: [hondacivicforum](https://www.hondacivicforum.com/forum/mechanical-problems-technical-chat-8/2004-16p-data-link-connector-troubleshooting-98549/), [CR-V manual](http://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_4v.htm), [RSX ECU pinout](https://e-tunez.com/2002-2006-prb-ecu-pin-outs-rsx) |
| DIAG-H | It is a named SAE J2534-2 protocol: "J2534-2/4_0500 Pass-Thru Extended Feature - HONDA_DIAGH" (January 2022, paywalled). Forum descriptions: "bi-directional serial data line connected to the DLC" carrying the 92Hm/2 protocol for non-engine units. | HIGH (document exists), LOW (description): [SAE Mobilus](https://saemobilus.sae.org/content/J2534-2/4_5_00/), [driveaccord thread](https://www.driveaccord.net/threads/accord-2003-obdii-or-can-bus.24332/) |
| DIAG-H pin | "Pin 14 - data" on the HIM 3-pin adapter; "pin 1 or 14" for cars up to about 2005 (Russian forum); "Honda cable needs to have the #14 pin activated". | LOW: [digital-kaos](https://www.digital-kaos.co.uk/forums/showthread.php/157964-Honda-HIM-3pin-cable), [autoprogs.ru](https://autoprogs.ru/diagnostika-honda-remont-elektroniki-262/honda-hds-6430/index37.html), [odyclub](https://www.odyclub.com/threads/hds-and-j2534-pass-thru.349083/) |
| DIAG-H level | "The HDS DIAG Line is a 5v data bus not a 12v data bus as for the standard OBDII K Line" (first-generation Insight project). Contradicting practice: users bridge pin 7 (12 V K-line with tester pull-up) to pin 14 inside J2534 cables and report reading ABS/SRS/immobilizer codes. | LOW, conflicting: [Insight Central](https://www.insightcentral.net/threads/honda-him-hds-project.20427/page-2), [Honda-Tech](https://honda-tech.com/forums/engine-management-tuning-124/honda-hds-cheap-mvci-cables-3312290/) |
| Write enable | K-series ECM lists E30 = WEN "Write Enable Signal" (red/white). One pinout site labels DLC pin 12 "Programming out". A Russian forum post says to ground pins 9, 12 and 15 "for SCS mode in HDS". No voltage level, polarity or timing was found anywhere. | LOW: [RSX ECU pinout](https://e-tunez.com/2002-2006-prb-ecu-pin-outs-rsx), [pinoutguide](https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml), [autoprogs.ru](https://autoprogs.ru/diagnostika-honda-remont-elektroniki-262/honda-hds-6430/index37.html) |
| J2534 pin drivers | A J2534-style interface provides programmable voltage on pins 6, 9, 11, 12, 13, 14 and a short-to-ground on pin 15; OpenDiag implements exactly this set. This explains the forum advice to tie pins 9/12 to pin 15 when a generic pass-thru must ground SCS. | MEDIUM: [OpenDiag README](https://github.com/maximus64/opendiag-hardware) plus general knowledge of J2534-1 |

*Design note - SCS switch.* An N-MOSFET low-side switch (60 V, logic-level) with 47-100 ohm in series plus a small PTC, or a self-protected low-side switch, gate pulled down so it is off during reset. The load is only the control units' 5 V pull-ups (a few mA expected, to be measured). Read-back through a divider into an ADC or comparator serves three purposes: confirm the line really went low, detect that another tool is grounding it (sniffer mode), and detect an unexpected 12 V before switching. Keep the on-state drop well under 1 V.

*Design note - universal single-wire channel.* Because the level of DIAG-H and the nature of pin 12 are unknown, build four identical protected channels instead of dedicated circuits:

- node protection: series PTC (50-100 mA) and a low-capacitance bidirectional TVS (27-33 V);
- open-drain N-MOSFET driver (the same part as the SCS switch);
- three firmware-selected pull-ups through P-MOSFETs: 510 ohm to Vbat (K-line tester), about 1-4.7 kohm to 5 V (5 V bus), none (high impedance);
- comparator receiver with a programmable threshold (DAC or filtered PWM: Vbat/2 for 12 V lines, about 2.5 V for 5 V lines) feeding a UART/PIO input;
- high-value divider to an ADC for idle-level and pull-up characterisation;
- high-impedance (>= 100 kohm) in the unpowered and reset states.

Default mapping: channel A = pin 7 (in parallel with, or replaced by, the L9637D), B = pin 14, C = pin 9 (SCS), D = pin 12; solder jumpers or a header to re-route to pins 1, 3, 13 and 15. This copes with every hypothesis found (5 V or 12 V DIAG-H, DIAG-H on pin 1 or 14, write-enable as a level or as a grounded input) without a board respin. First action on the real car: measure the open-circuit voltage and the current into a 1 kohm resistor to ground on pins 9, 12 and 14 with the ignition on; that gives each line's pull-up voltage and resistance before anything is driven.

### 6. Passive sniffing

- Two testers on one K-line are a real hazard: "Both the ELM327 and the other scan tool would present 510 ohm resistors, so the vehicle would see 255 ohm connected externally. This would very likely cause data errors, and might even damage some circuitry"; the datasheet suggests putting 10 kohm in series with the 510 ohm and bridging it with a jumper only when acting as the tester (HIGH: [ELM327 datasheet p.79](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf)).
- Suitable receive-only taps: the L9637 K or LI input draws 4 uA typ (25 uA max) with thresholds at 0.45/0.55 x VS and built-in hysteresis (HIGH: [L9637 datasheet](https://www.st.com/resource/en/datasheet/l9637.pdf)); an LM339/LM393 comparator behind 10 kohm as in the ScanTool design (HIGH: [SparkFun schematic](https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf)); the ELM327 47 k/33 k divider loads the bus with 80 kohm (HIGH).
- A TLIN1027/TJA1027 is a poor passive tap on anything but a 12 V K-line because of its internal pull-up (TLIN1027: 20-60 kohm to VSUP) (HIGH: [datasheet](https://www.ti.com/lit/ds/symlink/tlin1027-q1.pdf)).
- Timing: one bit is 96 us at 10400 baud and 104 us at 9600 baud. A 1 us timestamp (MCU timer or PIO capture) is more than sufficient; even 10-20 us resolves inter-byte gaps. Wake-up pulses and 5-baud addresses are not UART bytes, so the sniffer should record **raw edges** (time, level) and decode bytes afterwards (*Design note*).
- Logic analysers: OBD9141 ships Saleae captures of real K-line traffic that can serve as a format/timing reference (HIGH: [OBD9141](https://github.com/iwanders/OBD9141)). Cheap 8-channel FX2 analysers accept only logic-level inputs, so they need the comparator or divider front end; a Saleae tolerates 12 V signals directly (MEDIUM, general knowledge).
- Physical access: an OBD Y-cable (one male, two female) or an in-line breakout puts the tap between dealer tool and car. Verify with a continuity meter that all sixteen positions are wired, in particular 7, 9, 12, 14 and 15; many cheap cables populate only the legislated pins (LOW, general knowledge). Breakout boxes with indicator LEDs load each pin and can pull down weak 5 V lines such as SCS; prefer a passive one (*Design note*).

*Design note - sniffer mode of the device.* Pass all sixteen pins straight through between a male plug and a female socket; tap pins 7, 14, 9, 12, 15 and 1 through >= 100 kohm into the comparators; capture with PIO + DMA at 1 MHz or more; emit both a raw edge log and decoded bytes with microsecond timestamps over USB and to microSD. A 10-22 ohm sense resistor in series with the K-line between the two connectors, read by a differential comparator, tells which side is pulling the line low (tester or car), which removes guesswork when both ends speak on one wire. Store captures in a format the REA/TShark tooling can ingest (pcapng with a user DLT, or sigrok files).

### 7. Mechanics, parts and bill of materials

**Connectors and cables**

| Part | Notes | Price | Conf. / source |
|---|---|---|---|
| J1962 male, right-angle, for PCB (generic, e.g. Elecbee EB-020-0038) | Check drawing: PCB-mount versus solder-cup | US$2.32 | MEDIUM: [Elecbee](https://www.elecbee.com/en-52046-truck-diagnostic-j1962m-automobile-obd2-male-obd-16-pin-connector-gold-plated-90°-right-angle-male-shell) |
| Generic PCB-solder J1962 male/female (bulk) | 100-piece minimums | US$0.40-1.30 | LOW: [Alibaba listing](https://www.alibaba.com/product-introduction/OBD-16Pin-Connector-Female-Male-PCB_1600089008531.html) |
| SparkFun OBD-II connector DEV-09911 | Bare male connector | about US$4.50 | MEDIUM: [Little Bird listing](https://littlebirdelectronics.com.au/products/obd-ii-connector.md) |
| Aptiv/Delphi 12110252 | Cable-mount, crimp terminals | GBP 1.42 | MEDIUM: [RS](https://uk.rs-online.com/web/p/products/8010991) |
| Macchina "CCBDM205" | Right-angle male used on the M2 | not priced | HIGH: [M2 BOM](https://raw.githubusercontent.com/macchina/m2-hardware/master/M2/Interface%20Board/BOM-PCB-01016%20R1.csv) |
| OBD-II to DE9 cable (SparkFun style) | DE9 carries GND, CAN-H, K-line, CAN-L, J1850-, J1850+, L-line, Vbat only (per the OBD-II UART schematic) - **no pin 9 or pin 12** | - | HIGH: [schematic sheet 2](https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf) |
| 16-conductor OBD extension, Y-splitter, passive breakout box | Verify full population | roughly US$8-15, US$8-12, US$20-40 | LOW (estimates) |

Enclosure: generic "OBD2 male plug with shell" kits include a housing sized for a small PCB (listings above). Status LEDs: power, K activity, channel-B activity, SCS asserted, wireless link (the ELM327 and OBD-II UART designs use four or five). microSD on SPI for stand-alone logging.

**(a) Breadboard prototype from modules** (prices are 2026 estimates unless a source is given)

| Item | Qty | Est. cost (US$) | Conf. |
|---|---|---|---|
| Raspberry Pi Pico 2 W with headers | 1 | 7-9 | HIGH ([SparkFun](https://www.sparkfun.com/raspberry-pi-pico-2-w-with-headers.html)) |
| Second Pico as SWD debug probe (optional) | 1 | 4-5 | LOW |
| L9637D + SOIC-8 to DIP adapter | 2 | 2-3 each | MEDIUM ([Octopart](https://octopart.com/e-l9637d013tr-stmicroelectronics-712929)) |
| TLIN1027DRQ1 or TJA1027T/20 + adapter (alternate) | 1 | 2 | LOW |
| LM339 or LM393, 2N7002/BSS138, small P-MOSFETs, 510 ohm 0.5 W resistors, resistor/capacitor assortment, TVS diodes | - | 8-10 | LOW |
| 12 V to 5 V buck module, inline fuse holder + 1 A fuse | 1 | 4-5 | LOW |
| OBD 16-conductor extension cable (to cut) or passive breakout box | 1 | 10-40 | LOW |
| OBD Y-splitter (fully wired) | 1 | 8-12 | LOW |
| Breadboard, jumper wires, microSD breakout | - | 10-15 | LOW |
| FTDI-based KKL 409.1 cable (day-one tool) | 1 | 10-15 | LOW |
| 8-channel 24 MHz logic analyser (sigrok-compatible) | 1 | 10-15 | LOW |
| **Total** | | **about 75-130** | LOW |

**(b) Custom PCB, first batch of five** (estimates)

| Item | Est. cost per board (US$) | Conf. |
|---|---|---|
| Pico 2 W module (castellated) or ESP32-S3-WROOM-1 | 5-7 | MEDIUM |
| L9637D, optional TLIN1027 footprint, LM339 x2, MOSFETs, load switches | 4-5 | LOW |
| 40-60 V buck regulator with inductor, 3.3 V rail | 3-4 | LOW |
| PTC, TVS, reverse-polarity MOSFET, bus TVS arrays | 2-3 | LOW |
| J1962 male with shell, optional female socket for pass-through | 3-6 | MEDIUM |
| USB-C, ESD protection, microSD socket, LEDs, button | 2-3 | LOW |
| Passives, test points, headers | 2 | LOW |
| Two- or four-layer PCB, five pieces, plus shipping | 2-6 | LOW |
| **Parts per board** | **about 23-36** | LOW |
| One-off assembly service set-up for the batch (optional) | 30-60 total | LOW |

### 8. Bench set-up for a spare ECM

- The 2001-2005 engine computers use five connectors: A (31 pins), B (24), C (22), D (17), E (31); A, B and E suffice for a manual-transmission ECM (LOW-MEDIUM: [BoostedK20 forum](http://www.boostedk20.com/forums/showthread.php/1127-ECU-Pinout-and-Mil-Spec-Part-Numbers)).
- K-series ECM pins (2002-2006 RSX list; expected to match the EP3 Civic Si, **not verified for D17 engines**):

| Function | Pin | Wire colour | Note |
|---|---|---|---|
| IGP1 / IGP2 (switched +12 V from the PGM-FI main relay) | A3 / A2 | yellow/black | Feed directly from the bench supply |
| PG1 / PG2 (power ground) | A5 / A4 | black | |
| LG1 / LG2 (logic ground) | A9 / A8 in the wire list, A24 / A23 in the table | brown/yellow | Source conflicts with itself; verify |
| IG1 (ignition-switched input) | E9 | black/yellow | +12 V = "ignition on" |
| MRLY (main relay control output) | E7 | red/yellow | Not needed when IGP is fed directly |
| K-line | E23 | light blue | To DLC pin 7 |
| SCS | E29 | brown | To DLC pin 9 |
| WEN (write enable) | E30 | red/white | DLC destination unverified |
| IMOCD (immobilizer code) | E27 | red/blue | |
| MIL (lamp output) | E31 | green/orange | LED + resistor to +12 V shows blink codes |

  (MEDIUM, community pinout: [e-tunez](https://e-tunez.com/2002-2006-prb-ecu-pin-outs-rsx). The Jazz/Fit manual independently gives K-line = E23 and DLC pin 5 = ECM E3: [manual](https://www.hondafitjazz.com/manual/A00/HTML/97/SAA2E97A20326919461FAAT00.HTML). One 2005 Civic EX pin list places "23 - lt blu - K line" under connector B - LOW, conflicting.)
- Needed on the bench (*Design note*): a current-limited 12-13.8 V supply (limit 1-2 A) with a fuse; ECM connector pigtails cut from a donor harness (or individual terminals); a J1962 female socket wired as a bench DLC (16 = +12 V, 4 and 5 = ground, 7 = K-line, 9 = SCS, further pins as the wiring diagram dictates); the interface's own 510 ohm K-line pull-up; an LED on the MIL output.
- Expected behaviour: with the interface's 510 ohm pull-up the K-line idles near supply voltage (the in-car manual check expects 8.5 V or more), and the ECM should answer diagnostics while logging many sensor/immobilizer faults, which is useful for exercising DTC read/clear. Immobilizer-dependent functions need the immobilizer unit and key. ABS/SRS units can be bench-powered the same way for DIAG-H work, never with real inflators connected.
- Commercial bench harnesses (Hondata, KTuner and similar) were not researched because the search budget ran out.

## Conflicts and uncertainties

1. **DIAG-H voltage level**: "5 V data bus" (Insight Central) versus successful bridging of pin 14 to a 12 V K-line cable (Honda-Tech). Both are single forum sources about different models. Hardware must start in high-impedance/5 V mode and measure first.
2. **DIAG-H pin**: pin 14 (HIM adapter, odyclub) versus "pin 1 or 14" (autoprogs.ru); pinoutguide.com shows pin 14 as CAN-low and a question-marked "13 or 14" for immobilizer control. Which pins are populated on the actual 2002 Civic is unknown from this dimension's sources.
3. **How the dealer tool grounds SCS**: CR-V manual implies pin 9 to pin 4; a Russian forum advises grounding pins 9, 12 and 15 together (probably an adaptation for J2534 devices whose only ground switch is pin 15).
4. **L9637D lifecycle**: one distributor aggregator shows "Obsolete" for the non-"E-" order code while ST lists the E-L9637D013TR as Active. Order the E- prefixed part.
5. **MC33660**: NXP and DigiKey say discontinued/last-time-buy, Mouser snapshots still show stock with deliveries expected in 2026. Treat as unavailable.
6. **SN65HVDA195**: the features text says no dominant time-out; a search summary claimed the block diagram shows a time-out block. The TI product page statement is explicit, and TI's successor TLIN1027 is unambiguous, so prefer the TLIN1027.
7. **ESP32-S3 USB speed**: one comparison site calls it "high-speed OTG"; Espressif's own material describes full-speed USB-OTG. Irrelevant at these data rates.
8. **K-line pin on the D17 ECM**: E23 (Honda-Tech post, K-series list, Jazz/Fit manual) versus connector B pin 23 (one 2005 Civic EX list).
9. **Macchina M2 as a K-line tester**: the schematic shows no master pull-up fitted; whether it communicates reliably with ECMs that expect the tester's 510 ohm was not tested by anyone found.
10. **OpenDiag part numbers** were read from a low-resolution schematic export and are approximate.

## Open questions

| # | Question | How to answer |
|---|---|---|
| 1 | Idle voltage, pull-up resistance and logic thresholds of DIAG-H on this car | Bench/car measurement: multimeter and oscilloscope on pin 14 (and pin 1), ignition on, nothing attached; then current into 1 kohm to ground. Buy and read SAE J2534-2/4_0500 for the specified physical layer. |
| 2 | Which DLC cavities are populated on the owner's car (1, 3, 9, 12, 13, 14, 15) and where they go | Inspect the DLC; read the body/PGM-FI wiring diagram pages of the 2001-2005 Civic service manual. |
| 3 | SCS pull-up current and the low level the units require | Measure on the car; start with a 100 ohm series resistor. |
| 4 | Does a write-enable line reach the DLC, what level does the dealer tool apply, and when | Wiring diagram; sniff an HDS/HIM ECM update with all pins tapped; reverse engineer HDS update modules (REA target). |
| 5 | Baud rates and pulse-width tolerances Honda uses (wake-up pulse, reflash speed above 20 kbit/s?) | Sniff the dealer tool with raw edge capture; determines whether a 20 kbit/s LIN-type transceiver is enough. |
| 6 | What is inside an HDS HIM clone and a cheap "HDS cable": MCU, bus drivers, which J1962 pins are driven | Buy one, photograph and trace the boards, dump the firmware (REA/Ghidra target). |
| 7 | Whether HDS in J2534 mode requires HONDA_DIAGH and pin-switch features for this car, and how it commands SCS | Read Honda's J2534 requirements page (techinfo.honda.com/Rjanisis/pubs/web/RJAAI001_J2534.htm - TLS error from this tool); reverse engineer the HDS J2534 calls (REA target). |
| 8 | Exact ECM connector pins for the owner's engine (D17 versus K20), including back-up power and immobilizer | Service manual ECM terminal tables; continuity test on a donor harness. |
| 9 | Whether cheap OBD Y-cables and breakout boxes pass all 16 pins and whether LEDs load them | Continuity and leakage test before use. |
| 10 | Real sleep current of a Pico 2 W based design | Bench measurement on the breadboard prototype. |
| 11 | comma.ai white/grey panda K-line driver circuit and Macchina A0 details | Fetch the old panda hardware guide and A0 schematic when search budget is available (low priority). |

## Notes for the REA phase

- HDS/i-HDS installation: the HIM driver and J2534 client code should reveal pin numbers, programming-voltage/short commands, DIAG-H parameters and timing constants (Ghidra).
- J2534 vendor DLLs that implement HONDA_DIAGH (Bosch MVCI, Denso DST-i, Drew Technologies) for the physical-layer configuration (Ghidra).
- Firmware of an HDS HIM clone or cheap HDS cable; one forum teardown of a related J2534 clone mentions an FT232R plus NXP LPC2148 (LOW) (Binwalk/Ghidra).
- Bus captures from the sniffer: design the capture format (raw edges plus decoded bytes, microsecond timestamps) for TShark/sigrok ingestion from the start.
- OpenDiag firmware (GPLv3, ESP32-S3) and OBD9141 are readable reference implementations of K-line timing; no reverse engineering needed.

## Sources

- https://www.st.com/resource/en/datasheet/l9637.pdf - ST L9637 ISO 9141 interface datasheet (read from a mirror copy, December 1998 edition): ratings, thresholds, 510 ohm tester pull-up.
- https://www.st.com/en/automotive-analog-and-power/l9637.html - ST product page (status Active per search summary).
- https://octopart.com/e-l9637d013tr-stmicroelectronics-712929 - L9637D price/status snapshot.
- https://www.ti.com/product/TLIN1027-Q1 and https://www.ti.com/lit/ds/symlink/tlin1027-q1.pdf - TI LIN/K-line transceiver without dominant time-out.
- https://www.ti.com/product/SN65HVDA195-Q1 - TI K-line capable LIN PHY, no time-out, successor note.
- https://www.ti.com/product/SN65HVDA100-Q1 - TI LIN PHY with TXD dominant time-out.
- https://www.nxp.com/docs/en/data-sheet/TJA1027.pdf and https://www.nxp.com/products/interfaces/automotive-lin-solutions/lin-2-2a-sae-j2602-transceiver:TJA1027 - NXP TJA1027 (K-line compatible, no time-out).
- https://www.nxp.com/pip/TJA1029 - NXP TJA1029 (same footprint, with time-out).
- https://community.nxp.com/pwmxy87654/attachments/pwmxy87654/mpc5xxx/3714/1/AH1103_v3%200_TJA1021_22_24_27_29%20LIN%20transceiver.pdf - NXP application hints AH1103 (TJA1027 versus TJA1029).
- https://www.nxp.com/products/interfaces/in-vehicle-network/lin-transceivers-and-system-basis-chips/iso-k-line-serial-link-interface:MC33290 - MC33290 archived/discontinued.
- https://www.nxp.com/products/interfaces/in-vehicle-network/lin-transceivers-and-system-basis-chips/iso-k-line-serial-link-interface:MC33660 - MC33660 no longer manufactured.
- https://www.digikey.co.nz/en/products/detail/nxp-usa-inc/MC33660EF/3524281 - MC33660 distributor status.
- https://www.mouser.com/datasheet/2/268/atmel-9146-lin-networking-ata6663-ata6664_datashee-1368894.pdf - ATA6663 (time-out 55 ms) versus ATA6664 (none).
- https://www.radiolocman.com/datasheet/data.html?di=479539 - MCP2003B datasheet listing (TXD timer).
- https://infineon.com/cms/en/product/interface/automotive-transceiver/automotive-lin-transceivers/tle7259-3ge/ - TLE7259-3 (TxD time-out).
- https://www.vishay.com/applications/automotive/diesel_ecuandfuelinjectordrivers/canbus/iso9141_bustransceiver - Vishay Si9241A/Si9243A listing.
- https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf - ELM327 datasheet (reference circuit, sniffing warning, low-power figures).
- https://www.sparkfun.com/products/9555 and https://cdn.sparkfun.com/assets/a/3/f/9/c/OBD-II-UART-v13.pdf - SparkFun OBD-II UART (STN1110) page and schematic.
- https://www.obdsol.com/solutions/chips/stn2120/ - STN2120 discontinued.
- https://github.com/macchina/m2-hardware - Macchina M2 schematics and BOM; https://docs.macchina.cc/m2-docs/detailed-reference/interfaces.md - M2 interface documentation.
- https://github.com/maximus64/opendiag-hardware and https://www.tindie.com/products/opendiag/opendiag-open-source-obd2-interface/ - OpenDiag open hardware (ESP32-S3, TJA1027, pin drivers).
- https://github.com/iwanders/OBD9141 - K-line library, tested transceivers, logic captures.
- https://github.com/muki01/OBD2_K-line_Reader - six K-line interface schematics.
- https://github.com/commaai/panda/blob/master/python/__init__.py - current panda library (no K-line functions).
- https://www.hpacademy.com/technical-articles/tactrix-openport-2-0-alternatives/ - Openport 2.0 out of production in 2026.
- https://www.cnx-software.com/2024/08/22/wican-pro-esp32-s3-powered-obd-scanner-for-vehicle-diagnostics-with-smart-home-integration/ and https://www.crowdsupply.com/meatpi-electronics/wican-pro - WiCAN Pro hardware, current, price.
- https://freematics.com/products/freematics-one-plus-model-b/ - Freematics ONE+ Model B.
- https://circuitdigest.com/news/from-pico-2-to-pico-2-w-raspberry-pi-pico-2w-launched-at-7 , https://www.cnx-software.com/2024/11/25/7-raspberry-pi-pico-2-w-board-2-4-ghz-wifi-4-bluetooth-5-2-wireless-module/ , https://www.tomshardware.com/raspberry-pi/raspberry-pi-pico-2-w-review - Pico 2 W price and specifications.
- https://esp32.atomic14.com/compare/esp32-c6-mini-1-vs-esp32-s3-mini-1/ - ESP32-S3 versus ESP32-C6 peripheral comparison.
- https://www.analog.com/en/resources/technical-articles/low-quiescent-current-surge-stopper-robust-automotive-supply-protection.html - ISO 7637-2 / ISO 16750-2 supply transients overview.
- https://w3inte.intertek.ch/automotive/standards/iso-16750-2/ - Intertek overview of ISO 16750-2 tests (secondary source).
- https://saemobilus.sae.org/content/J2534-2/4_5_00/ - SAE J2534-2/4_0500 "Pass-Thru Extended Feature - HONDA_DIAGH".
- https://www.hondafitjazz.com/manual/A00/HTML/97/SAA2E97A20326919461FAAT00.HTML - Honda Jazz/Fit DLC circuit troubleshooting (terminal voltages, E23).
- https://www.justanswer.com/honda/1bezu-no-communication-data-link.html - transcription of the 2001 Civic DLC test step (8.5 V).
- https://www.hondacivicforum.com/forum/mechanical-problems-technical-chat-8/2004-16p-data-link-connector-troubleshooting-98549/ - owner measurements on a 2004 Civic DLC (search snippet; page itself returned 403).
- http://www.crvclub.ru/manuals/CR-V-2_manual_eng/manual_uk_shop/group_4v.htm - CR-V manual text naming the SCS line on DLC pin 9 (search snippet).
- https://e-tunez.com/2002-2006-prb-ecu-pin-outs-rsx - K-series ECM pin list.
- http://www.boostedk20.com/forums/showthread.php/1127-ECU-Pinout-and-Mil-Spec-Part-Numbers - ECM connector sizes (search snippet).
- https://www.insightcentral.net/threads/honda-him-hds-project.20427/page-2 - "DIAG line is a 5 V data bus" (search snippet; page not fetchable).
- https://honda-tech.com/forums/engine-management-tuning-124/honda-hds-cheap-mvci-cables-3312290/ - pin 7 to pin 14 bridge on J2534 cables (search snippet; 403 on fetch).
- https://www.odyclub.com/threads/hds-and-j2534-pass-thru.349083/ - "pin 14 activated" remark, Mongoose experience.
- https://autoprogs.ru/diagnostika-honda-remont-elektroniki-262/honda-hds-6430/index37.html - Russian forum: DIAG-H on pin 1 or 14, grounding 9/12/15.
- https://www.digital-kaos.co.uk/forums/showthread.php/157964-Honda-HIM-3pin-cable - HIM 3-pin adapter pin use (search snippet; 403 on fetch).
- https://www.k20a.org/threads/honda-hds-him-serial-cable-pinout.208962/ - HIM RS-232 cable pinout.
- https://honda-civic-hatchback-mk9.blogspot.com/2015/07/honda-diagnostic-system-clone.html - HIM clone opened (two boards).
- https://www.driveaccord.net/threads/accord-2003-obdii-or-can-bus.24332/ - forum description of DIAG-H and 92Hm/2.
- https://pinoutguide.com/CarElectronics/honda_obd_2_pinout.shtml - community Honda DLC pin table (unreliable for pins 12-14).
- https://www.tunerpro.net/forum3/viewtopic.php?p=23029 - KKL cable internals and Honda motorcycle wake-up pulse discussion.
- https://www.elecbee.com/en-52046-truck-diagnostic-j1962m-automobile-obd2-male-obd-16-pin-connector-gold-plated-90°-right-angle-male-shell , https://uk.rs-online.com/web/p/products/8010991 , https://littlebirdelectronics.com.au/products/obd-ii-connector.md - J1962 male connector options and prices.
