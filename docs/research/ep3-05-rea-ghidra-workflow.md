# EP3 round 2, dimension 5: applying the REA toolkit + Ghidra to this car's RE targets, free, on this PC

Research date: 2026-11-14. Web research only; nothing was installed, cloned, downloaded or run. GitHub
content was read through `api.github.com`, `raw.githubusercontent.com` and `registry.npmjs.org`. Pages were
read through a summarising fetch tool, so exact strings (versions, byte values, command names) are reproduced
as returned and flagged for re-check at setup time.

This file builds on round 1's `09-rea-toolkit-fit.md` (REA fit) and `05-ecm-reprogramming-and-firmware.md`
(OKI nX-8/500S, flash). Where this round confirms, corrects or extends round 1, it says so. It does **not**
re-derive round 1's REA inventory; read `09` for the full tool/CLI catalogue.

Confidence tags: **[HIGH]** primary source or two independent sources agree; **[MEDIUM]** one credible source;
**[LOW]** inference, single weak source, or conflicting reports.

## Summary

1. REA's current release is still **6.9.0** (published 2026-11-12); no 6.9.x or 6.10 exists as of today. Node
   requirement unchanged: `^22.23.4 || ^24.22.1 || >=26.12.1`. The PC's Node v22.16.0 is rejected; upgrade to
   **v22.23.5** (latest 22.x LTS, released 2026-11-11) before any REA command. **[HIGH]**
2. Free setup is exactly as round 1 found: `npx rea-agents setup` (writes `~/.claude.json` `mcpServers.rea` +
   the `reverse-engineer-anything` skill into `~/.claude/skills`, keeps `.rea.backup`), previewed with
   `rea setup --client claude_code --dry-run --json`, reversed with `rea uninstall`; or a repo-clean single
   project install via `claude mcp add --scope local` (no skill, no repo write). **[HIGH/MEDIUM]**
3. **The biggest change this round is for the HDS target.** A GPL-3.0 project, `huhwhatbruh/HondaDiagnosticSuite`,
   has already taken HDS apart: it decompiled Honda's diagnostic **bytecode** (139 modules, 10,021 functions)
   and extracted **36,000 DTCs, 25,882 live-signal definitions and 3,607 unit converters for all 182 systems**,
   shipped as an encrypted database with a committed `release-key.txt`. This is almost the entire "pull HDS's
   DTC/parameter/command tables" job, done, for free. REA/Ghidra on HDS is now mostly a *verification and
   gap-fill* tool, not the primary extractor. **[HIGH]**
4. That project also confirms HDS's architecture: diagnostic logic is a **custom Honda bytecode run by an
   in-app interpreter** (not .NET, not Java), so REA's Ghidra P0 lane on HDS should target the native
   interpreter + the K-line framing, while the per-system logic is bytecode *data* needing custom parsing. **[MEDIUM]**
5. Cross-cutting gold: its `hondascan/src/diagh.rs` is a port of HDS's **"DIAG-H" = Honda's own K-line message
   family** and carries byte sequences lifted from HDS — wake-up `FE 04 91 6D`, stop `AD 04 F1 5E`, keep-awake
   `A0 05 70 01 EA`, commands `READOUT 0xA0 / CLEAR 0xA1 / INFOREAD 0xA2 / WAKE 0xFE`, frame
   `[cmd,len,data,checksum]` summing to 0 mod 256 — the same frame round 1 derived independently from the
   motorcycle libs. This partially closes the project's single biggest gap (no public Honda *car* byte
   sequences). It is UNVERIFIED on a car by its own authors. **[MEDIUM]**
6. ECU firmware is still the hard target. **No** nX-8/500S Ghidra/Rizin/IDA module exists anywhere (confirmed
   this round, GitHub search `total_count:0`), and REA admits no SuperH / arbitrary-raw image on Windows. The
   realistic free path is **plain Ghidra with a hand-written SLEIGH processor spec**, forked from the Unlicense
   `VIRUXE/ghidra-oki66207` module (a validated, table-driven SLEIGH for the *older* OKI nX-8/200 core). **[HIGH]**
7. The nX-8/500S instruction set is documented and free: pgmfi hosts `nx_800.500s_core.pdf` (core instruction
   set), `msm66591_ml66592_um.pdf` (full MCU manual) and `msm66q591_um.pdf` (flash protocol for the 66Q parts).
   This closes round 1's open "obtain the OKI manual" item and makes the SLEIGH effort bounded. **[HIGH]**
8. REA's `execute_rizin_command` is a dead end for the ECU: Rizin/radare2 ships no OKI nX-8 architecture, so
   `asm_arch` has no value that decodes 66Q592 code. Cheap check at setup: `rizin -L`. **[MEDIUM]**
