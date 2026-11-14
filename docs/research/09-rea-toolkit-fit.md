# REA reverse-engineering toolkit: what it is and how it fits the Honda OBD project

Research date: 2026-11-14. Web research only; REA was not installed, cloned or run. All repository files were read through `raw.githubusercontent.com`, `api.github.com`, `registry.npmjs.org` and `unpkg.com`. Every page was read through a summarising fetch tool, so exact strings (commands, environment-variable names, limits) are reproduced as returned and must be re-checked against `rea --help` / `rea setup --dry-run` at install time. Documentation on `main` (pushed 2026-11-14) can be slightly ahead of the released npm package 6.9.0 (released 2026-11-11/12).

Confidence tags: **[HIGH]** primary source or two independent sources agree; **[MEDIUM]** one credible source; **[LOW]** inference, single weak source, or conflicting reports.

## Summary

1. REA ("Reverse Engineer Anything", `github.com/morluto/rea`, npm `rea-agents`, MIT) is a TypeScript CLI plus MCP server that puts one "Evidence" contract in front of many external reverse-engineering engines (Ghidra, IDA, Hopper, Kuna, JADX, TShark, sigrok-cli, Binwalk/Unblob, Rizin, objdump, GDB, Frida) and a large set of built-in static parsers. Latest release is 6.9.0; the bundled skill advertises 255 MCP tools.
2. **Blocker on this PC:** 6.9.0 requires Node `^22.23.4 || ^24.22.1 || >=26.12.1` and exits with code 1 on anything else. Node v22.16.0 is too old; Node must be upgraded (staying on 22.x LTS is enough) before anything works.
3. Default install is `npx rea-agents setup`: interactive, shows a plan, asks approval, writes `mcpServers.rea` into `~/.claude.json` and a `reverse-engineer-anything` skill into `~/.claude/skills`, keeps a `.rea.backup`. A single-project install is possible with `claude mcp add --scope local ...` (nothing written into the repo, no skill installed). `rea uninstall` reverses setup.
4. On native Windows REA is only partly functional: Ghidra support is an experimental, read-only "P0" boundary (PE x86/x86-64/ARM, DOS MZ, raw 6502 only), Hopper and Kuna are unavailable, and the firmware (Binwalk/Unblob), packet-field (TShark) and logic-signal (sigrok) lanes are Linux-only. WSL2 or a Linux VM is needed for those.
5. **Good fit:** Honda HDS Windows binaries and J2534 DLLs (built-in PE export/import/resource inspectors plus Ghidra P0 decompilation), .NET assemblies (metadata + CIL, no C#), Android diagnostic apps (JADX lane, Windows x64 supported).
6. **Poor fit today:** ECU firmware. SuperH is not an admitted CPU family on any REA provider; raw-binary images are admitted only for x86, x86_64, arm, arm64, 6502, m68k (on Windows: 6502 only), and `REA_GHIDRA_LANGUAGE_ID` cannot override a raw import. Open upstream items: issue #4246 (caller-selected language/base), issue #3532 and draft PR #4678 (SH-4 only). Stock Ghidra itself does ship `SuperH:BE:32:SH-2`, `SH-2A` and `SH-1`, so ECU firmware should be analysed in Ghidra directly until REA catches up.
7. K-line traffic: REA accepts only saved `pcap`/`pcapng` (TShark 4.x) or logic-analyser recordings decoded by `sigrok-cli 0.7.2` (UART decoder with caller-given baud rate). It does no live capture, guesses no baud rate, and sigrok has no K-line/KWP2000/ISO 9141 decoder, so Honda message framing needs our own parser. Raw UART logs have no REA path without conversion.
8. Trust signals are mixed: npm publishes come from a GitHub Actions trusted publisher with SLSA provenance and 6.9.0 declares no install scripts, but there is a single npm maintainer, the project is 7 months old with 35 releases and frequent breaking changes, the package is 190 MB / 10,585 files with a prebuilt Windows native addon, and the popularity numbers (459k stars, 113k forks, 1.94M downloads/month) are extraordinary and cannot be validated. A third-party audit rates the skill MEDIUM risk.
9. REA imposes an evidence discipline that suits this project: every conclusion must separate observations, inferences and unknowns, cite Evidence IDs, and keep limitations; target content is data, not instructions; MCP paths must be absolute; Evidence persists only when explicitly exported.
10. Its disclaimer supports interoperability and analysis of software you own or are authorised to examine, and explicitly excludes help with circumventing licence checks or technical protection measures; that boundary matters for HDS licensing and immobilizer functions.
11. Recommended role: REA natively on Windows for PC-side targets (HDS, J2534 DLLs, APKs); REA inside WSL2 for signal/packet/firmware lanes; plain Ghidra for the ECU image; re-evaluate when #4246 lands.

## Findings

### 1. What REA is and does

**Identity**

- REA is published as `morluto/rea` on GitHub (description: "Reverse engineer anything with agents, from app behavior down to native binaries."), as the npm package `rea-agents` (description: "Reverse engineer anything from your terminal or agent with one CLI and MCP server."), and at `rea.tools`. The README carries an official-source notice: "Get REA from morluto/rea, rea.tools, or the rea-agents npm package." **[HIGH]** ([GitHub API][api], [npm latest][npmlatest], [README][readme])
- Licence MIT for REA-authored code; third-party material keeps its own licences (`THIRD_PARTY_NOTICES.md`). **[HIGH]** ([GitHub API][api], [npm latest][npmlatest])
- MCP registry name `io.github.morluto/rea`; reference registration is `npx -y rea-agents@6.9.0 mcp` over stdio. **[HIGH]** ([mcp.json][mcpjson], [server.json][serverjson])

**Architecture**

- One entry script `scripts/rea.mjs` (bin names `rea` and `rea-agents`) checks the Node version, then loads either the one-shot CLI (`dist/cli-runtime/...`) or the MCP server (`dist/mcp/main.js`; stdio by default, opt-in plain-HTTP transport with `rea mcp --http`). "CLI commands use the same workflows" as the MCP tools. **[HIGH]** ([rea.mjs][reamjs], [README][readme])
- Internal layering (from the contributor guide): domain -> provider adapters -> application workflows -> CLI/MCP adapters. **[MEDIUM]** ([AGENTS.md][agents])
- Native engines are driven through bridges shipped in the package: Java scripts for headless Ghidra (`bridge/ghidra/ReaGhidraBridge.java`, `ReaGhidraPrepareRaw.java`, `ReaGhidraApplySeeds.java` ...), a Python bridge for Hopper and Cutter, a Java bridge for JADX, Python helpers for pwntools/pwndbg/mitmproxy, and a prebuilt Windows native addon `native/windows/build/rea-windows-x64.node` (process, filesystem and DACL controls). **[HIGH]** ([package.json][pkg])
- Results are returned as "Evidence": records with `evidence_id`, `provider`, `coverage`, `limitations`, `residual_unknowns`, `evidence_links`; there is an "Unknown ledger" (`record_unknown`, `update_unknown`, `list_unknowns`). Each MCP connection has its own in-memory Evidence ledger; records persist only through exported bundles. **[MEDIUM]** ([MCP contracts][mcpc], [CLI reference][cli])
- The bundled skill `reverse-engineer-anything` declares `version "62"` and `tool_count 255`. The full generated tool catalogue (`docs/public/product-catalog.json`) is not committed; it has to be read from a running server (`tools/list`, `binary_session`). **[MEDIUM]** ([SKILL.md 6.9.0][skill], [MCP contracts][mcpc])

**Back-ends and what each needs**

| Back-end | Used for | Who supplies it | Version stated | Native Windows host | Source / confidence |
|---|---|---|---|---|---|
| Ghidra | Disassembly, decompilation, xrefs, strings, byte search | Caller ("Setup does not install Ghidra or Java") | 12.x (>=12.1), verified 12.1.4; 64-bit full JDK 21+ (12.2 needs JDK 25+) | Experimental "P0", read-only | [install][inst], [Windows P0][winp0] **[HIGH]** |
| IDA | Same, via `ida-pro-mcp` / `ida-free-mcp` / `idalib-mcp` registrations | Caller, own licence | Verified IDA Pro 9.3 (Windows), 9.1 headless (macOS arm64) | Yes | [IDA provider][ida] **[MEDIUM]** |
| Hopper | Native analysis | Setup can install after approval (macOS, Linux) | current Hopper | No (the guide lists Hopper as unavailable on Windows) | [install][inst] **[MEDIUM]** |
| Kuna | Decompiler (Apache-2.0, Noelo-Lab) | Caller (`REA_KUNA_COMMAND`) | 1.990 recommended | No ("Windows is not supported yet.") | [Kuna provider][kuna] **[MEDIUM]** |
| JADX | APK decompilation | Caller: `jadx-headless-mcp` 0.7.1 JAR + full JDK 17+ (`REA_JADX_MCP_JAR`, `JAVA_HOME`) | 0.7.1 | Yes, x64 (see conflicts) | [Android guide][android], [README][readme] **[MEDIUM]** |
| JEB | Android/other, commercial | Caller-started JEB client (`REA_JEB_MCP_URL`) | not checked | not checked | [README][readme] **[MEDIUM]** |
| Binwalk / Unblob | Firmware signature scan / extraction | Caller (`REA_BINWALK_COMMAND`, `REA_UNBLOB_COMMAND`) + util-linux `prlimit` | Binwalk 3.1.x (verified 3.1.0), Unblob 26.6.x (verified 26.6.4) | No ("verified on Linux x64") | [firmware guide][fw] **[HIGH]** |
| TShark / capinfos | Saved packet captures | Caller (`REA_TSHARK_COMMAND`, `REA_CAPINFOS_COMMAND`) | Wireshark 4.x | Field lane: no. Summary lane: claimed, unverified | [packet guide][pkt], [pcap guide][pcap] **[HIGH]** |
| sigrok-cli | Logic-analyser recordings | Caller (`REA_SIGROK_COMMAND`) + `prlimit` | 0.7.2 with libsigrokdecode | No ("macOS and Windows are refused") | [signal guide][sig] **[HIGH]** |
| GNU objdump, Rizin | One-shot header/disassembly commands | Caller (`REA_OBJDUMP_COMMAND`, `REA_RIZIN_COMMAND`) | Rizin 0.6-0.9.1 described | Not stated | [tools doc][tools] **[MEDIUM]** |
| GDB / pwndbg / pwntools | Debug sessions, ELF layout, recorded crashes | Caller | n/a | No (Linux only) | [tools doc][tools], [binary diagnostics][bindiag] **[MEDIUM]** |
| Frida | Runtime instrumentation | Optional npm dependency `frida` 17.23.2 | 17.23.2 | Not documented (verifier needs Linux) | [npm latest][npmlatest], [Frida guide][frida] **[MEDIUM]** |
| adb, Apktool | Android device and resource lanes | Caller | n/a | not checked | [README][readme] **[MEDIUM]** |
| Docker + AgentEnv | Only for the "RL environment" feature (`agent-env run`) | Caller | n/a | irrelevant here | [README][readme] **[MEDIUM]** |
| Built-in parsers | PE imports/exports/resources/certificates/load config, ELF, Mach-O, .NET metadata, Go, Rust, IL2CPP, SQLite, Intel HEX/S-record, UF2/DfuSe, ESP, FIT/uImage, devicetree, entropy, JavaScript/Electron | REA itself | n/a | Mostly host-independent | [README][readme], [CLI reference][cli] **[MEDIUM]** |

**MCP tools most relevant to this project** (names verbatim; the catalogue is larger)

| Family | Tools | Purpose | Source |
|---|---|---|---|
| Session | `open_binary`, `close_binary`, `binary_session`, `reset_investigation`, `inspect_analysis_settings`, `update_analysis_settings`, `discover_tools` | Select target/provider, report availability, tune limits, end session | [MCP contracts][mcpc] |
| Evidence | `inspect_analysis_view`, `get_evidence_bundle`, `export_evidence_bundle`, `record_unknown`, `update_unknown`, `list_unknowns` | Page retained results, export bundles, track open questions | [MCP contracts][mcpc] |
| Native (Ghidra/IDA/Hopper) | `binary_overview`, `list_procedures`, `search_procedures`, `list_strings`, `search_strings`, `list_names`, `list_segments`, `read_bytes`, `search_bytes`, `address_to_file_offset`, `procedure_pseudo_code`, `procedure_assembly`, `read_function_instructions`, `inspect_native_instruction`, `procedure_callers`, `procedure_callees`, `procedure_references`, `xrefs`, `analyze_function`, `compare_functions`, `batch_decompile`, `decompile_all`, `get_call_graph`, `trace_call_path`, `trace_native_values`, `trace_feature`, `match_native_functions` | Inventory, search, decompile, cross-reference, function dossiers, value flow | [MCP contracts][mcpc], [Ghidra provider][ghidra] |
| Artifact | `inspect_artifact`, `inventory_artifact`, `extract_artifact`, `compare_artifacts`, `inspect_desktop_application`, `inspect_byte_entropy`, `inspect_native_exports`, `inspect_pe_resources`, `inspect_binary_layout` | Containers (ZIP/APK/IPA/MSIX/AppX/ASAR/DMG, Intel HEX, S-record, UF2 ...), entropy, PE/ELF tables | [skill reference][skillref], [binary diagnostics][bindiag], [hex records][hex] |
| Firmware | `inspect_firmware_regions`, `extract_firmware`, `inspect_devicetree` | Binwalk scan, Unblob extraction | [firmware guide][fw] |
| Packets | `inspect_packet_capture`, `inspect_packet_fields` (fields, display filter, decode-as); `inspect_pcap_capture` (fixed summaries) | Saved pcap/pcapng via TShark | [packet guide][pkt], [pcap guide][pcap] |
| Signals | `decode_signal_recording`, `inspect_signal_decoders` | sigrok-cli protocol decoding of recordings | [signal guide][sig] |
| Android | `inspect_android_package`, `search_android_classes`, `inspect_android_class`, `inspect_android_method`, `trace_android_references`, `inspect_android_jni_bindings`, `project_android_application_graph`, `inspect_apk_signature` | Manifest, classes, Java/smali method bodies, references, JNI links | [Android guide][android] |
| Managed (.NET) | `inspect_managed_artifact`, `inspect_managed_members`, `inspect_managed_native_boundaries`, `compare_managed_members`, `verify_managed_native_boundaries`, `import_managed_reconstruction`, `project_managed_application_graph` | Assembly identity, types, CIL, `ldstr` strings, P/Invoke declarations | [managed guide][managed] |
| Direct tools | `inspect_with_objdump`, `execute_rizin_command`, `start_gdb_session` ..., `start_rizin_debug_session` ... | Caller-supplied objdump, Rizin, GDB | [tools doc][tools] |
| Frida | `list_frida_devices`, `list_frida_processes`, `start_frida_session`, `load_frida_script`, `resume_frida_session`, `unload_frida_script`, `frida_session_status`, `close_frida_session` | Spawn/attach and inject scripts | [Frida guide][frida] |
| Guidance | `guided_workflows`, `search_guidance`, `read_guidance_section` | Built-in documentation lookup | [MCP contracts][mcpc] |

Tool families named for `REA_MCP_TOOL_FAMILIES` / `--tool-families`: `session`, `firmware`, `direct`, `browser`, `pcap`, `javascript-runtime`, `android`, `adb`, `apktool`, `frida`, `jeb`, `flutter`, `native`, `artifact`, `enhanced`, `application`, `managed`, `electron`. Unset means the complete catalogue is advertised. **[MEDIUM]** ([MCP contracts][mcpc])

MCP prompts: `investigate_feature`, `compare_application_versions`, `verify_reconstruction`, `trace_crash`, `audit_residual_unknowns`, `prepare_bounded_process_capture`. **[MEDIUM]** ([MCP prompts][prompts])

**CLI commands most relevant to this project** (verbatim; all from [CLI reference][cli] unless noted, **[MEDIUM]**)

| Command | Purpose |
|---|---|
| `rea setup`, `rea doctor`, `rea providers`, `rea capabilities`, `rea --version`, `rea update`, `rea uninstall` | Install/readiness/lifecycle (`update`, `uninstall` from [install][inst]) |
| `rea mcp`, `rea mcp add`, `rea mcp doctor` | Serve MCP, register with clients, check MCP startup |
| `rea analyze`, `rea inspect`, `rea search`, `rea search-bytes`, `rea function`, `rea decompile`, `rea xrefs`, `rea trace`, `rea instructions`, `rea address-to-file-offset` | Provider-backed native analysis |
| `rea inspect-artifact`, `rea extract-artifact`, `rea compare-artifacts` | Containers and record formats |
| `rea inspect-native-exports`, `rea inspect-pe-imports`, `rea inspect-pe-resources`, `rea inspect-pe-certificates`, `rea inspect-pe-load-config`, `rea inspect-binary-hardening`, `rea inspect-byte-entropy` | Built-in PE/ELF/byte inspectors ([binary diagnostics][bindiag]) |
| `rea inspect-managed-artifact`, `rea inspect-managed-members`, `rea compare-managed-members` | .NET |
| `rea inspect-android-package`, `rea search-android-classes`, `rea inspect-android-class`, `rea inspect-android-method`, `rea trace-android-references`, `rea inspect-android-jni-bindings` | APK via JADX ([Android guide][android]) |
| `rea inspect-firmware-regions`, `rea extract-firmware` | Binwalk / Unblob ([firmware guide][fw]) |
| `rea inspect-packet-capture`, `rea inspect-packet-fields`, `rea inspect-pcap-client`, `rea inspect-pcap-capture` | TShark lanes ([packet guide][pkt], [pcap guide][pcap]) |
| `rea decode-signal-recording`, `rea inspect-signal-decoders` | sigrok lane ([signal guide][sig]) |
| `rea inspect-with-objdump`, `rea list-objdump-architectures`, `rea execute-rizin-command`, `rea debug-with-gdb`, `rea debug-with-rizin` | Caller-supplied tools ([tools doc][tools]) |
| `rea instrument-with-frida` | One-shot Frida script ([Frida guide][frida]) |
| `rea inspect-sqlite-database` | SQLite structure and samples (needs Node 24.15+ or 26+ per [README][readme]) |
| `rea evidence-export`, `rea evidence-import`, `rea compare`, `rea inspect-analysis-view` | Evidence bundles |

Common flags: `--json`, `--format json|jsonl|md|csv|tsv|yaml` (TOON default), `--provider`, `--evidence_output`, `--detail summary|complete`, `--snapshot`. Exit codes: 0 completed (may still be partial), 1 could not complete. **[MEDIUM]** ([CLI reference][cli])

### 2. Installation and footprint on Windows 11 with Claude Code

**Node.js requirement (blocker)**

- `engines.node` of 6.9.0 is `^22.23.4 || ^24.22.1 || >=26.12.1`. README wording: "REA requires Node.js 22.x (>=22.23.4), 24.x (>=24.22.1), or 26.12.1+, plus npm." Node 23, 25, earlier 26.x and prereleases are unsupported. **[HIGH]** ([npm latest][npmlatest], [README][readme], [package.json][pkg])
- The check is enforced at start-up, not just advisory: `scripts/rea.mjs` writes the message to stderr and calls `process.exit(1)`; no bypass flag or environment variable is visible. The PC's Node v22.16.0 therefore cannot run REA 6.9.0. **[HIGH]** ([rea.mjs][reamjs], [runtime-admission.mjs][admit])
- This requirement arrived as a breaking change in 6.8.0 (2026-10-25). **[MEDIUM]** ([CHANGELOG][changelog])
- Python 3.13 on the PC is irrelevant to the core tool; Python is used only by optional bridges (Hopper, Cutter, pwntools, mitmproxy). **[MEDIUM]** ([package.json][pkg])

**Install methods** (commands verbatim from the guide; none were run)

| Method | Command | Effect | Source |
|---|---|---|---|
| Guided setup via package runner | `npx rea-agents setup` (or `npx rea-agents@latest setup`) | Registers MCP + installs skill for selected agents; registration points into the npx cache ("keep its cache entry available") | [install][inst], [README][readme] |
| Global install | `npm install --global rea-agents` then `rea setup` | Same, with a stable install path; `rea update` works | [install][inst] |
| Dry run | `rea setup --client claude_code --dry-run` | Read-only plan, status `planned`, exit 0 | [install][inst] |
| One project, no setup | `claude mcp add --scope local rea -- "C:\Program Files\nodejs\node.exe" "C:\tools\rea\scripts\rea.mjs" mcp` | Local-scope registration; no skill; `rea doctor`, `rea uninstall`, `rea update` do not see it | [install][inst] |
| Separate Claude profile | `CLAUDE_CONFIG_DIR=~/.claude-rea npx rea-agents setup --client claude_code` (PowerShell: `$env:CLAUDE_CONFIG_DIR`) | Writes `$CLAUDE_CONFIG_DIR/.claude.json` and `$CLAUDE_CONFIG_DIR/skills` instead of the default profile | [install][inst] |
| Claude Code plugin | `/plugin marketplace add morluto/rea` then `/plugin install rea@rea` | Self-contained plugin (since 6.9.0); MCP launched with `node ${CLAUDE_PLUGIN_ROOT}/scripts/plugin-mcp-launcher.mjs 6.9.0`; skills from `./skills` | [install][inst], [plugin.json][plugin], [marketplace.json][market] |
| Skill only | `npx skills add morluto/rea --skill reverse-engineer-anything` | Instructions only, no MCP registration or engines | [install][inst] |
| `rea mcp add` | `rea mcp add` with `--agent`, `--no-global` | Registers the current launcher; no skills; `--no-global` "writes project-level settings only" | [install][inst] |
| curl installer | `curl -fsSL https://raw.githubusercontent.com/morluto/rea/main/install.sh \| bash` | POSIX shell script, "not signature-verified"; not applicable to PowerShell | [install][inst] |

All **[MEDIUM]** (single documentation source), except the two `npx`/`npm` commands which the README repeats **[HIGH]**.

**What `setup` changes on the machine (Claude Code)**

| Item | Change | Backup / safety | Confidence |
|---|---|---|---|
| `~/.claude.json` (or `$CLAUDE_CONFIG_DIR/.claude.json`) | Adds key `mcpServers.rea`: absolute Node (or Bun) runtime + REA entry script + `mcp`, optional `env`. "Unrelated entries and comments" preserved | First snapshot kept as `.rea.backup` (never replaced); if the file later differs, a `.rea.backup.<sha256>.recovery` copy is made before editing; writes are staged and verified | **[MEDIUM]** ([install][inst], [client config][client]) |
| `~/.claude/skills` (or `$CLAUDE_CONFIG_DIR/skills`) | Installs the bundled `reverse-engineer-anything` skill with its `references` directory | Byte-exact backups of changed skill files; refuses symlinked skill directories; `--skill=false` omits it | **[MEDIUM]** ([install][inst]) |
| `CLAUDE.md`, `settings.json`, hooks, permissions, slash commands | Not mentioned anywhere as modified | Verify with `--dry-run` (absence in docs is not proof) | **[LOW]** ([client config][client]) |
| Ghidra, Java, Node, IDA | Never installed or upgraded by setup | Product rule: "`rea setup` must show its planned changes and require approval before writing files or installing Hopper." | **[HIGH]** ([AGENTS.md][agents], [install][inst], [SKILL.md 6.9.0][skill]) |
| Hopper | Optional download/install with separate approval; not offered on Windows | n/a | **[MEDIUM]** ([install][inst]) |

- The plan shows "exact paths, changes, external software, network origins, integrity evidence, and package-manager commands"; declining or Ctrl-C changes nothing. Interactive mode needs a real terminal (stdin, stdout and stderr). **[MEDIUM]** ([install][inst])
- Setup flags: `--client <id>` (repeatable), `--all-detected`, `--skill=false`, `--skill-client <id>`, `--dry-run`, `--yes`, `--json`, `--install-hopper`, `--accessible`, `--tool-families`. **[MEDIUM]** ([install][inst], [client config][client])
- Claude Code scopes (Claude Code's own documentation): local scope is the default, "loads only in the project where you added it", and is stored "in `~/.claude.json` under that project's path"; project scope writes `.mcp.json` into the project root; precedence is local, project, user, plugin. So `claude mcp add --scope local` keeps the repository clean. Environment values are passed with `--env KEY=value`. **[HIGH]** ([Claude Code MCP docs][ccmcp], [install][inst])
- Windows notes from the guide: use PowerShell, not cmd.exe; use `npm.cmd` / `npx.cmd` for cache recovery; quit Claude Desktop fully before approving setup because a running app can overwrite its registration. **[MEDIUM]** ([install][inst])

**Windows prerequisites for the Ghidra lane** ([Windows P0][winp0], [install][inst]; **[MEDIUM]** unless noted)

| Requirement | Value |
|---|---|
| Host | Windows 10+ x64; ordinary non-elevated user |
| Volumes | Target and temporary directories on fixed local NTFS or ReFS volumes; no UNC paths, SUBST aliases, reparse points/junctions, alternate streams |
| Ghidra | Operator-installed 12.x (>=12.1), verified with 12.1.4 |
| Java | 64-bit full JDK within Ghidra's range (JDK 21+ for 12.1) |
| Environment | `$env:GHIDRA_INSTALL_DIR = "C:\tools\ghidra_12.1.4_PUBLIC"`, `$env:JAVA_HOME = "C:\tools\jdk-21"`, `$env:REA_ANALYSIS_PROVIDER = "ghidra"` |
| Temp | `TMP` and `TEMP` must point to a writable local directory with no dot-prefixed path element |
| Checks | `rea doctor --json`, `rea providers --json`, `rea inspect "C:\fixtures\sample.exe" --provider ghidra --format json` |
| Heap | Default `2G` (`GHIDRA_HEADLESS_MAXMEM`) |
| Start-up deadline | Default 330,000 ms (`REA_GHIDRA_STARTUP_TIMEOUT_MS`); the project's own Windows runs saw the first function query take roughly 407-601 s on small fixtures with 512 MiB heaps |

**JADX lane prerequisites**: `jadx-headless-mcp` 0.7.1 JAR (guide gives a pinned SHA-256, reported to me as `6e5eacf500b64292bfb73c49797c1958f6ee44646e43e868039ae7feb573ff75`; re-check against REA's own metadata), full JDK 17+ including `jdk.compiler`, `REA_JADX_MCP_JAR` set to the absolute JAR path; the guide has PowerShell download-and-verify commands. **[MEDIUM]** ([Android guide][android])

**Update and uninstall**

- Update: `rea update` (alias `rea --update`) for npm/Bun global installs; it "installs the exact resolved release", verifies the CLI version and keeps dependency lifecycle scripts disabled. Package-runner installs use `npx rea-agents@latest setup`. Local-scope entries must be recreated by hand. **[MEDIUM]** ([install][inst], [README][readme])
- Uninstall: `rea uninstall` removes REA-owned agent registrations and skill files and preserves Evidence files, captures, Node, other MCP servers; `rea uninstall --purge-data` also removes `~/.rea` (legacy cache/state) and provider caches. Local scope: `claude mcp remove --scope local rea` from the same directory. Malformed client configuration stops uninstall before anything is removed. **[MEDIUM]** ([install][inst])

**Footprint**

- Package 6.9.0: 10,585 files, 189,641,205 bytes unpacked, 42 runtime dependencies all shipped inside the tarball (`bundleDependencies` + `npm-shrinkwrap.json`), one optional dependency `frida` 17.23.2. **[HIGH]** ([npm latest][npmlatest], [package.json][pkg])
- 255 MCP tools is a large catalogue for an agent context; mitigations are `--tool-families` / `REA_MCP_TOOL_FAMILIES`, the compact input-schema profile (default) and `REA_MCP_TOOL_CATALOG_PROFILE=hot-cold` with `discover_tools`. **[MEDIUM]** ([SKILL.md 6.9.0][skill], [client config][client], [MCP contracts][mcpc])

### 3. Trust and maintenance signals

**Repository (GitHub API, cross-checked against the repository web page)**

| Field | Value | Confidence |
|---|---|---|
| Created | 2026-04-14T00:51:32Z | **[HIGH]** ([API][api]) |
| Last push | 2026-11-14T17:00:59Z | **[HIGH]** ([API][api]) |
| Stars | 459,387 (web page: 459.4k) | **[HIGH]** as a reported figure ([API][api], [web][web]) |
| Forks | 112,608 (web page: 112.6k) | **[HIGH]** as a reported figure |
| Watchers (subscribers) | 1,739 | **[HIGH]** as a reported figure |
| Open issues + PRs | 1,541 (web page: 757 issues + 784 pull requests) | **[HIGH]** |
| Commits | 7,532 | **[MEDIUM]** ([web][web]) |
| Licence | MIT | **[HIGH]** |
| Language / size | TypeScript; 163,736 KB | **[HIGH]** |
| Owner | `morluto`, a User account created 2020-12-22, 13,823 followers, 263 public repos | **[MEDIUM]** ([owner API][owner]) |
| Contributors | At least 30; owner 4,333 commits, then 709, 659, 271 ...; includes `cursoragent` and `github-actions[bot]` | **[MEDIUM]** ([contributors API][contrib]) |
| Releases | `rea-agents-6.9.0` published 2026-11-12, `6.8.0` 2026-10-25, `6.0.0` 2026-10-08, by `github-actions[bot]` | **[HIGH]** ([releases API][rels], [6.0.0 release][rel600]) |
| Issue numbering | Issues/PRs already above #10,000 (e.g. #10476 on 2026-11-14) | **[MEDIUM]** ([issue search][issuesearch]) |

**npm registry**

| Field | Value | Confidence |
|---|---|---|
| Latest | 6.9.0 (`dist-tags.latest`) | **[HIGH]** ([npm][npm], [npm latest][npmlatest]) |
| First publish | 2026-07-12T12:46:31Z (version 0.2.1) | **[MEDIUM]** ([npm][npm]) |
| Versions | 35 between 0.2.1 and 6.9.0 (about four months) | **[MEDIUM]** ([npm][npm]) |
| Maintainers | One: `morluto` | **[HIGH]** ([npm latest][npmlatest]) |
| Publisher of 6.9.0 | GitHub Actions trusted publisher (OIDC), SLSA v1 provenance attestation, registry signatures | **[HIGH]** ([npm latest][npmlatest]) |
| Install scripts in 6.9.0 | `preinstall`, `install`, `postinstall` absent. `prepare` and `prepack` exist but do not run for registry installs. Versions 1.2.0-1.7.0 did have `"postinstall": "node scripts/prepare-node-pty.mjs"` | **[HIGH]** for 6.9.0 ([package.json][pkg]); **[MEDIUM]** for the old versions ([npm][npm]) |
| Optional dependency | `frida` 17.23.2 with `allowScripts: {"frida@17.23.2": true}`; Frida's own install step fetches a prebuilt native binding (the guide mentions a `prebuild-install` warning) | **[MEDIUM]** ([package.json][pkg], [install][inst]) |
| Downloads | 1,944,247 in 2026-10-15..2026-11-13 (three days report 0) | **[MEDIUM]** ([npm downloads][npmdl]) |
| Notable bundled dependencies | `playwright-core`, `@lydell/node-pty`, `isomorphic-git`, `openpgp`, `ws`, `@modelcontextprotocol/server` 2.3.1 | **[HIGH]** ([npm latest][npmlatest]) |

**Mismatches between README claims and the APIs**

- README says "400,000 GitHub stars"; the API shows 459,387. The README lags rather than overstates. **[HIGH]** ([README][readme], [API][api])
- An independent article dated 2026-10-14/18 saw a README banner of 180,000 stars and "over 200,000" on the repository. If accurate, stars more than doubled in about four weeks. Combined with a fork-to-star ratio near 0.25 and only 1,739 watchers, the popularity figures are unusual and I could not validate them independently; treat them as no evidence of quality. The article itself says "Popularity is not a quality signal." **[LOW]** for any interpretation ([developersdigest][dd], [API][api])
- The README target table says APK analysis works on "Linux/macOS/Windows x64", while the Android guide says the Windows boundary "is new on repository main; an older released package can still reject this host". **[MEDIUM]** ([README][readme], [Android guide][android])
- The README does not mention Frida, yet the package carries it as an optional dependency and ships a Frida guide. **[MEDIUM]** ([README][readme], [npm latest][npmlatest])
- No mismatch found for licence (MIT in both), version (6.9.0 in npm, `mcp.json`, `server.json`, plugin manifests) or Node requirement. **[HIGH]**

**Network access and data locality**

- README: "REA analyzes targets locally. Your agent receives the tool results, and its model provider follows its own data policy." So binaries stay on disk, but whatever the tools return (decompiled code, strings) goes to the model provider through the agent. **[HIGH]** ([README][readme])
- Documented network use: npm/Bun for install and update, optional Hopper download during setup (not on Windows), and operation-driven network use (browser capture, website analysis, IDA Free's cloud decompiler). `scripts/rea.mjs` itself makes no network request at start-up. **[MEDIUM]** ([install][inst], [SECURITY.md][sec], [rea.mjs][reamjs])
- No telemetry statement exists in README, SECURITY.md or the installation guide, and an issue search for "telemetry" found no description of REA-side telemetry. This is absence of a statement, not a code audit. **[LOW]** ([README][readme], [SECURITY.md][sec], [telemetry issue search][telsearch])
- The HTTP transport is plain HTTP with no TLS or client authentication; since 6.9.0 non-loopback binds need an explicit allowed-hosts setting. stdio is the default and is all this project needs. **[MEDIUM]** ([SECURITY.md][sec], [CHANGELOG][changelog])

**Third-party assessments**

- skills.sh "Agent Trust Hub" audit of the skill (2026-11-11): overall MEDIUM risk; categories COMMAND_EXECUTION, REMOTE_CODE_EXECUTION ("The skill facilitates the execution of remote code through the `npx -y rea-agents@latest` command."), INDIRECT_PROMPT_INJECTION (untrusted binaries/captures as input), CREDENTIALS_UNSAFE, DYNAMIC_EXECUTION (Frida scripts). **[MEDIUM]** ([skills.sh audit][trusthub])
- The one independent article found did not run REA; it relies on the README and forum threads. I found no hands-on independent review on Windows. **[MEDIUM]** ([developersdigest][dd])
- Socket.dev's page for the package returned HTTP 403, so its supply-chain score could not be read. **[HIGH]** (observation)

### 4. Fit to this project's targets

| Target | REA lane | Runs on native Windows 11? | Verdict |
|---|---|---|---|
| a. HDS dealer software | Built-in PE inspectors; Ghidra P0 for native PE; managed (.NET) inspectors; `inspect_artifact` for some containers | Yes (experimental, read-only) | Useful, with gaps (no Java lane, no CAB/MSI/InstallShield, no C# output) |
| b. ECU firmware (SuperH raw image) | None admitted | No (and not on Linux either) | Not usable today; use Ghidra directly |
| c. Android diagnostic APKs | JADX lane | Yes, x64 (confirm after install) | Good |
| d. K-line captures | TShark lane (pcap/pcapng) or sigrok lane (logic recordings) | No (Linux only) | Partial, via WSL2; framing needs own parser |
| e. Clone cable / ELM327 firmware | Intel HEX / S-record inspector, entropy, Binwalk; native lane only for some ARM | Partly | Marginal |
| f. Windows J2534 DLLs | Built-in PE inspectors; Ghidra P0 | Yes | Good |

**a. Honda HDS (Windows application)**

- Built-in, engine-free inspectors work on explicit PE files: `rea inspect-native-exports` ("reads the export table of an explicit PE or ELF image on any supported host"), `rea inspect-pe-imports` (imports and delay-load tables), `rea inspect-pe-resources`, `rea inspect-pe-certificates`, `rea inspect-pe-load-config`, `rea inspect-binary-hardening`, `rea inspect-byte-entropy`. These give a quick map of which EXE/DLL exports what and which serial/USB APIs each imports. **[MEDIUM]** ([binary diagnostics][bindiag], [CLI reference][cli])
- Native code: Windows Ghidra P0 admits non-managed PE applications and DLLs for x86, x86-64, ARMNT and ARM64. x86 (32-bit) PE support on Windows arrived in 6.0.0 ("**ghidra:** support native x86 PE applications on Windows"), so a 32-bit HDS is admitted by 6.9.0. 27 read-only inventory, memory and function-analysis operations are available; DLLs "are imported as immutable copies" and never loaded. **[HIGH]** for admission ([Windows P0][winp0], [6.0.0 release][rel600])
- Limits on Windows: no renaming, comments or other database mutation ("Windows P0 does not admit database mutations"; on Linux/macOS only atomic function-name and entry-comment edits exist); no Ghidra GUI state; an operator-prepared Ghidra project cannot be reused on Windows ("Windows, raw/DOS/DOL and special pinned-language import profiles require fresh import"); cold import of a large binary can exceed the 330 s default deadline. **[MEDIUM]** ([Windows P0][winp0], [install][inst], [skill reference][skillref])
- .NET parts: built-in parser returns types, methods, signatures, CIL bodies, `ldstr` user strings, resources as metadata rows and declared P/Invoke module/entry-point pairs. It does **not** decompile to C#; `ilspycmd` output can be imported as non-canonical inference with `import_managed_reconstruction`. Mixed-mode C++/CLI native bodies are not mapped. **[MEDIUM]** ([managed guide][managed])
- Java parts: no JAR/class lane is documented; the skill reference "never mentions Java bytecode, JAR files, or class files". An open PR (#6147) mentions a "JVM reader". Use JADX/CFR directly, outside REA. **[MEDIUM]** ([skill reference][skillref], [telemetry issue search][telsearch])
- Installers and data: `inspect_artifact` covers "application bundles, archives, ZIP/APK/IPA/MSIX/AppX, ASAR, or DMG inputs"; CAB, MSI, 7z, NSIS and InstallShield are not mentioned, so unpacking an HDS installer needs other tools. `inspect_desktop_application` only detects .NET, Unity, Godot, GameMaker, Unreal and Electron families and does not inventory Java, databases or INI/XML files. `rea inspect-sqlite-database` handles SQLite only. Honda's DTC/parameter databases in a proprietary format would need custom parsing. **[MEDIUM]** ([skill reference][skillref], [desktop applications][desk])
- Finding protocol tables: realistic workflow is `search_strings` / `search_bytes` for known request bytes, then `xrefs`, `analyze_function`, `procedure_pseudo_code`, `trace_native_values` around the hits, exporting an Evidence bundle per binary. **[LOW]** (my inference from the tool list)
- Runtime logging of the HDS-to-interface dialogue with REA's Frida lane is theoretically possible (spawn/attach, inject script), but Windows host support for that lane is undocumented and its verifier requires Linux. **[LOW]** ([Frida guide][frida])

**b. ECU firmware dumps**

- Headerless images go through `open_binary` with `format: "raw-binary"`, an `architecture` and an `entry_point` (CLI: `rea inspect <path> --target-format raw-binary --architecture <a> --entry-point 0x... --provider ghidra`). Only Ghidra accepts them: "Hopper and IDA refuse this interpretation." **[HIGH]** ([raw-binary][rawbin], [skill reference][skillref])
- Admitted raw architectures and their fixed Ghidra languages:

| `architecture` | Ghidra language | Compiler spec |
|---|---|---|
| `x86` | `x86:LE:32:default` | `gcc` |
| `x86_64` | `x86:LE:64:default` | `gcc` |
| `arm` | `ARM:LE:32:v7` | `default` |
| `arm64` | `AARCH64:LE:64:v8A` | `default` |
| `6502` | `6502:LE:16:default` | `default` |
| `m68k` | `68000:BE:32:default` | `default` |

  SuperH, PowerPC, MIPS, V850, H8, M16C and Oki families are absent; Thumb-only ARM, Cortex-M, big-endian ARM, 68020, CPU32 and ColdFire are explicitly not admitted. **[HIGH]** ([raw-binary][rawbin], [skill reference][skillref], [issue #4246][i4246])
- On Windows the raw lane is narrower still: "Windows Ghidra P0 admits raw images only for `6502`." **[HIGH]** ([raw-binary][rawbin], [skill reference][skillref])
- The language override does not rescue this: issue #4246 quotes the refusal "REA_GHIDRA_LANGUAGE_ID cannot override the admitted raw-binary import.", and the installation guide says raw-binary opens are refused while an override is set. **[HIGH]** ([issue #4246][i4246], [install][inst])
- A pre-built Ghidra project does not rescue it either: project reuse "supports native ELF, PE and Mach-O programs on POSIX" only, checks the program's recorded import SHA-256, and excludes raw imports and Windows. **[MEDIUM]** ([install][inst])
- The IDA provider admits only x86, x86_64, arm, arm64: "Every other CPU family, including families that REA recognizes later, is refused until REA admits it explicitly." Kuna takes only ELF and thin Mach-O for x86-64, AArch64 and ARM. **[MEDIUM]** ([IDA provider][ida], [Kuna provider][kuna])
- Upstream status: issue #3532 "[Feature] Headerless SuperH-4 (SH-4) raw-binary lane for the Ghidra provider" (open, 2026-10-14); draft PR #4678 "feat(ghidra): support raw SH-4 binaries" (little-endian SH-4 only); issue #2410 reports REA refusing SuperH PE files; design issue #4246 "[Design] Caller-selected Ghidra language and base address for raw-binary imports" (open, last updated 2026-11-05) lists SuperH among unsupported embedded CPUs and proposes a generic `language_id` plus a separate `base_address`. None targets big-endian SH-2. **[HIGH]** ([issue search][issuesearch], [issue #4246][i4246])
- Even where a raw lane exists, REA maps the whole file at one address (the entry point doubles as load address), reads no reset vector or vector table, labels no peripherals and supports one segment only; "No emulation of reset vectors, memory-mapped I/O, or firmware boot sequences." A microcontroller image with separate ROM, RAM and I/O regions is poorly served. **[MEDIUM]** ([raw-binary][rawbin])
- What Ghidra itself offers: the stock SuperH module defines `SuperH:BE:32:SH-2A`, `SuperH:BE:32:SH-2` and `SuperH:BE:32:SH-1` (all big-endian, compiler `default`). So Ghidra used directly (GUI or `analyzeHeadless` with the binary loader and an explicit base address) is the right tool if the ECU is SH-2 class. **[HIGH]** ([Ghidra superh.ldefs][ghsh])
- REA pieces that still help with firmware: `rea inspect-artifact` / `rea extract-artifact` for Intel HEX (I8HEX/I16HEX/I32HEX) and Motorola S-record (S19/S28/S37) files, with checksum verification and one raw `.bin` per contiguous region, up to 128 MiB; `rea inspect-byte-entropy` to spot erased, compressed or encrypted areas; `compare_artifacts` between two dumps. **[MEDIUM]** ([hex records][hex], [binary diagnostics][bindiag])
- Possible fallback inside REA: `execute_rizin_command` accepts `asm_arch`, `asm_bits`, `asm_cpu` and `base_address`, so a caller-supplied Rizin could disassemble an arbitrary-CPU raw image. Host support for this one-shot command is not stated and Rizin's SuperH coverage was not checked. **[LOW]** ([tools doc][tools])
- Untested idea: wrap the dump in an ELF32 big-endian container with machine type SuperH and open it under Linux REA. Whether REA admits that machine type is undocumented (it refuses families it has not admitted elsewhere). **[LOW]**

**c. Android apps with Honda diagnostics**

- Lane: `rea inspect-android-package`, `search-android-classes`, `inspect-android-class`, `inspect-android-method` (Java source or smali), `trace-android-references`, `inspect-android-jni-bindings`, plus `inspect-apk-signature`. Output includes decoded manifest, permissions, components, class/field/method inventories with static constant values, incoming references and JNI links to bundled `.so` files; optional R8/ProGuard `mapping_path`. **[MEDIUM]** ([Android guide][android])
- Limits: 64 MiB per DEX member, 120 s default per operation (`REA_JADX_OPERATION_TIMEOUT_MS`), no split-APK/AAB/XAPK handling, no runtime capture, no repackaging, obfuscated names stay obfuscated without a mapping file. **[MEDIUM]** ([Android guide][android])
- Native libraries inside an APK are ELF; Windows Ghidra P0 does not admit ELF, so analysing them through REA needs Linux/WSL2 (or Ghidra directly). **[MEDIUM]** ([Windows P0][winp0])

**d. K-line serial captures**

- Packet lane (`inspect_packet_capture`, `inspect_packet_fields`): needs TShark 4.x and util-linux `prlimit`; "Unsupported hosts, including macOS and Windows, are refused before inspection". It supports display filters, field extraction and decode-as overrides, on already saved captures: the guides name "pcap/pcapng" and nothing else, and the verified fixtures are Ethernet/IPv4/UDP/DNS and Bluetooth HCI PCAPs ("Other formats (including PCAPNG), USB, BLE ATT and actual physical capture are not established by these fixtures"). Default input cap 32 MiB, 30 s per command. "This tool does not acquire traffic, open an interface or control a device." **[HIGH]** ([packet guide][pkt])
- Summary lane (`inspect_pcap_capture`): "requires Linux with util-linux prlimit or Windows with owned Job Objects" but "Actual Windows/Bun Wireshark execution is unverified here"; it returns only IP-centric inventories (endpoints, DNS, TLS SNI, HTTP URIs), which say nothing about a serial bus. **[MEDIUM]** ([pcap guide][pcap])
- Raw UART logs (hex dumps, terminal logs, our own sniffer output) are not an accepted input; they would have to be converted to pcap/pcapng with a link type TShark can dissect. REA runs TShark with an empty private configuration directory, so per-user Wireshark settings (for example user-defined link-type mappings) are unlikely to apply; custom dissector support is "not addressed". I found no documented K-line link type. **[LOW]** (inference from [packet guide][pkt])
- Signal lane (`decode_signal_recording`): caller-supplied `sigrok-cli 0.7.2` with libsigrokdecode on Linux; worked example `rea decode-signal-recording /artifacts/uart.vcd --decoders uart:rx=RX:baudrate=9600 --input_format vcd --annotations uart=rx-data --format json`. Decoder options use sigrok's colon-separated `key=value` syntax; "REA does not guess wiring, baud rate or protocol."; default input cap 32 MiB (raisable with `producer_limits`); output is annotation text with sample ranges, and "REA does not parse display text into protocol facts." No live acquisition. **[HIGH]** ([signal guide][sig])
- sigrok's decoder list (131 decoders) includes `uart`, `can`, `lin` and SAE J1850 VPW, but nothing for ISO 9141 / K-line, KWP2000 / ISO 14230 or OBD. So the signal lane yields a byte stream at the chosen baud rate; Honda message framing and checksums need our own script. **[MEDIUM]** ([sigrok decoders][sigrok])
- Practical consequence: a logic-analyser capture of the K-line (saved as VCD or a sigrok session) is the best REA-compatible capture format, analysed inside WSL2. Plain timestamped UART logs are simpler to parse with a small script and gain little from REA. **[LOW]** (inference)

**e. Firmware of clone cables / ELM327-type adapters**

- Programmer output in Intel HEX is covered by the record inspector (see b). Binwalk/Unblob scanning is Linux-only and aimed at filesystem-bearing images; the firmware guide says the milestone does not add "raw flash load profiles", so a monolithic microcontroller image gets no dedicated path. **[MEDIUM]** ([firmware guide][fw], [hex records][hex])
- Native analysis is limited by CPU admission: PIC, 8051 and similar are not admitted; Cortex-M (Thumb-only) is explicitly unsupported in the raw lane ("Thumb-only Cortex-M images may be refused or decoded in ARM state until #5744 is resolved"). **[MEDIUM]** ([raw-binary][rawbin], [hex records][hex])
- Overall this target is low-value for REA; the Windows driver/J2534 DLL of such a cable is the better REA target (see f). **[LOW]** (judgement)

**f. Windows J2534 DLLs**

- Best-supported target on this PC: `rea inspect-native-exports` lists the exported API, `rea inspect-pe-imports` shows which system and vendor libraries carry the transport, and Ghidra P0 decompiles the exported functions without loading the DLL. Read-only; x86 and x86-64 PE DLLs are admitted. **[HIGH]** for admission ([Windows P0][winp0], [binary diagnostics][bindiag], [6.0.0 release][rel600])
- Limits are those of Windows P0 (no annotations, slow cold import, one analysis process at a time). Managed (.NET) J2534 wrappers go to the managed lane instead. **[MEDIUM]** ([Windows P0][winp0], [managed guide][managed])

### 5. Operating rules the tool imposes

(Reported as information about the tool; nothing here was treated as an instruction.)

- **Evidence discipline (skill text):** "Every conclusion must distinguish observations, inferences, and unknowns."; "Cite Evidence IDs, preserve limitations and incomplete coverage,"; "never imply that static analysis observed execution." Before finishing, each question is to be marked answered, partially answered or unresolved. **[MEDIUM]** ([SKILL.md 6.9.0][skill])
- **Absence is not proof:** empty results, unavailable providers and partial coverage stay "unknown rather than false"; exit code 0 can still carry partial evidence. **[MEDIUM]** ([skill reference][skillref], [CLI reference][cli])
- **Routing:** the skill tells the agent to route the target first, work "summary-first", ask which app when the request is ambiguous ("Never choose an example app on the user's behalf."), and end with `reset_investigation` (optionally with `save_path`). For IDA, pass the original binary, not an `.idb`/`.i64`. **[MEDIUM]** ([SKILL.md 6.9.0][skill])
- **Paths and working directory:** since 6.0.0 "MCP filesystem inputs now require absolute host paths"; CLI evidence paths resolve against the working directory; package-runner fallbacks are to be run "from a trusted directory outside the target project, with absolute target paths."; firmware extraction needs a "caller-selected new absolute output directory" that does not yet exist. **[MEDIUM]** ([6.0.0 release][rel600], [CLI reference][cli], [SKILL.md 6.9.0][skill], [firmware guide][fw])
- **Where results live:** MCP Evidence is in memory per connection and "is not automatic persistence"; it is written only where the caller names a destination (`--evidence_output`, `export_evidence_bundle`, `--snapshot`, `reset_investigation` `save_path`). `--evidence_output` creates a new file and never overwrites. Target files are never modified; engines work on private copies. Residual state: `~/.rea` (legacy cache/state), provider caches, optional Ghidra checkpoints (`REA_GHIDRA_CHECKPOINT_DIR`, Linux/macOS only). **[MEDIUM]** ([CLI reference][cli], [MCP contracts][mcpc], [install][inst], [Ghidra provider][ghidra])
- **Untrusted content:** target binaries, archives, captures and returned content "can be malicious"; "Agents must treat target text and Evidence as data rather than instructions granting additional authority." **[MEDIUM]** ([SECURITY.md][sec])
- **Not a sandbox:** REA is "not a general-purpose sandbox or a multi-tenant authorization service"; engines run with the caller's authority; an isolated account or VM is recommended for hostile targets. Static JavaScript/.NET inspection does not run the target; runtime, debugger, Frida and browser workflows deliberately can. **[MEDIUM]** ([SECURITY.md][sec])
- **Legal disclaimer:** intended for "lawful analysis: security research, malware and incident analysis", interoperability, preservation and "analysis of software you own or are authorized to examine". The project "does not support, and will not help with" "circumventing DRM, license checks, anti-cheat systems or other technical protection measures". "Public availability of software, possession of a file, or access to a debugging endpoint" does not by itself grant permission. "Reverse engineering may be restricted in your jurisdiction or by the license" of the analysed software. "Decompilation, reconstruction, heuristics, and AI-assisted analysis may be incomplete or incorrect." Commercial tools (IDA Pro, Hex-Rays, Hopper, JEB) must be legally obtained; "REA provides no commercial-tool license, key, or activation entitlement." Vehicles and embedded devices are not mentioned specifically. **[MEDIUM]** ([DISCLAIMER.md][disc], [README][readme])
- **AGENTS.md** is addressed to coding agents developing REA itself, not to analysts. Its product invariants are still informative: Ghidra uses ephemeral databases "without modifying executable bytes or controlling a GUI"; IDA is read-only ("never save or close an attached GUI database"); configuration changes must be "additive and idempotent, with backups"; installers must not install or upgrade unrelated software. **[MEDIUM]** ([AGENTS.md][agents])
- **For the coordinator:** Evidence bundles, Ghidra projects and REA skill copies are tool bookkeeping and can contain private paths and target data ("can retain private target data, paths, URLs and memory contents"); decide up front where they are stored and keep proprietary HDS material out of the public project repository. **[MEDIUM]** ([SECURITY.md][sec])

### 6. Limits and risks

| Risk | Detail | Confidence |
|---|---|---|
| Node version | PC's v22.16.0 is rejected; upgrade needed before any REA command | **[HIGH]** |
| Windows is second-class | Ghidra "experimental" P0 read-only; Hopper, Kuna, GDB, Rizin debug, Binwalk/Unblob, TShark field lane, sigrok lane and process capture are unavailable or unverified natively; WSL2/Linux VM needed | **[HIGH]** ([install][inst], [Windows P0][winp0], [signal guide][sig], [packet guide][pkt], [firmware guide][fw]) |
| No embedded CPU coverage | SuperH and other automotive MCU families not admitted on any provider | **[HIGH]** |
| Churn | 35 releases in four months, major version 6, breaking changes in consecutive minors (Node floor, absolute paths, settings fields); pin an exact version and re-read the changelog before updating | **[MEDIUM]** ([npm][npm], [CHANGELOG][changelog]) |
| Supply chain | Single maintainer; 190 MB tarball with bundled dependencies and a prebuilt native addon; optional Frida binding downloaded by its own install step; `npx -y rea-agents@latest` executes whatever is latest | **[MEDIUM]** |
| Data exposure | Decompiled HDS code and strings returned by tools are sent to the model provider by the agent | **[HIGH]** ([README][readme]) |
| Prompt injection | Analysed binaries and captures are an injection surface; REA marks target content as data but relies on the agent host for enforcement | **[MEDIUM]** ([SECURITY.md][sec], [skills.sh audit][trusthub]) |
| Performance | Cold Ghidra import on Windows observed at 407-601 s for a first function query on small fixtures; default deadline 330 s; default heap 2G | **[MEDIUM]** ([Windows P0][winp0]) |
| Context size | 255 tools advertised by default | **[MEDIUM]** |
| Commercial licences | None required for this project (Ghidra, JADX, TShark, sigrok, Binwalk are free). IDA Pro, Hopper and JEB are optional and add nothing for SuperH inside REA | **[MEDIUM]** ([IDA provider][ida], [DISCLAIMER.md][disc]) |
| Experimental areas | Windows Ghidra P0, firmware "first milestone", Frida lane, hot-cold catalogue, HTTP transport, Kuna | **[MEDIUM]** |
| Legal boundary | Project excludes licence-check / protection-measure circumvention; HDS licensing and immobilizer functions sit close to that line | **[MEDIUM]** ([DISCLAIMER.md][disc]) |

## Conflicts and uncertainties

1. **Packet capture on Windows.** `packet-captures.md` says macOS and Windows "are refused before inspection"; `pcap-analysis.md` says capture inspection works on "Windows with owned Job Objects" but is unverified; the README lists both lanes as Linux. Different tool families, but the practical answer for K-line work is "assume Linux only". ([packet guide][pkt], [pcap guide][pcap], [README][readme])
2. **APK analysis on Windows.** README says Windows x64 is supported; the Android guide says this is new on `main` and older packages can reject the host. Whether 6.9.0 as released includes it must be checked with `rea doctor --provider jadx --json`. ([README][readme], [Android guide][android])
3. **m68k raw lane.** The provider document and the 6.9.0 skill reference list `m68k`; issue #4246 (written 2026-10-15) lists it as pending. The newer documents win, but it shows how fast the admitted list moves. ([raw-binary][rawbin], [issue #4246][i4246])
4. **Popularity figures.** API and web page agree on 459k stars / 113k forks; an article four weeks earlier reported about 200k; a search-engine summary quoted "419 weekly downloads" from Socket while the npm API reports about 1.94M per month. I could not reconcile these and attach no weight to them. ([API][api], [developersdigest][dd], [npm downloads][npmdl])
5. **Docs versus release.** Documentation was read from `main`, two to three days newer than 6.9.0. The raw-binary and Windows statements were cross-checked against the skill reference inside the 6.9.0 npm tarball; other details (tool names, limits) may differ slightly in the installed version.
6. **What setup touches beyond `~/.claude.json` and the skill directory.** The guides mention nothing else, but that is silence, not a guarantee. `--dry-run` is the authoritative answer.
7. **Telemetry.** No statement either way was found; no code audit was done.
8. **Summariser risk.** Long documents were truncated or paraphrased by the fetch tool (it refused some verbatim quotes over 125 characters). Hash values, timeouts and byte limits in this file are indicative only.

## Open questions

| # | Question | How to answer |
|---|---|---|
| 1 | Which microcontroller family is in this car's ECM/PCM (SH-2 class or something else)? Decides whether stock Ghidra suffices and whether any REA lane could ever apply | Other research dimension; read the part marking on the board or identify from a dump |
| 2 | Does REA 6.9.0 under Linux admit an ELF with the SuperH machine type (ELF-wrapped dump)? | Ten-minute bench test in WSL2 after install: `rea inspect <wrapped.elf> --provider ghidra --json` |
| 3 | When will caller-selected language and base address land (issue #4246), and will it cover big-endian SH-2? | Watch issues #4246, #3532 and PR #4678 before each phase that touches firmware |
| 4 | Exactly which files does `setup` plan to change on this PC? | `rea setup --client claude_code --dry-run --json` (read-only) once Node is upgraded and the owner agrees to install |
| 5 | Does the JADX lane start on Windows x64 with 6.9.0? | `rea doctor --provider jadx --json` after placing the JAR |
| 6 | How long does Ghidra P0 take on real HDS binaries (multi-megabyte 32-bit PE), and does it fit the deadlines? | Bench test with raised `REA_GHIDRA_STARTUP_TIMEOUT_MS` and heap |
| 7 | Is drive `D:` a fixed local NTFS/ReFS volume without junctions in the path? | Check volume properties before placing targets there |
| 8 | Does REA's Frida lane work on a Windows host (prebuilt binding for the chosen Node version)? | `rea doctor` plus `list_frida_devices` after install; otherwise use Frida directly |
| 9 | Can K-line byte logs be wrapped into pcapng so that TShark, under REA's private configuration directory, shows useful fields? | Experiment in WSL2; fall back to a custom parser |
| 10 | Does REA make any network call of its own at run time (update checks, telemetry)? | Run once behind a firewall rule or with a network monitor; or search the installed `dist` for network calls |
| 11 | What is HDS made of (native only, .NET, Java, installer format, database formats)? Determines which lanes above are actually used | Other research dimension; then `inspect-native-exports` / `inspect-managed-artifact` on the real files |
| 12 | Are the popularity numbers genuine? | Not answerable from public APIs; irrelevant if the tool is judged on a dry run and a pinned version |

## Sources

- [api]: https://api.github.com/repos/morluto/rea - GitHub API repository record (stars, forks, dates, licence, issue count)
- [web]: https://github.com/morluto/rea - repository web page (cross-check of counts, commits, issues/PR split)
- [npm]: https://registry.npmjs.org/rea-agents - npm registry document (dist-tags, version list, publish times, maintainers)
- [npmlatest]: https://registry.npmjs.org/rea-agents/latest - npm metadata for 6.9.0 (engines, dependencies, dist, provenance)
- [npmdl]: https://api.npmjs.org/downloads/range/last-month/rea-agents - npm daily download counts
- [readme]: https://raw.githubusercontent.com/morluto/rea/main/README.md - README (quick start, target table, FAQ, official-source notice)
- [agents]: https://raw.githubusercontent.com/morluto/rea/main/AGENTS.md - contributor-agent guidelines and product invariants
- [inst]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/installation.md - installation, setup behaviour, Ghidra configuration, update, uninstall
- [client]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/client-configuration.md - per-client configuration files and environment variables
- [cli]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/cli.md - CLI command reference, flags, exit codes, Evidence
- [prompts]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/mcp-prompts.md - MCP prompt names and rules
- [mcpc]: https://raw.githubusercontent.com/morluto/rea/main/docs/reference/mcp-contracts.md - MCP tool contracts, sessions, Evidence fields, tool families
- [rawbin]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/raw-binary.md - headerless raw-binary workflow and admitted architectures
- [winp0]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/windows-ghidra-p0.md - Windows Ghidra "P0" boundary
- [ghidra]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/ghidra-provider.md - Ghidra provider operations and limits
- [ida]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/ida-provider.md - IDA provider modes, admitted CPUs, read-only rules
- [kuna]: https://raw.githubusercontent.com/morluto/rea/main/docs/kuna-provider.md - Kuna provider
- [ppc]: https://raw.githubusercontent.com/morluto/rea/main/docs/ghidra-ppc.md - PowerPC ELF lane (shows how REA gates CPU families)
- [hex]: https://raw.githubusercontent.com/morluto/rea/main/docs/hex-records.md - Intel HEX / S-record inspection and extraction
- [tools]: https://raw.githubusercontent.com/morluto/rea/main/docs/reverse-engineering-tools.md - objdump, Rizin, GDB, Cutter integration
- [fw]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/firmware/firmware-analysis.md - Binwalk / Unblob firmware lane
- [pkt]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/data/packet-captures.md - TShark field-extraction lane
- [pcap]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/data/pcap-analysis.md - TShark/capinfos summary lane
- [sig]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/signals/signal-recordings.md - sigrok-cli signal decoding lane
- [android]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/android/android-analysis.md - JADX-based APK analysis
- [managed]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/managed/managed-code-analysis.md - .NET metadata and CIL analysis
- [bindiag]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/native/binary-diagnostics.md - PE/ELF export, resource, entropy and layout inspectors
- [frida]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/runtime/frida-instrumentation.md - Frida instrumentation lane
- [desk]: https://raw.githubusercontent.com/morluto/rea/main/docs/desktop-applications.md - installed desktop application directory inspection
- [disc]: https://raw.githubusercontent.com/morluto/rea/main/DISCLAIMER.md - legal disclaimer and responsible-use statement
- [sec]: https://raw.githubusercontent.com/morluto/rea/main/SECURITY.md - security policy and trust boundaries
- [changelog]: https://raw.githubusercontent.com/morluto/rea/main/CHANGELOG.md - changelog (6.9.0 and 6.8.0 read)
- [pkg]: https://raw.githubusercontent.com/morluto/rea/main/package.json - package manifest (files, scripts, engines)
- [mcpjson]: https://raw.githubusercontent.com/morluto/rea/main/mcp.json - reference MCP registration
- [serverjson]: https://raw.githubusercontent.com/morluto/rea/main/server.json - MCP registry manifest
- [plugin]: https://raw.githubusercontent.com/morluto/rea/main/.claude-plugin/plugin.json - Claude Code plugin manifest
- [market]: https://raw.githubusercontent.com/morluto/rea/main/.claude-plugin/marketplace.json - Claude Code marketplace manifest
- [reamjs]: https://raw.githubusercontent.com/morluto/rea/main/scripts/rea.mjs - CLI entry script (Node version gate)
- [admit]: https://raw.githubusercontent.com/morluto/rea/main/scripts/lib/runtime-admission.mjs - Node range check logic and message template
- [skill]: https://unpkg.com/rea-agents@6.9.0/skills/reverse-engineer-anything/SKILL.md - the skill shipped in npm 6.9.0
- [skillref]: https://unpkg.com/rea-agents@6.9.0/skills/reverse-engineer-anything/references/native-and-artifacts.md - skill reference on native targets and artifacts (6.9.0)
- [i4246]: https://api.github.com/repos/morluto/rea/issues/4246 - design issue on caller-selected Ghidra language and base address for raw imports
- [issuesearch]: https://api.github.com/search/issues?q=repo:morluto/rea+superh+OR+sh2+OR+sh-2+OR+sh4&per_page=20 - issue search (issues #3532, #2410, PR #4678)
- [telsearch]: https://api.github.com/search/issues?q=repo:morluto/rea+telemetry+in:title,body&per_page=8 - issue search for telemetry (also surfaced PR #6147)
- [rels]: https://api.github.com/repos/morluto/rea/releases?per_page=6 - recent GitHub releases
- [rel600]: https://api.github.com/repos/morluto/rea/releases/tags/rea-agents-6.0.0 - 6.0.0 release notes (Windows x86 PE, absolute paths)
- [owner]: https://api.github.com/users/morluto - owner account record
- [contrib]: https://api.github.com/repos/morluto/rea/contributors?per_page=30 - top contributors
- [trusthub]: https://www.skills.sh/morluto/rea/reverse-engineer-anything/security/agent-trust-hub - third-party automated security audit of the skill
- [dd]: https://www.developersdigest.tech/blog/reverse-engineer-anything-rea-mcp-agents-guide-2026 - independent article (not hands-on)
- [ghsh]: https://raw.githubusercontent.com/NationalSecurityAgency/ghidra/master/Ghidra/Processors/SuperH/data/languages/superh.ldefs - Ghidra SuperH language definitions
- [sigrok]: https://sigrok.org/wiki/Protocol_decoders - libsigrokdecode protocol decoder list
- [ccmcp]: https://code.claude.com/docs/en/mcp - Claude Code MCP scopes (local, project, user)

[api]: https://api.github.com/repos/morluto/rea
[web]: https://github.com/morluto/rea
[npm]: https://registry.npmjs.org/rea-agents
[npmlatest]: https://registry.npmjs.org/rea-agents/latest
[npmdl]: https://api.npmjs.org/downloads/range/last-month/rea-agents
[readme]: https://raw.githubusercontent.com/morluto/rea/main/README.md
[agents]: https://raw.githubusercontent.com/morluto/rea/main/AGENTS.md
[inst]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/installation.md
[client]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/client-configuration.md
[cli]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/cli.md
[prompts]: https://raw.githubusercontent.com/morluto/rea/main/docs/getting-started/mcp-prompts.md
[mcpc]: https://raw.githubusercontent.com/morluto/rea/main/docs/reference/mcp-contracts.md
[rawbin]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/raw-binary.md
[winp0]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/windows-ghidra-p0.md
[ghidra]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/ghidra-provider.md
[ida]: https://raw.githubusercontent.com/morluto/rea/main/docs/providers/ida-provider.md
[kuna]: https://raw.githubusercontent.com/morluto/rea/main/docs/kuna-provider.md
[ppc]: https://raw.githubusercontent.com/morluto/rea/main/docs/ghidra-ppc.md
[hex]: https://raw.githubusercontent.com/morluto/rea/main/docs/hex-records.md
[tools]: https://raw.githubusercontent.com/morluto/rea/main/docs/reverse-engineering-tools.md
[fw]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/firmware/firmware-analysis.md
[pkt]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/data/packet-captures.md
[pcap]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/data/pcap-analysis.md
[sig]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/signals/signal-recordings.md
[android]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/android/android-analysis.md
[managed]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/managed/managed-code-analysis.md
[bindiag]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/native/binary-diagnostics.md
[frida]: https://raw.githubusercontent.com/morluto/rea/main/docs/guides/runtime/frida-instrumentation.md
[desk]: https://raw.githubusercontent.com/morluto/rea/main/docs/desktop-applications.md
[disc]: https://raw.githubusercontent.com/morluto/rea/main/DISCLAIMER.md
[sec]: https://raw.githubusercontent.com/morluto/rea/main/SECURITY.md
[changelog]: https://raw.githubusercontent.com/morluto/rea/main/CHANGELOG.md
[pkg]: https://raw.githubusercontent.com/morluto/rea/main/package.json
[mcpjson]: https://raw.githubusercontent.com/morluto/rea/main/mcp.json
[serverjson]: https://raw.githubusercontent.com/morluto/rea/main/server.json
[plugin]: https://raw.githubusercontent.com/morluto/rea/main/.claude-plugin/plugin.json
[market]: https://raw.githubusercontent.com/morluto/rea/main/.claude-plugin/marketplace.json
[reamjs]: https://raw.githubusercontent.com/morluto/rea/main/scripts/rea.mjs
[admit]: https://raw.githubusercontent.com/morluto/rea/main/scripts/lib/runtime-admission.mjs
[skill]: https://unpkg.com/rea-agents@6.9.0/skills/reverse-engineer-anything/SKILL.md
[skillref]: https://unpkg.com/rea-agents@6.9.0/skills/reverse-engineer-anything/references/native-and-artifacts.md
[i4246]: https://api.github.com/repos/morluto/rea/issues/4246
[issuesearch]: https://api.github.com/search/issues?q=repo:morluto/rea+superh+OR+sh2+OR+sh-2+OR+sh4&per_page=20
[telsearch]: https://api.github.com/search/issues?q=repo:morluto/rea+telemetry+in:title,body&per_page=8
[rels]: https://api.github.com/repos/morluto/rea/releases?per_page=6
[rel600]: https://api.github.com/repos/morluto/rea/releases/tags/rea-agents-6.0.0
[owner]: https://api.github.com/users/morluto
[contrib]: https://api.github.com/repos/morluto/rea/contributors?per_page=30
[trusthub]: https://www.skills.sh/morluto/rea/reverse-engineer-anything/security/agent-trust-hub
[dd]: https://www.developersdigest.tech/blog/reverse-engineer-anything-rea-mcp-agents-guide-2026
[ghsh]: https://raw.githubusercontent.com/NationalSecurityAgency/ghidra/master/Ghidra/Processors/SuperH/data/languages/superh.ldefs
[sigrok]: https://sigrok.org/wiki/Protocol_decoders
[ccmcp]: https://code.claude.com/docs/en/mcp
