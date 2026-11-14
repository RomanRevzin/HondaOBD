# host/

Host application (PC side), in **C#/.NET**. Talks to the device over USB CDC / BLE / Wi-Fi and holds
the protocol logic, DTC/live-data decoding, service functions, and the UI. Nothing here yet.
Architecture and rationale: [`../docs/PLAN.md`](../docs/PLAN.md) §6.

## Planned shape

- **channel interface, two back-ends** — our device, or a commercial J2534 interface (via `J2534-Sharp`,
  MIT) so sessions can be compared against known-good tools.
- **Honda service layer** — DTC / live-data / snapshot / function-test logic, built from the protocol
  tables captured and reverse-engineered during the RE phase.
- **ELM327-compatible personality** (optional, later) — so existing apps (Torque, OBD Fusion) can use
  the device for the generic-OBD part.

## Data

- Generic DTC text: freely available (MIT, `mytrile/obd-trouble-codes`).
- Honda-specific DTC / parameter tables: built from the factory manual + captures; not in any open DB.
