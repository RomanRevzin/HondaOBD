# EP3 2002 K20A3 (PNF) ECU internals and the free path to read and reflash it

Dimension owner notes (research round 2). Scope: the owner's own US-market 2002 Honda
Civic Si (EP3, K20A3, 5-speed) ECM - part-number family, microcontroller and memory,
whether the STOCK ECU can be read out and reprogrammed, the disassembler situation for the
OKI core, and the bricking/immobilizer risks. Builds on round-1 file
`05-ecm-reprogramming-and-firmware.md`; where this round corrects or sharpens it, that is
flagged. Budget: ~28 web calls + one local datasheet extraction.

Confidence tags: **HIGH** = primary source or two independent sources agree; **MEDIUM** =
one credible source; **LOW** = single forum post, inference, or conflicting reports.

## Summary

1. The owner's 2002 Civic Si ECM is **37820-PNF** (hardware assembly P/N). PNF = US 02-05
   Civic Si manual; the suffix (A01..A13) is a calibration/running-change revision. A 2002
   car carries an early suffix, all with the OKI 66Q592 MCU. [HIGH]
2. Family map (all 02-04, OKI 66Q592, K-Pro-able): **PNF** = Civic Si (K20A3), **PND** =
   base RSX manual (K20A3), **PRB** A01-A12 = RSX Type-S (K20A2), **PRC** = JDM Integra
   Type R, **PPA** = CR-V (K24), **PRD/PRA** = Civic Type R variants. The K20A3 and K20A2
   differ as engines but share the ECU hardware family; the ECUs are interchangeable once
   converted and loaded with the right calibration. [HIGH]
3. The MCU is the **OKI ML66Q592** - a Flash-EEPROM ("Q") 16-bit part on the OKI-original
   **nX-8/500S** core (OLMS-66K series). Flash ~**192 KB** (one source says 256 KB). The
   core family is 128-pin QFP, 64 KB data space, ~4 KB internal RAM, built-in multiplier,
   segmented/paged memory, with an **EA pin** that forces external-program-memory access.
   [HIGH for core/identity; MEDIUM for exact 592 flash/RAM]
4. **Honda itself CAN reflash this ECM in the field over the DLC/K-line** (HIM/HDS/J2534
   "Updating the ECM/PCM"), write-guarded. **No aftermarket or free tool reflashes the
   STOCK EP3 ECU over OBD.** Every aftermarket path bypasses the factory flash: Hondata
   K-Pro solders in a USB daughterboard; Hondata's mail-in "Reflash" ($295) is a one-time
   bench reprogram needing the ECU + key + immobilizer; KTuner uses a modified/replacement
   ECU. [HIGH]
5. Even the paid commercial tool PCMflash (module 13, Honda/Acura K-Line, Keihin OKI
   ML66Q592) offers only **"virtual reading"** and **"only writing"** of the OKI, shipping
   a stock-files database - i.e. it can write a full image through the bootloader but does
   **not** read the ECU back, and it lists Fit/Jazz/Airwave, not the EP3 Civic Si. [HIGH]
6. **No public, free stock ROM dump exists** for 37820-PNF/PND/PRB, and no RomRaider def /
   TunerPro XDF / kmanager map for the stock 02-05 K20A3 ROM. The open repos hold only
   motorcycle (38770) and OBD0/OBD1 Honda bins. This is a hard wall for the stretch goal.
   [HIGH negative]
7. There is still **no public Ghidra/IDA module for nX-8/500S**. asm662 covers the older
   nX-8/200 (OBD0/OBD1). The dedicated "nX-8/500S Core Instruction Manual" is referenced by
   OKI datasheets but is not online; the MSM66591/ML66592 hardware user manual exists but is
   bot-blocked. Building a disassembler is the single biggest REA prerequisite. [HIGH]