9. Capture lane: REA ingests only saved pcap/pcapng (TShark) and sigrok recordings, Linux-only (WSL2), and
   sigrok has no K-line decoder. Two free conversion paths: (A) offline `text2pcap -l <DLT_USER 147-162>` +
   a Lua dissector mapped via Wireshark's DLT_USER table; (B) reuse the MIT **`jared52005/Monitor`** pipeline,
   which already wraps K-line in a dummy-IPv4 header on `DLT_RAW(101)` over TCP and has a `Passive_KLine`
   hardware folder (MC33660 K-line IC, the analogue of the device's L9637D). Monitor's IP-wrapped framing is
   more likely to survive REA's vanilla TShark than a raw DLT_USER pcap. **[HIGH/MEDIUM]**
10. Repo hygiene is already half-done: the public repo `github.com/RomanRevzin/HondaOBD` ships a `.gitignore`
    that excludes `/re/`, `/evidence/`, `*.gpr`, captures and dumps. Refinements below (meta-repo for versioned
    RE artifacts, keep Honda-derived DB + HDS binaries out for licensing, narrow the broad `*.bin`). **[HIGH]**
11. RE-environment checklist (all free): Node v22.23.5 -> WSL2 + Ubuntu (tshark, sigrok-cli, util-linux) ->
    JDK 21 -> Ghidra 12.1.4 -> `ghidra-oki66207` template + free OKI PDFs -> REA `--dry-run`. **[HIGH]**
12. Division of labour stands: REA natively on Windows for HDS/J2534 PE targets; REA in WSL2 for pcap/sigrok;
    **plain Ghidra (not REA)** for the ECU image; and `HondaDiagnosticSuite` read directly as the HDS data
    source. Do not install or run REA this round; this is planning only. **[HIGH]**

## Findings

### Q1. REA current version and the exact free setup

**Version and Node requirement (confirmed this round).**

- Latest npm release is **6.9.0** (`dist-tags.latest`), `engines.node` = `"^22.23.4 || ^24.22.1 || >=26.12.1"`,
  description "Reverse engineer anything from your terminal or agent with one CLI and MCP server."
  ([registry.npmjs.org/rea-agents/latest](https://registry.npmjs.org/rea-agents/latest)). The GitHub releases
  list shows `rea-agents-6.9.0` (2026-11-12) then `rea-agents-6.8.0` (2026-10-25) — **nothing newer than 6.9.0**
  ([releases API](https://api.github.com/repos/morluto/rea/releases?per_page=5)). **[HIGH]**
- Node target: the PC has v22.16.0 (too old). Latest 22.x LTS is **v22.23.5**, artifacts dated 11-Nov-2026
  ([nodejs.org/dist/latest-v22.x](https://nodejs.org/dist/latest-v22.x/)); any 22.x `>=22.23.4` satisfies REA.
  The 22.x line goes to maintenance/EOL ~April 2027, so this is a short-lived LTS — acceptable for a build
  phase, but expect to move to 24.x later. **[HIGH]**

**Free setup options (carried from round 1 `09`, still current for 6.9.0).**

| Method | Command | What it writes | Repo-clean? |
|---|---|---|---|
| Guided setup | `npx rea-agents setup` (or `@latest`) | `~/.claude.json` key `mcpServers.rea`; `reverse-engineer-anything` skill in `~/.claude/skills`; first copy saved as `.rea.backup` | No (writes to the user profile, not the repo) |
| Dry-run preview | `rea setup --client claude_code --dry-run --json` | nothing (status `planned`, exit 0) — read-only plan of exact paths/changes | n/a |
| Single project | `claude mcp add --scope local rea -- "<node.exe>" "<path>\scripts\rea.mjs" mcp` | local-scope MCP entry only, under the project's path in `~/.claude.json`; **no skill**, nothing in the repo tree | Yes |
| Uninstall | `rea uninstall` (`--purge-data` also removes `~/.rea`); local scope: `claude mcp remove --scope local rea` | removes REA's registrations/skill, preserves Evidence, Node, other MCP servers | n/a |

Source: round 1 `09-rea-toolkit-fit.md` section 2, from
[installation.md](https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/installation.md) and
[Claude Code MCP docs](https://code.claude.com/docs/en/mcp). **[MEDIUM]** (single documentation source) except the
`npx`/`claude mcp add` commands **[HIGH]** (repeated in README). Setup never installs Ghidra, Java or Node, and
must show its plan and ask approval before writing. Re-run the `--dry-run` once Node is upgraded to see the exact
file list on this machine before the owner approves. **[HIGH]**

**Recommendation.** For this project prefer the **guided `setup`** (the bundled skill's evidence discipline is
worth having), on a Node 22.23.5 runtime, with `REA_ANALYSIS_PROVIDER=ghidra`, `GHIDRA_INSTALL_DIR` and
`JAVA_HOME` set. If the owner wants zero footprint outside the project, `claude mcp add --scope local` works but
loses the skill. **[LOW]** (judgement)

### Q2. The K20A3 firmware target (OKI nX-8/500S): the realistic free RE workflow

**The gap, confirmed and sharpened.**

- The K20A3 (PNF) ECU MCU is the OKI **ML66Q592**, nX-8/500S core (round 1 `05`, [Hondata](https://www.hondata.mobi/forum/viewtopic.php?p=92466)). **[HIGH]**
- A targeted GitHub search for an nX-8/500S / 66591 / 66592 Ghidra or SLEIGH module returns **`total_count:0`**
  ([search/repositories](https://api.github.com/search/repositories?q=oki+nX-8+ghidra+OR+66592+OR+66591+sleigh)) —
  no module exists. **[HIGH]**
- REA cannot help load this image. Round 1 established: on Windows REA's raw-binary lane admits only `6502`;
  SuperH and all automotive MCU families are not admitted on any provider; `REA_GHIDRA_LANGUAGE_ID` cannot
  override the admitted raw import ([raw-binary.md](https://raw.githubusercontent.com/morluto/rea/main/docs/providers/raw-binary.md),
  [issue #4246](https://api.github.com/repos/morluto/rea/issues/4246)). **[HIGH]**
- `execute_rizin_command` exposes `asm_arch`/`asm_bits`/`asm_cpu`/`base_address`, but **Rizin/radare2 has no OKI
  nX-8 architecture plugin**, so there is no `asm_arch` value that decodes 66Q592 code. This path is a dead end;
  confirm with `rizin -L` / `rz-asm -L` at setup. **[MEDIUM]** (inference from Rizin's known fixed arch set;
  no nX-8 found in searches)

**The realistic free path: plain Ghidra + a hand-written SLEIGH spec, forked from an existing OKI template.**

Round 1 said "no nX-8 Ghidra module exists." That is correct for the **500S** core, but this round found a
working, validated Ghidra module for the *older sibling* core that is the right starting template:

| Resource | What it is | Core | License | Use |
|---|---|---|---|---|
| [`VIRUXE/ghidra-oki66207`](https://api.github.com/repos/VIRUXE/ghidra-oki66207) | Ghidra SLEIGH processor module, auto-generated from `src/66207.op` opcode table; `oki66207.slaspec` + `_p28.slaspec`, `.pspec` from `tools/gen_pspec.py`; **2606 constructors, full instruction coverage**; cross-checked against `dasm662` on the OEM P28 ROM, identical decode | OKI **nX-8/200** (66201/66207/66301) | Unlicense (public domain) | **Template**: fork the generator + SLEIGH skeleton; author a new nX-8/500S `.op` table + regs header ([README](https://raw.githubusercontent.com/VIRUXE/ghidra-oki66207/main/README.md)) |
| [`a1k0n/asm662`](https://github.com/a1k0n/asm662) / [`VIRUXE/asm662`](https://github.com/VIRUXE/asm662) | assembler/disassembler + opcode tables, BSD (orig. Andy Sloane) | nX-8/200 | BSD | opcode-table reference; the 66207 module is generated from these |
| [`a1k0n`-style 66507 DASM] (round 1 `05`) | one older OBD2A part | nX-8/500 (early) | — | partial 500-family reference |

The nX-8/500S is an **enhanced** core versus the nX-8/200, so the opcode table must be re-derived (the 200-core
tables will not decode 500S code directly). But the hard scaffolding — the SLEIGH skeleton, the table-driven
generator, the `dasm662`-cross-check validation method — is reusable and public-domain. Effort is **substantial
but bounded**: expect on the order of the 66207's 2606 constructors or more, written from the ISA manual, then
validated against a known dump. **[MEDIUM]**

**The ISA references are free (closes round 1 open question #4).** pgmfi's `66kAssemblerDocs` wiki page lists:

| File | Description (uploader's words) | Relevance |
|---|---|---|
| `nx_800.500s_core.pdf` | "nx8/500 ... Instruction set / Core docs" | **the nX-8/500S core instruction set** — the authoritative source for the `.op` table |
| `msm66591_ml66592_um.pdf` | "MCU used in RSX ecus and probably 2001+ Civic" | full MSM66591/ML66592 User's Manual: memory map, peripherals, registers (for the `.pspec`) |
| `msm66q591_um.pdf` | "Manual describing flash protocol for 66Qxxx !!!" | **flash read/write protocol for the 66Q parts** — directly relevant to the reflash/dump stretch goal (cross-cutting to dimension `05`) |
| `Oki_66201_Instruction_Manual.pdf`, `66207*` | nX-8/200 manuals | matches the ghidra-oki66207 template basis |

Source: [pgmfi 66kAssemblerDocs](https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/66kAssemblerDocs.html);
a 466-page copy of the User's Manual (FEUL66591-66592-01, dated 2002-03-04) is also mirrored on ManualsLib. **[HIGH]**

**Workflow (plain Ghidra, not REA):**
1. Obtain a ROM image (separate problem — bench read via the OKI EA-pin trick or the 66Q flash/Vpp mode; see `05`).
2. Fork `ghidra-oki66207`; write `nx8_500s.op` from `nx_800.500s_core.pdf` and the regs header from `msm66591_ml66592_um.pdf`.
3. Generate `.slaspec`/`.pspec`, compile with `sleigh`, load the raw image in Ghidra with an explicit base/vector layout (the 66592 has separate ROM/RAM/SFR regions; set them as memory blocks manually — REA's one-segment raw lane would be wrong here even if it admitted the CPU).
4. Validate: disassemble a region whose bytes you can hand-decode from the manual; iterate.
5. Only *then* is REA marginally useful (entropy/HEX-record inspectors via `rea inspect-artifact`); the decompilation itself stays in plain Ghidra. **[MEDIUM]** (my synthesis)

### Q3. The HDS software and J2534 DLL targets

**This question is largely pre-solved by a free GPL project — read it before reaching for Ghidra.**

[`huhwhatbruh/HondaDiagnosticSuite`](https://api.github.com/repos/huhwhatbruh/HondaDiagnosticSuite) (GPL-3.0-or-later,
Rust, created 2026-10-11, actively pushed 2026-11-13) states in its
[README](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/main/README.md):

- "I took HDS apart piece by piece, without installing Windows." "Honda's diagnostic logic is bytecode. I wrote
  a decompiler for it: 139 modules, 10,021 functions, no opcode left undecoded." Functions were "run next to
  Honda's own interpreter" (9,660 compared, 0 mismatches). **[HIGH]**
- Extracted data: **182 systems**, **~36,000 trouble codes** with Honda's descriptions and repair text,
  **25,882 signal definitions** each with Honda's formula, **3,607 unit converters** (67,553 runs, 0 mismatches).
  Shipped as a database "derived from Honda's diagnostic data," encrypted, opened by `hondascan/release-key.txt`;
  file format not named. **[HIGH]**
- "**113 of the 182 systems use the older K-line or Honda's own older protocol**," and "an ELM327 cannot speak"
  that older protocol — i.e. exactly the EP3/K20A3's class. **[HIGH]**

**Architectural consequence for REA/Ghidra.** HDS is a native Windows app (round-1 dimension `08`; Honda's own
pages require Windows 10+ for i-HDS), but its per-system diagnostic logic is **Honda's proprietary bytecode run
by an in-app interpreter** — not .NET CIL, not Java. So:

- REA's **managed (.NET) lane and any Java lane are mostly irrelevant** to HDS core logic; the "command dispatch"
  is a bytecode VM, not an x86 `switch`.
- REA's useful HDS lanes are the **built-in PE inspectors** (`inspect-native-exports`, `inspect-pe-imports`,
  `inspect-pe-resources`) to map which EXE/DLL carries the serial/USB transport, and **Ghidra P0** (32-bit x86
  PE, read-only, admitted since 6.0.0) to analyse the **bytecode interpreter** and the native K-line framing.
  Then `search_strings`/`search_bytes` for known request bytes (`FE 04 ...`), `xrefs`, `procedure_pseudo_code`.
- The **bytecode modules and the database are data files** — not a Ghidra job. Custom parsing is needed, and
  `HondaDiagnosticSuite` has already done it. The decompiler tool itself is **not** published; what is published
  is the derived data + a Rust client. **[MEDIUM]** (architecture inferred from the project's own account + HDS-is-native)

**J2534 DLL workflow (REA's strongest native-Windows lane).**

- `rea inspect-native-exports` lists the J2534 API a vendor DLL exports (`PassThruOpen`, `PassThruConnect`,
  `PassThruWriteMsgs`, `PassThruReadMsgs`, `PassThruIoctl`, ...); `rea inspect-pe-imports` shows the USB/serial
  transport libraries behind it; **Ghidra P0 decompiles the exported functions without loading the DLL**
  (x86 and x86-64 PE admitted). Round 1 `09` section 4f, [binary-diagnostics.md](https://raw.githubusercontent.com/morluto/rea/main/docs/guides/native/binary-diagnostics.md),
  [windows-ghidra-p0.md](https://raw.githubusercontent.com/morluto/rea/main/docs/providers/windows-ghidra-p0.md). **[HIGH]**
- Worked precedent: `HondaDiagnosticSuite` reverse-engineered the **GODIAG GD101** (a cheap J2534 interface) by
  tracing "the GD101's Windows driver" and rewriting it "as a Linux J2534 library" (`hondascan/drivers/gd101_j2534.c`,
  `tools/gd101_pt_probe.c`, tested against a fake with the device's 10-frame transmit queue), and "the bus was
  read from the interface's firmware with a disassembler." This is exactly the "pull protocol out of a clone
  cable's J2534 DLL" task, with an affordable target already demonstrated. **[HIGH]**

**REA's gaps and the free tools that fill them (use alongside REA):**

| REA gap | Free filler |
|---|---|
| No JAR/class/bytecode-Java lane | JD-GUI, CFR, Procyon, or Fernflower (unlikely needed — HDS is native) |
| No CAB/MSI/InstallShield/NSIS unpack (only ZIP/APK/IPA/MSIX/ASAR/DMG) | 7-Zip, Universal Extractor 2, `lessmsi` (MSI), `innoextract` (Inno), InstallShield tools |
| .NET lane returns CIL, not C# | ILSpy / `ilspycmd` / dnSpyEx (then `import_managed_reconstruction` to bring C# back as non-canonical inference) |
| Proprietary DB formats not parsed (`rea inspect-sqlite-database` is SQLite-only) | read `HondaDiagnosticSuite`'s extracted DB instead; or custom parser |

Sources: round 1 `09` section 4a, [managed-code-analysis.md](https://raw.githubusercontent.com/morluto/rea/main/docs/guides/managed/managed-code-analysis.md),
[desktop-applications.md](https://raw.githubusercontent.com/morluto/rea/main/docs/desktop-applications.md). **[MEDIUM]**

**Recommendation.** Treat `HondaDiagnosticSuite` as the HDS data source (DTCs, signals, converters, and the
decompiled logic for the 113 older-protocol systems). Use REA only to (a) verify specific frames against the
real HDS binary, and (b) RE a clone J2534 DLL's transport. Keep the licensing note from Q5 in mind. **[LOW]** (judgement)

### Q4. Capture analysis lane: K-line logs -> pcap -> REA/TShark/Wireshark

**Constraints (round 1 `09` section 4d, confirmed).** REA accepts only *saved* `pcap`/`pcapng` via TShark 4.x
and logic-analyser recordings via `sigrok-cli 0.7.2`; both lanes are **Linux-only** (macOS/Windows refused), so
they run in **WSL2**. REA does no live capture and guesses no baud rate. **sigrok has no ISO 9141 / K-line /
KWP2000 decoder** (its decoder set has `uart`, `can`, `lin`, J1850, but nothing for K-line/OBD), so the signal
lane yields a byte stream at a chosen baud rate and Honda framing needs our own parser.
([packet-captures.md](https://raw.githubusercontent.com/morluto/rea/main/docs/guides/data/packet-captures.md),
[signal-recordings.md](https://raw.githubusercontent.com/morluto/rea/main/docs/guides/signals/signal-recordings.md),
[sigrok decoders](https://sigrok.org/wiki/Protocol_decoders)). **[HIGH]**

**Two free ways to make a K-line capture that Wireshark/TShark can dissect.**

*Path A — offline `text2pcap` + a USER link type + a Lua dissector.* The device logs each K-line byte with a
timestamp (hex dump). Then:
- `text2pcap -t "%H:%M:%S." -l <N> klog.txt klog.pcap`, where `<N>` is a USER DLT. **USER0–USER15 = DLT 147–162**
  (`text2pcap -l 147` ... `162`).
- In full Wireshark, map that DLT_USER to a Lua dissector via **Preferences -> Protocols -> DLT_USER -> Encapsulations
  Table** (the canonical recipe is the Wireshark wiki ["How to Dissect Anything"](https://wiki.wireshark.org/HowToDissectAnything)).
- Guy Harris (Wireshark core) notes there is **no generic serial/UART link type**, so a USER encapsulation is the
  correct choice for a custom framed protocol like K-line
  ([ask.wireshark.org serial/UART DLT](https://ask.wireshark.org/question/3584/link-layer-header-type-for-serialuart-communication/)).
- The Lua dissector must handle K-line's **half-duplex echo, the length byte, and the sum-to-0 checksum**
  (round 1 frame format; the DIAG-H frames in Q3/cross-cutting are a ready field map). **[HIGH]** for the mechanism

*Path B — reuse the MIT `jared52005/Monitor` pipeline.* [`jared52005/Monitor`](https://github.com/jared52005/Monitor)
is a Wireshark feeder that already solves this for K-line:
- It runs a **TCP server** (TCP:19000 for K-line / IP-RAW protocols) and streams **classic pcap** (magic
  `A1B2C3D4`, snaplen 65535).
- For ISO 9141 / ISO 14230 (and KW1281, VW TP2.0, ISO15765) it wraps the payload in a **dummy IPv4 header** on
  `DLT_RAW` (**link type 101**) with IP protocol byte `0x90 + n` (**ISO14230 = 1 -> 0x91**), then dissects with
  **Lua dissectors**; it has a `Plugins` folder and a **`Hardware/Passive_KLine`** folder, with software modules
  for a J2534 device and for **FT232 + MC33660** (an off-the-shelf K-line transceiver — the analogue of the
  device's L9637D). License MIT. **[HIGH]**
- Why this matters for REA: REA's TShark field lane was round-1-verified only on Ethernet/IPv4/UDP/DNS and
  Bluetooth-HCI fixtures, and REA runs TShark with an **empty private config directory**, so a DLT_USER->Lua
  mapping and custom Lua plugins **probably will not load under REA**. Monitor's **dummy-IPv4-on-DLT_RAW**
  framing rides on TShark's *built-in* IP dissection, so REA's vanilla TShark can at least peel the IP layer and
  expose the K-line payload bytes as data, even without the custom dissector. **Monitor's encapsulation is the
  more REA-friendly choice.** **[MEDIUM]** (inference from round 1's TShark-config note + Monitor's design)

**Practical split.** Use **full Wireshark** (with Monitor's Lua dissectors, or a DLT_USER Lua dissector) for
human protocol analysis; feed **REA** the *saved* `.pcap` in WSL2 for mechanical field extraction/filtering and
for co-analysis with the HDS Evidence bundles. Plain timestamped UART logs gain little from REA directly —
convert first or parse with a small script. **[MEDIUM]**

**Test point on the device for clean captures.** Tap the **logic-side RX** of the K-line transceiver (the L9637D
RX pin, a clean 0/3.3 V digital copy of the bus), not the 12 V bus wire, into (a) a spare Pico UART for the
device's own byte logging and (b) a pin-header for an external logic analyser (sigrok/PulseView). Bring a common
**ground** to the same header. For the second physical layer (DIAG-H, ~5 V on DLC pin 14) add a second logic-side
RX tap. A clean digital tap avoids the analog edge/threshold problems of probing pin 7 directly and makes both
the device log and any sigrok capture unambiguous. This matches the `Passive_KLine` + MC33660 pattern in Monitor.
**[MEDIUM]** (engineering synthesis, grounded in the device's L9637D design and Monitor's precedent)

### Q5. Repo hygiene: keep RE artifacts out of the public HondaOBD repo

`D:\hondaobd` **is** a git repo with remote `https://github.com/RomanRevzin/HondaOBD.git` (public), top-level
`docs/ firmware/ hardware/ host/ README.md .gitignore`. (The session banner's "is a git repository: false" was
a detection miss on Windows.) It already ships a thoughtful RE-exclusion `.gitignore` block:

```gitignore
/re/
/evidence/
*.gpr          # Ghidra project file
*.rep          # Ghidra project dir
*.rwd *.bin *.rom
captures/ *.pcap *.pcapng *.sr *.vcd
/build/ /firmware/build/ *.o *.elf *.uf2 *.hex
bin/ obj/ *.user   # .NET host
```

This correctly implements the standing rule (RE evidence bundles, Ghidra projects, dealer software, dumps and
captures never enter the user's source history). **[HIGH]** (observed)

**Refinements:**
- **Honda-derived data and HDS binaries, explicitly out.** If the owner mirrors `HondaDiagnosticSuite`'s
  `hondascan.db` + `release-key.txt`, or any extracted HDS bytecode/strings, keep them in `/re/` (already
  ignored) and **never** commit them to the public repo — its GPL covers the project's code, not Honda's data
  rights. Add belt-and-braces patterns: `*.db` is too broad (could hit a legit app DB), so prefer a dedicated
  ignored dir, e.g. `/vendor-hds/` and `/re/hds/`. **[MEDIUM]**
- **`*.bin` is broad** — it will also hide any legitimate binary asset the owner may want tracked. Keep it, but
  be aware; scope firmware dumps under `/re/dumps/` instead if a tracked `*.bin` is ever wanted.
- **Version RE artifacts separately with a meta-repo** (CLAUDE.md dotfiles pattern) when they are worth keeping
  (a working SLEIGH spec, annotated Ghidra exports, labelled captures): a second bare git-dir over the same
  work-tree, `status.showUntrackedFiles no`, deny-by-default `info/exclude`, `git add -f` the meta paths. The
  public repo stays blind; the artifacts get history. Check `ls ~/.claude-repos/` first. **[HIGH]** (per CLAUDE.md)
- **Prefer `.git/info/exclude`** for any Claude/tool-specific ignores you do not want collaborators to see;
  the tracked `.gitignore` is the right place for the *policy-signalling* RE block the owner already wrote. **[MEDIUM]**
- **REA residual state lives in `$HOME`, not the repo** (`~/.rea`, provider caches, `~/.claude.json`, skills), so
  it is already outside the work-tree; nothing to ignore there. Evidence bundles land only where you name them
  (`--evidence_output`, `export_evidence_bundle`) — point those at `/re/evidence/`. **[MEDIUM]** (round 1 `09` section 5)

**Suggested storage layout:** keep all RE material in a sibling tree *outside* the repo, e.g.
`D:\hondaobd-re\{hds\, j2534\, firmware\, ghidra-projects\, evidence\, captures\}`, with the meta-repo over
`D:\hondaobd` tracking only the small reusable text artifacts (SLEIGH sources, dissector `.lua`, notes). Large
regenerable blobs (ROM dumps, Ghidra `.rep` databases, raw captures) stay untracked in either repo. **[MEDIUM]**

### Q6. Ordered RE-environment setup checklist for this PC (all free)

Stand this up only when the project reaches the RE phase and the owner consents to installs.

| # | Step | Command / source | Why |
|---|---|---|---|
| 1 | Upgrade Node 22.16.0 -> **v22.23.5** (latest 22.x LTS, `>=22.23.4`) | nvm-windows, or the `.msi` from [nodejs.org/dist/latest-v22.x](https://nodejs.org/dist/latest-v22.x/) | REA 6.9.0 rejects anything below 22.23.4 |
| 2 | Install **WSL2 + Ubuntu** | `wsl --install` (PowerShell, admin), reboot | REA's TShark field, sigrok and firmware lanes are Linux-only |
| 3 | In Ubuntu: `tshark`, `sigrok-cli` + libsigrokdecode, util-linux (`prlimit`) | `sudo apt install tshark sigrok-cli util-linux` | the pcap/signal lanes REA drives |
| 4 | Install **JDK 21** (64-bit full JDK, e.g. Temurin) | adoptium.net | Ghidra 12.1 requires JDK 21 minimum (official release notes) |
| 5 | Install **Ghidra 12.1.4** | github.com/NationalSecurityAgency/ghidra releases | REA needs `>=12.1` (verified 12.1.4); also the plain-Ghidra ECU lane. Set `GHIDRA_INSTALL_DIR`, `JAVA_HOME` |
| 6 | (Firmware) clone the Unlicense `ghidra-oki66207` template; fetch the free OKI PDFs | pgmfi 66kAssemblerDocs | basis for the nX-8/500S SLEIGH spec |
| 7 | **REA dry-run**, then setup | `rea setup --client claude_code --dry-run --json`, then `npx rea-agents setup` (or `claude mcp add --scope local`) | preview exact file writes; set `REA_ANALYSIS_PROVIDER=ghidra` |

Prerequisite notes (round 1 `09` section 2): target/temp dirs on a fixed local NTFS/ReFS volume with **no
junctions/UNC/SUBST** in the path; `TMP`/`TEMP` writable with no dot-prefixed element; raise
`REA_GHIDRA_STARTUP_TIMEOUT_MS` (cold Windows imports took 407–601 s on small fixtures). Verify drive `D:` meets
the volume rule before placing targets. Ghidra JDK 25 claims seen online are **unverified** and conflict with the
official JDK-21 minimum — treat them as wrong for 12.1.x. **[HIGH]** except the JDK-25 caveat **[LOW]**.

## Conflicts and uncertainties

1. **"DIAG-H" has two meanings in this project's sources.** Round 1 (`02`/`04`) used "DIAG-H" for the ~5 V
   second serial line on **DLC pin 14** (SRS/immobilizer/EPS). `HondaDiagnosticSuite`'s `diagh.rs` uses "DIAG-H"
   for **Honda's own K-line *message family*** ("the 55 systems HDS lists as the HONDA protocol family"),
   explicitly a "one-wire K-line port" with unknown baud and physical line. These may be the same thing (HDS's
   name for the Honda-protocol family that happens to run on the pin-14 line) or not. The project itself says
   the physical line, baud and init are unknown and it has never driven it. Hand to the protocol dimension to
   reconcile; do not assume pin 14 vs pin 7 from this project alone. ([diagh.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/main/hondascan/src/diagh.rs))
2. **`HondaDiagnosticSuite` scope vs validation.** Its *data* (DTCs, signals, converters, decompiled logic) is
   Honda's whole 182-system catalogue, so it applies to the EP3. But its *road validation* is a 2015 Accord
   (CAN, via GODIAG GD101) plus partial 2019/2021 Civic over ELM327; the **K-line / older-protocol path is
   "simulator only" with only a session probe in source**, and reflash/immobilizer are **not built**. So the
   EP3's actual transport is exactly what is unproven there — the owner's device fills that gap. Do not read
   "it supports K-line" as "it has driven a K-line car." (README; `diagh.rs` STATUS comment)
3. **Feature-list vs status-table conflict in that repo.** The feature list says "SocketCAN, slcan, J2534,
   ELM327 ... are supported," while the status table marks SocketCAN/slcan/ELM327/K-line "Simulator only." The
   status table is the more cautious, likely-current statement.
4. **REA TShark + custom dissectors.** Whether REA's TShark (empty private config dir) loads a Lua dissector or
   a DLT_USER mapping is **unconfirmed** (round 1 inference, LOW). The Monitor dummy-IPv4 approach sidesteps it
   but is itself untested under REA. Resolve with a 10-minute WSL2 bench test after install.
5. **nX-8/500S vs nX-8/200 opcode overlap.** How much of the 66207 `.op` table transfers to the 500S is unknown
   until the `nx_800.500s_core.pdf` ISA is compared opcode-by-opcode; the 500S is enhanced, so assume a new table.
6. **Docs read from `main`** can be a few days ahead of npm 6.9.0; tool names/limits may differ slightly in the
   installed package (carried from round 1).

## Open questions (and the cheapest free way to close each)

| # | Question | Cheapest free close |
|---|---|---|
| 1 | Does REA's WSL2 TShark dissect a Monitor-style dummy-IPv4 K-line pcap, and will a Lua dissector or DLT_USER mapping load under REA? | 10-min WSL2 bench test after install: build one sample pcap, run `rea inspect-packet-fields` |
| 2 | How much of `ghidra-oki66207`'s 66207 opcode table maps to the nX-8/500S? | Diff a page of `nx_800.500s_core.pdf` against `src/66207.op` before committing to the port |
| 3 | What file format is `HondaDiagnosticSuite`'s `hondascan.db`, and does it hold the 113 older-protocol systems' frames usable for the EP3? | Read `hondascan/src/db.rs`, `dbcrypt.rs`, `build-db.rs`; decrypt locally with the committed `release-key.txt` |
| 4 | Are `diagh.rs`'s HDS-derived frames (`FE 04 91 6D` etc.) correct for a real 2001-05 Honda, and on which pin/baud? | Sniff a real HDS/clone session on the bench with the device's passive tap; compare (protocol dimension) |
| 5 | Exactly which files will REA `setup` change on this PC? | `rea setup --client claude_code --dry-run --json` once Node is 22.23.5 |
| 6 | Is there any public stock K20A3/PNF ROM dump to validate the SLEIGH spec against? | Non-English search (drive2.ru, Chinese tuning sites); otherwise bench-read one (dimension `05`) |
| 7 | Does `HondaDiagnosticSuite`'s GD101 Linux J2534 driver (`gd101_j2534.c`) reveal the clone-cable transport framing reusable by the device? | Read `drivers/gd101_j2534.c` + `probes/gd101_pt_dump.rs` directly |

## Sources

- [registry.npmjs.org/rea-agents/latest](https://registry.npmjs.org/rea-agents/latest) — REA 6.9.0 version, `engines.node`, description (confirmed latest this round).
- [api.github.com/repos/morluto/rea/releases](https://api.github.com/repos/morluto/rea/releases?per_page=5) — release tags; newest is `rea-agents-6.9.0` (2026-11-12), nothing newer.
- [nodejs.org/dist/latest-v22.x](https://nodejs.org/dist/latest-v22.x/) — current Node 22 LTS = v22.23.5 (2026-11-11); upgrade target.
- [morluto/rea installation.md](https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/installation.md) — setup, dry-run, uninstall behaviour (via round 1 `09`).
- [code.claude.com/docs/en/mcp](https://code.claude.com/docs/en/mcp) — Claude Code MCP scopes; `--scope local` keeps the repo clean.
- [api.github.com/repos/huhwhatbruh/HondaDiagnosticSuite](https://api.github.com/repos/huhwhatbruh/HondaDiagnosticSuite) — repo metadata: GPL-3.0, Rust, created 2026-10-11.
- [HondaDiagnosticSuite README](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/main/README.md) — HDS bytecode decompilation; 36k DTCs / 25,882 signals / 3,607 converters / 182 systems; 113 older-protocol systems; GODIAG GD101 driver RE; encrypted DB + release key; K-line simulator-only; reflash/immobilizer not built.
- [HondaDiagnosticSuite diagh.rs](https://raw.githubusercontent.com/huhwhatbruh/HondaDiagnosticSuite/main/hondascan/src/diagh.rs) — "DIAG-H = Honda's own K-line message family"; HDS-derived frames `FE 04 91 6D` / `AD 04 F1 5E` / `A0 05 70 01 EA`; cmds `READOUT 0xA0 / CLEAR 0xA1 / INFOREAD 0xA2 / WAKE 0xFE`; `[cmd,len,data,checksum]` sum-to-0; never run on a car.
- [api.github.com/repos/VIRUXE/ghidra-oki66207](https://api.github.com/repos/VIRUXE/ghidra-oki66207) — Ghidra SLEIGH module for OKI nX-8/200 (66201/66207/66301), Unlicense.
- [ghidra-oki66207 README](https://raw.githubusercontent.com/VIRUXE/ghidra-oki66207/main/README.md) — table-driven `.op` generation, 2606 constructors, validated vs dasm662; the template for an nX-8/500S port.
- [VIRUXE/asm662](https://github.com/VIRUXE/asm662) / [a1k0n/asm662](https://github.com/a1k0n/asm662) — OKI 66k (nX-8/200) assembler/disassembler + opcode tables; BSD.
- [api.github.com GitHub repo search (nX-8 ghidra / 66592)](https://api.github.com/search/repositories?q=oki+nX-8+ghidra+OR+66592+OR+66591+sleigh) — `total_count:0`: no nX-8/500S Ghidra/SLEIGH module exists.
- [pgmfi 66kAssemblerDocs](https://mycomputerninja.com/~jon/www.pgmfi.org/twiki/bin/view/Library/66kAssemblerDocs.html) — free OKI docs: `nx_800.500s_core.pdf` (500S ISA), `msm66591_ml66592_um.pdf` (MCU manual), `msm66q591_um.pdf` (66Q flash protocol).
- [jared52005/Monitor](https://github.com/jared52005/Monitor) — MIT K-line/CAN Wireshark feeder; dummy-IPv4 on DLT_RAW(101), IP proto 0x90+ISO14230, TCP:19000, classic pcap, Lua dissectors, `Passive_KLine` + FT232/MC33660 hardware.
- [Wireshark "How to Dissect Anything"](https://wiki.wireshark.org/HowToDissectAnything) — text2pcap + USER DLT (147–162) + DLT_USER encapsulation table to a custom dissector.
- [ask.wireshark.org serial/UART DLT](https://ask.wireshark.org/question/3584/link-layer-header-type-for-serialuart-communication/) — no generic serial link type; use a USER encapsulation.
- [sigrok Protocol_decoders](https://sigrok.org/wiki/Protocol_decoders) — decoder list has uart/can/lin/J1850 but no K-line/KWP2000/ISO 9141.
- [Ghidra 12.1 release notes (JDK 21 minimum)](https://ghidradocs.com/12.1_PUBLIC/docs/WhatsNew.html) — Ghidra 12.1 requires at minimum JDK 21.
- Round 1 in-repo: `docs/research/09-rea-toolkit-fit.md` (REA inventory, Windows P0, raw-binary admission, TShark/sigrok lanes, setup footprint) and `docs/research/05-ecm-reprogramming-and-firmware.md` (OKI nX-8/500S, 66Q592, flash, bench read).