8. Free read-out of the stock flash is therefore a **bench** problem, not an OBD one: the
   EA-pin external-access/RAM-monitor trick (proven on older OKI parts) or a flash Vpp/
   serial programming mode, on a cheap junkyard PNF ECU. For *reading* you only power the
   board - the immobilizer is irrelevant on the bench. To *run* a stock US ECU you must
   satisfy the immobilizer (a programmed key + the unit, or a Hybrid-Racing immo box). [MEDIUM]

## Findings

### Q1. PNF part-number family, year variants, and the RSX relationship

| ECU P/N | Car (02-04 unless noted) | Engine | MCU | K-Pro? |
|---|---|---|---|---|
| **37820-PNF** | **US Civic Si (EP3), 5MT - the owner's car** | K20A3 | OKI 66Q592 | yes (A01-A12) |
| 37820-PND | US base RSX (DC5), 5MT | K20A3 | OKI 66Q592 | yes |
| 37820-PRB (A01-A12) | US RSX Type-S | K20A2 | OKI 66Q592 | yes |
| 37820-PRB (A13-A16) | **05-06 RSX** | K20Z1 | **different MCU** | **no** |
| 37820-PRC | JDM Integra Type R | K20A | OKI 66Q592 | yes (no wideband box needed) |
| 37820-PPA | CR-V | K24 | OKI 66Q592 | yes |
| 37820-PRD / PRA | Civic Type R (CTR / EP3 Euro) | K20A | OKI 66Q592 | PRD yes; PRA no wideband |

- **PNF = US 2002-2005 Civic Si (EP3), K20A3, 5-speed.** The bare 37820-PNF is the hardware
  assembly; suffixes A01..A13 are revisions. Marketplace/vendor listings consistently tie
  PNF-A05/A12 to the 02-05 Civic Si. [HIGH]
  ([Hybrid Racing ECU options](https://guides.hybrid-racing.com/Item/K-series_ECU_Options),
  [Hondata reflash - Civic Si 2002-2004](https://www.hondata.com/reflash-civicsi-2002),
  [partrequest PNF-A05 listing](https://www.partrequest.com/auto-parts-detail/2002-2005-honda-civic-si-5-speed-k20a3-ep3-ecu-ecm-326017750763))
- Hybrid Racing lists verbatim which ECUs "can be Kpro'd": *"PRB with code number A01-A12
  and 305 02-04 RSX manual," "PND 02-04 Base RSX manual," "PNF 02-04 Civic Si manual," "PRC
  02-04 JDM Integra Type R manual," "PPA 02-04 CRV manual," "PRD 02-04 CTR manual"*; and
  that *"PRB A13-A16 05-06 RSX"* will not. [HIGH]
  ([Hybrid Racing](https://guides.hybrid-racing.com/Item/K-series_ECU_Options))
- **K20A3 vs K20A2:** the Type-S K20A2 differs in pistons, cams, crank, valve springs and
  compression, and engages VTEC ~5800 rpm; the K20A3 (Si and base RSX) is the economy
  i-VTEC. But the ECUs share the hardware family/MCU and are interchangeable once K-Pro'd
  with the correct calibration. Hondata ships base maps for "K20A2, K20A3 (RSX Base), K20A3
  (Civic SI), K20A." [HIGH]
  ([Haltech K-series notes](https://support.haltech.com/portal/en/kb/articles/thomas-te),
  [Hybrid Racing](https://guides.hybrid-racing.com/Item/K-series_ECU_Options))
- **2005 running change (the "Si equivalent" of the D17 03/04 changes):** minor, not an MCU
  swap. Hondata tunes PNF **A01-A12** and built a wiring **adapter harness** so an 02-04
  K-Pro ECU runs in an 05 car (which also needs the 02-04 O2 sensor). So the 05 Civic Si
  differs mainly in harness/sensor, not silicon. [HIGH for A01-A12 tuning + adapter; MEDIUM
  that 05 is only a harness/sensor change]
  ([Hondata K-Pro page](https://hondata.com/kpro),
  [evasive K-Pro listing](https://www.evasivemotorsports.com/store/product/hondata-kpro/))
- **CORRECTION / caution:** one forum account claimed a "2005 PNF-A13" ECU was refused for a
  "different processor." Corroboration shows the **A13 new-MCU cutoff belongs to the RSX
  PRB-A13..A16 (05-06)**, not the Civic PNF; the PNF-A13-new-MCU claim is likely a
  conflation. The owner's **2002 PNF is unambiguously 66Q592**. [LOW for a PNF MCU change;
  HIGH that 2002 PNF = 66Q592]
  ([search corroboration - PRB-A13 is the "different microprocessor" part, not PNF](https://hondata.com/kpro))

### Q2. The microcontroller and memory

| Parameter | Value | Conf. |
|---|---|---|
| MCU marking (Honda part) | OKI **ML66Q592** / "66Q592" (flash "Q" version) | HIGH |
| CPU core | **OKI nX-8/500S**, OLMS-66K series, 16-bit | HIGH |
| Internal flash | **192 KB** (PCMflash x2); 256 KB in one RU listing [conflict] | MEDIUM |
| Internal RAM | ~4 KB (sister 66589; exact 592 unconfirmed) | MEDIUM |
| Data memory space | 64 KB | MEDIUM |
| Package | 128-pin plastic QFP (QFP128-P-2828 family) | MEDIUM |
| Min instruction time | 100 ns @ 20 MHz (sister part) | MEDIUM |
| Boot/program pins | **EA** pin = force external program access; Vpp/TEST exact pins not pinned | MEDIUM |

- MCU identity is solid from three independent angles: Hondata states a K-Pro donor "must
  have the **66Q592** processor"; PCMflash lists the Honda K-line ECU as "**Keihin OKI
  ML66Q592**"; the OKI user manual is titled "MSM66591/ML66592 ... nX-8/500S." [HIGH]
  ([Hondata forum](https://www.hondata.mobi/forum/viewtopic.php?p=92466),
  [PCMflash module list](https://ecutools.eu/chip-tuning/pcmflash/),
  [OKI MSM66591/ML66592 manual (title)](https://manualmachine.com/oki/ml66592/22525539-user-manual))
- Flash size **192 KB** appears twice in the PCMflash listing ("Keihin OKI ML66Q592/192KB");
  a Russian tool listing shows "L66Q592/256KB" - treat 192 KB as the better-supported
  figure, 256 KB as the conflict. [MEDIUM] ([PCMflash](https://ecutools.eu/chip-tuning/pcmflash/))
- **Core architecture** (from the freely downloadable sister-part datasheet MSM66589, same
  nX-8/500S core; extracted locally with pdftotext) [HIGH for the family]:
  - "128-pin plastic QFP (QFP128-P-2828-BK)."
  - "Internal RAM : 4K bytes," "Data memory space : 64K bytes."
  - "employs OKI original nX-8/500S CPU core," "Built-in multiplier," addressing modes:
    "Register / Page / Pointing register indirect / Stack / Immediate," "ROM table
    reference instructions," and an "operation code trap."
  - **EA pin:** "Normally set to 'H' level. If set to 'L' level, the program memory goes
    into external access mode and accesses external program memory." External bus is
    multiplexed: P0 = AD0-7, P1 = A8-15, P9 bit = A16, with PSEN/ALE/RD/WR/WAIT.
  - Flash ("Q") version holds its internal ROM at "8000H to 0FFFFH in Segment 1."
  ([OKI MSM66589 datasheet via ej9.ru](https://www.ej9.ru/data/chipms/msm66589.pdf))
  The ML66Q592 is the larger (192 KB) sibling in the same family, so these numbers are the
  right architectural model but the exact 592 RAM/package should be confirmed from the 592
  manual (bot-blocked this round).
- **External EEPROM / immobilizer store:** not confirmed on a 37820-PNF teardown. K-Pro can
  switch the **immobilizer OFF in software**, so the immobilizer state lives in rewritable
  memory (internal flash/EEPROM, or possibly a small external serial EEPROM like the 93C56
  that round 1 guessed). Unconfirmed either way. [LOW]
  ([Hondata EP3 FAQ](https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/))
- No public teardown photos of a 37820-PNF board were found this round. [negative]

### Q3. Reflash reality for the STOCK ECU - Honda vs aftermarket

Be precise about the two very different statements:

- **"Honda can reflash it in the field": TRUE.** Honda reprograms the EP3 ECM over the DLC/
  K-line with its own tools (HIM EQS05A35570, later HDS+MVCI, and J2534 for independents),
  write-guarded ("You cannot update an ECM/PCM with the program it already has... It will
  only accept a new program"; "Do not turn the ignition switch OFF while updating"). [HIGH]
  (round-1 primary bulletins: [Acura SB 01-026](https://static.nhtsa.gov/odi/tsbs/2013/SB-10039532-9101.pdf),
  [Civic Hybrid SB 11-043](https://static.nhtsa.gov/odi/tsbs/2012/SB-10042649-3282.pdf))
- **"An aftermarket tool can reflash the stock ECU over OBD": FALSE for the EP3.** Every
  aftermarket path bypasses the factory flash entry:

| Product | How it gets in | Cost / access | What it proves |
|---|---|---|---|
| Hondata K-Pro (v1-v4) | USB **daughterboard soldered** onto the stock ECU; "not reversible" | paid; US = competition-use | stock ECU is NOT OBD-reflashable by aftermarket |
| Hondata mail-in **Reflash** | **bench** reprogram by a Hondata dealer; one-time | **$295**; send ECU + **key + immobilizer**; "return to stock" option | even Hondata needs bench + key + immo, not in-car OBD |
| KTuner | dealer-modified / replacement ECU + jumper harness | paid | same - no stock-ECU OBD reflash |
| **PCMflash module 13** (commercial) | K-line bootloader **write only**; **"virtual reading"**; "stock files database included" | paid; lists **Fit/Jazz/Airwave**, not EP3 | the OKI K-line path writes but does **not read back**; EP3 support unconfirmed |

  ([Hondata reflash](https://www.hondata.com/reflash-civicsi-2002),
  [reflash form PDF](https://www.hondata.com/downloads/ecu_reflash_form.pdf),
  [Hondata EP3 FAQ](https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/),
  [PCMflash](https://ecutools.eu/chip-tuning/pcmflash/))
- Hondata's EP3 FAQ, verbatim: K-Pro is *"a hardware modification to the Acura RSX base and
  Type-S ECU (as well as the US Civic Si...)"*; the reflash is *"a one-time changing of the
  stock ECU programming"*; and on the immobilizer, *"No key and immobiliser is needed. As
  long as you install that ECU back into the car from which it came, the immobiliser will
  function."* [HIGH vendor]
  ([Hondata EP3 FAQ](https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/))
- **Implication:** the factory flash *entry* is reachable (Honda writes it; PCMflash writes
  the Fit/Jazz sibling through its bootloader), but it is gated by the dealer protocol and
  almost certainly a security/seed step, and the *read* is not exposed even in paid tools.
  For the owner's free device, the realistic OBD capability is **write/erase through the
  bootloader if (and only if) the protocol+seed are recovered**; a read-out is a bench job.
  [HIGH for the architecture; the EP3-specific command set is still unproven]

### Q4. Reading the firmware out - free / bench routes, public dumps, defs, checksum

**(a) Over the DLC (K-line):** no public ROM-read for the car OKI. The motorcycle Keihin
protocol has a real `pgmfi_read_flash_bytes`; the *later* Honda SH7058 K-line supports UDS
SID23 ReadMemoryByAddress but "appears to require **SID27 security access**," and a common
cable "could not dump RAM." For the **OKI EP3 there is no public read command at all**, and
PCMflash falls back to "virtual reading" (reconstruct from a database, not a real read).
[MEDIUM-HIGH]
([RomRaider via search - PZX SH7058 SID27 note](https://www.romraider.com/forum/viewtopic.php?t=19677),
[PCMflash](https://ecutools.eu/chip-tuning/pcmflash/))

**(b) Bench / silicon routes (FREE, the realistic path):**

| Route | Status | Conf. |
|---|---|---|
| **EA-pin external-access / RAM-monitor dump** (drive EA low, run a stub, stream internal ROM out a serial port) | pgmfi "McuReaders" prove it on older OKI 66207/66507; nX-8/500S EA pin confirmed present and functional; **unproven on 592** | MEDIUM |
| **Flash Vpp / serial or parallel programming mode** on the "Q" part | exists on OKI flash MCUs; exact 592 Vpp/TEST pins not pinned from free sources | MEDIUM |
| **Chip-off + external reader** | fallback, esp. for mask-ROM (non-Q) parts | MEDIUM |

([pgmfi McuReaders](https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/McuReaders.html),
[OKI MSM66589 datasheet - EA pin](https://www.ej9.ru/data/chipms/msm66589.pdf))

**(c) Public ROM dumps and definitions: none found for the stock 02-05 K20A3.** [HIGH negative]
- `roccofz1/HondaECU` holds only **motorcycle** bins (38770-*, CBR, 262144 bytes = 256 KB)
  and one CBR XDF - no car K20.
  ([repo tree](https://api.github.com/repos/roccofz1/HondaECU/git/trees/HEAD?recursive=1))
- pgmfi hosts only OBD0/OBD1 Honda basemaps.
  ([pgmfi/mycomputerninja](https://mycomputerninja.com/?p=344))
- RomRaider "Honda Stock ROM List" shows K20 only as a **2009 Civic K20Z3** (37805-RRB-A140)
  - a later CAN car, not the OKI EP3; one posted file was "just filled with FFs."
  ([RomRaider stock list](https://www.romraider.com/forum/viewtopic.php?f=68&t=19311))
- rusEFI "Vault of Honda OEM" = OBD1/OBD2A + 125-pin K-plug catalogue, no OKI dumps.
  ([rusEFI vault](https://wiki.rusefi.com/Vault-Of-Honda-OEM))
- The only K20 TunerPro XDFs (OldSkullTuning) cover **later** Keihin CAN ECUs (K20A, K20Z3/
  Z4/Z5), are **paid (60 EUR)**, and don't mention K20A3/OKI.
  ([OldSkullTuning XDF](https://oldskulltuning.com/tunerpro-maps-for-honda-2-0l-i-vtec-ecu-keihin/))
- The K-Pro ecosystem (kmanager) edits its own modified-ECU format, not a stock-ROM map, so
  it is not a source of a stock 37820-PNF definition.

**(d) Checksum algorithm:** unknown for the car OKI ROM. PCMflash performs "checksum
correction," so a checksum exists; the motorcycle ROM uses two 8-bit sums (a plain sum and a
two's-complement-of-sum). Not confirmed for the car part. [MEDIUM]

**(e) Junkyard 37820-PNF for bench work:** cheap and common used (eBay/partrequest list
PNF-A05, -A12, etc., typically tens of dollars). For *reading firmware* you only need to
power the board - no engine start - so the immobilizer is irrelevant on the bench. Bench
power pins for the K20 RSX/EP3 ECU: **A2/A3 = IGP battery power** (corroborated by a Hondata
K-swap thread reading ~2 V on plug A2/A3 ignition-on), plus grounds, main relay (MRLY), and
comms **K-line = E23**, **SCS = E29** (round-1 Fit/Jazz layout, consistent). Verify exact
pins against the Civic factory wiring diagram or the Haltech/Hybrid-Racing EP3 pinout images.
[HIGH used-ECU availability / MEDIUM exact pins]
([Hondata K-swap pinout thread](https://hondata.mobi/forum/viewtopic.php?p=137991),
[Hybrid Racing K-series ECU pinouts 02-06 (images)](https://hybrid-racing.com/blogs/hybrid-racing/k-series-ecu-pinouts-02-06))

### Q5. The disassembler gap for OKI nX-8/500S

- **No public Ghidra SLEIGH module and no IDA processor module for nX-8/500S.** Confirmed
  again this round - searches for a SLEIGH/opcode table return nothing. [HIGH]
- What exists in the community: **a1k0n/asm662** = assembler/disassembler for the older OKI
  **nX-8/200** (66201/66207/66301, OBD0/OBD1 Honda); a one-off **66507 DASM** for an OBD2A
  nX-8/500 (non-S) part. Neither covers the 591/592. [HIGH]
  ([asm662](https://github.com/a1k0n/asm662))
- **ISA documentation availability** (the thing that governs how hard a disassembler is):
  - The OKI datasheet points to a separate **"nX-8/500S Core Instruction Manual"** covering
    the instruction set and addressing modes - **not located online** this round (the main
    blocker). [gap]
  - The **MSM66591/ML66592 hardware User's Manual** (March 2002) exists on manualmachine /
    manuals.online but both return HTTP 403 to automated fetch - obtainable by a human. [MEDIUM]
  - **Sister-part datasheets are freely downloadable** (MSM66589/66573/66507 on ej9.ru,
    datasheetbank) and give the architecture: 128-pin QFP, segmented memory, 64 KB data
    space, register/page/pointing-register-indirect/stack/immediate addressing, built-in
    multiplier, ROM-table-reference instructions, opcode trap. [HIGH]
- **Scope to build the disassembler:** obtain the nX-8/500S instruction manual (or the 591/
  592 user manual) OR reconstruct the opcode map from asm662's nX-8/200 tables (the 500S is
  an evolution of the 200) and validate against a real 592 dump. A full SLEIGH module for a
  16-bit segmented OKI core with paged/pointing-register addressing is a substantial,
  multi-week effort and is the single biggest REA prerequisite for the reflash stretch goal.
  [HIGH]

### Q6. Risk and recovery

- **Bricking on interrupted flash:** the factory procedure warns the ECM "can be damaged" if
  the ignition is switched off mid-update; recovery is ignition-on + a jumper battery (not a
  charger) + retry (SB 01-026). The motorcycle protocol has an explicit **bootloader recover
  mode (0x7b)**, implying the OKI bootloader survives a bad *application* flash; whether the
  car OKI behaves the same is unconfirmed. PCMflash's "checksum correction" + stock-files
  database imply a recoverable write path on the sibling. [HIGH caution / MEDIUM bootloader
  survivability] ([Acura SB 01-026](https://static.nhtsa.gov/odi/tsbs/2013/SB-10039532-9101.pdf))
- **Immobilizer coupling:** the ECM stores the immobilizer pairing. A swapped or bench-
  swapped ECM will not start the engine until re-registered - Honda HDS "IMMOBI -> Setup"
  with a **programmed key present** and the car's **per-VIN immobilizer code** (from Honda's
  iN). This is a legitimate anti-theft requirement; document the architecture, not a defeat.
  [HIGH]
- **Can a bench ECU run without the car's immobilizer?**
  - For **reading firmware**: yes - you power the board only; no start; immobilizer is
    irrelevant. [HIGH]
  - To **run/start an engine** with a stock US ECU on a bench or in a swap: the immobilizer
    signal must be satisfied. Hybrid Racing sells an **"immobilizer removal box"** that lets
    a US-spec stock PRB/PND/PNF/PPA run on their harness (*"No other ECU's will work with the
    immobilizer box"*); the **JDM PRC needs no box - just ground one wire** to run the fuel
    pump. A K-Pro'd ECU can also disable the immobilizer in software. [HIGH]
    ([Hybrid Racing ECU options](https://guides.hybrid-racing.com/Item/K-series_ECU_Options),
    [Hondata EP3 FAQ](https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/))

## Conflicts and uncertainties

- **"2005 Civic Si PNF has a different (untunable) MCU":** one forum account vs. the
  corroborated fact that the A13+ new-MCU cutoff is the **RSX PRB-A13..A16**, not PNF.
  Treat the PNF-MCU-change claim as a likely conflation. The 2002 PNF = 66Q592 is certain.
  [resolved toward: no PNF MCU change; owner's car is 66Q592]
- **Flash size 192 KB vs 256 KB:** PCMflash says 192 KB (twice); a Russian listing says
  256 KB; the motorcycle 38770 bins are 256 KB (different family). Lean 192 KB; verify by a
  bench dump or the 592 manual. [open]
- **Exact 592 RAM / package / Vpp-TEST pins:** taken from the sister 66589 datasheet and the
  family; the 592 user manual (which would settle it) was bot-blocked. [open]
- **Whether the car OKI exposes any K-line read or accepts the moto 0x7d/0x7e flash set:**
  unknown. The 0x72/0x71 *diagnostic* layer looks shared bikes<->cars (round 1); the *flash*
  and *read* layers are unproven on the car, and PCMflash's "virtual reading" suggests there
  is **no** K-line read. [open - sniff a Honda write session to settle]
- **External immobilizer EEPROM on the PNF board:** unconfirmed; the immo state is in
  rewritable memory but its exact location (internal vs a serial EEPROM) is not pinned. [open]

## Open questions (and the cheapest FREE way to close each)

1. **Is there any K-line ROM-read on the OKI EP3, or is it write-only?** -> Sniff a Honda
   HDS/HIM/J2534 session (or a PCMflash write) against a junkyard PNF on the bench with the
   passive sniffer; the absence of a read confirms the bench-only dump path. (Free; uses the
   device's own sniffer.)
2. **A free stock 37820-PNF ROM dump** -> none public; produce one on the bench via the
   EA-pin external-access trick or a Vpp programmer on a scrap ECU, then share it to seed the
   community. (Cost = one junkyard ECU, already needed for bench work.)
3. **Exact ML66Q592 flash size, RAM, package, and Vpp/TEST programming pins** -> get the
   MSM66591/ML66592 User's Manual (manualmachine/manuals.online via a browser, since the
   fetcher is 403'd) and/or read a bench dump's size directly. (Free.)
4. **The nX-8/500S opcode map for a Ghidra module** -> obtain the "nX-8/500S Core Instruction
   Manual" (not online yet - try OKI/LAPIS archives, university libraries, bitsavers
   requests) or reconstruct from asm662's nX-8/200 tables validated against a real dump. (Free
   but labor-heavy; the key REA task.)
5. **The factory flash command set / seed-key / block layout / checksum for the OKI car** ->
   falls out of (1) sniffing + (4) disassembly; the SID27-style security seen on later Honda
   K-line is the pattern to watch for. (Free.)
6. **Exact EP3 ECU bench-power and DLC pinout** -> read the Civic factory wiring diagram
   (charm.li) or the Haltech/Hybrid-Racing EP3 pinout images; confirm A2/A3 = IGP, E23 =
   K-line, E29 = SCS on the actual PNF connector. (Free.)

## Sources

- https://guides.hybrid-racing.com/Item/K-series_ECU_Options - Hybrid Racing K-series ECU options: verbatim list of which ECUs (PNF/PND/PRB/PRC/PPA/PRD/PRA) can be K-Pro'd, immobilizer box. COMMUNITY/VENDOR, read via fetch.
- https://www.hondata.com/reflash-civicsi-2002 - Hondata "Reflash - Civic Si 2002-2004" product: $295 mail-in, send ECU+key+immobilizer, return-to-stock. VENDOR.
- https://www.hondata.com/downloads/ecu_reflash_form.pdf - Hondata reflash form: what to send, return-to-stock option. VENDOR (search-surfaced).
- https://civictype-r.co.uk/2008/07/17/hondata-faq-ep3-a-general-information/ - Hondata EP3 FAQ: K-Pro = daughterboard+USB hardware mod, reflash = one-time stock reprogram, immobilizer behavior. ARTICLE/VENDOR FAQ, read via fetch.
- https://hondata.com/kpro - Hondata K-Pro page: 02-04 donor list incl. PNF, 05 adapter harness, PRB-A13..A16 "different microprocessor." VENDOR.
- https://www.hondata.mobi/forum/viewtopic.php?p=92466 - Hondata staff: K-Pro donor "must have 66Q592 processor"; 05-06 PRB different MCU. FORUM (vendor staff).
- https://ecutools.eu/chip-tuning/pcmflash/ - PCMflash module 13 Honda/Acura K-Line: "Keihin OKI ML66Q592/192KB," "Only writing / Virtual reading, checksum correction, stock files database," Fit/Jazz/Airwave 2001-2007. VENDOR list, read via fetch.
- https://www.ej9.ru/data/chipms/msm66589.pdf - OKI MSM66589 datasheet (sister nX-8/500S part): 128-pin QFP, 4KB RAM, 64KB data space, EA-pin external-access, addressing modes, opcode trap. PRIMARY datasheet, extracted locally with pdftotext.
- https://manualmachine.com/oki/ml66592/22525539-user-manual - OKI MSM66591/ML66592 User's Manual (title confirms nX-8/500S core). PRIMARY, but HTTP 403 to the fetcher - open in a browser.
- https://api.github.com/repos/roccofz1/HondaECU/git/trees/HEAD?recursive=1 - roccofz1/HondaECU repo tree: only motorcycle 38770 bins (256KB) + one CBR XDF; no car K20. SOURCE (GitHub API).
- https://mycomputerninja.com/?p=344 - pgmfi stock dump repository note: OBD0/OBD1 only. COMMUNITY.
- https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/McuReaders.html - pgmfi OKI EA-pin ROM-read trick for 66207/66507. COMMUNITY REF (round-1 source, reused).
- https://www.romraider.com/forum/viewtopic.php?f=68&t=19311 - RomRaider "Honda Stock ROM List": K20 only as 2009 K20Z3 (later CAN). FORUM (403 to fetcher; content via search).
- https://www.romraider.com/forum/viewtopic.php?t=19677 - RomRaider "K-line open-source reflash utilities": SH7058 SID27 security, Piasini, K-line read still WIP. FORUM (403 to fetcher; content via search).
- https://wiki.rusefi.com/Vault-Of-Honda-OEM - rusEFI Honda OEM vault: OBD1/OBD2A + 125-pin K-plug catalogue, no OKI dumps. WIKI, read via fetch.
- https://oldskulltuning.com/tunerpro-maps-for-honda-2-0l-i-vtec-ecu-keihin/ - OldSkullTuning TunerPro XDF: later Keihin K20A/Z3/Z4/Z5 only, paid 60 EUR; no K20A3/OKI. VENDOR, read via fetch.
- https://github.com/a1k0n/asm662 - OKI nX-8/200 assembler/disassembler (OBD0/OBD1 Honda); does not cover nX-8/500S. SOURCE.
- https://support.haltech.com/portal/en/kb/articles/thomas-te - Haltech K-series notes: K20A2 vs K20A3 engine differences, K20 direct-fire coils. VENDOR.
- https://hondata.mobi/forum/viewtopic.php?p=137991 - Hondata K-swap thread: ~2 V on ECU plug A2/A3 ignition-on (IGP power pins), main relay. FORUM.
- https://hybrid-racing.com/blogs/hybrid-racing/k-series-ecu-pinouts-02-06 - Hybrid Racing 02-06 K-series ECU pinout images (A/B and C/D/E connectors). VENDOR (images only).
- https://static.nhtsa.gov/odi/tsbs/2013/SB-10039532-9101.pdf - Acura SB 01-026 "Updating Control Units/Modules": MVCI CM Update flow, failure recovery. PRIMARY (round-1 source, reused for Q3/Q6).
- https://static.nhtsa.gov/odi/tsbs/2012/SB-10042649-3282.pdf - Honda SB 11-043 Civic Hybrid PGM-FI reflash proof. PRIMARY (round-1 source, reused for Q3).
