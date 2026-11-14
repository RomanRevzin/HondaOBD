# HondaOBD

A DIY device that plugs into the 16-pin OBD2 connector of a **2002 Honda Civic Si (EP3, K20A3)** and
reads/writes as many diagnostic codes and service functions as the car physically allows — engine,
ABS, airbag (SRS), immobilizer and electric power steering — up to and including ECM reflash. A
right-to-repair / owner-diagnostics / enthusiast-tuning project.

> **Status:** research complete, build plan drafted. Nothing built yet.

## Why this is a real project and not a dongle purchase

The 2001–2005 Civic is **pre-CAN**: all diagnostics run over Honda's single-wire **K-line** family,
not CAN bus. Off-the-shelf OBD dongles only reach the legislated emissions data on it. Worse, the
airbag and immobilizer units sit on a *second*, ~5 V serial wire Honda calls **DIAG-H** (connector
pin 14) that ordinary K-line adapters never touch. Covering "every code" means driving both lines,
plus Honda's SCS service line — which no existing open-hardware board does.

## Start here

- **[docs/PLAN.md](docs/PLAN.md)** — the build plan: scope, feasibility, hardware/software
  architecture, reverse-engineering strategy, phased roadmap, risks, and open decisions.
- **[docs/research/](docs/research/)** — nine detailed research notes (the car & its control units,
  connector pinout, Honda protocols, SRS/ABS/EPS/immobilizer, ECM reflash, interface hardware,
  firmware/software stack, dealer tools, and the REA reverse-engineering toolkit), each with sources
  and confidence tags.

## Hardware direction (draft)

Raspberry Pi Pico 2 W (RP2350, PIO for exact bit-timing) + an ST L9637D K-line transceiver, a separate
configurable DIAG-H channel, a switched SCS line, vehicle-grade power protection, and a passive
sniffer mode for reverse-engineering a dealer tool on the car.

## License

**GPL-3.0** (chosen — see §0 of [docs/PLAN.md](docs/PLAN.md)), so the best K-series/Honda protocol
references (HondaECU, freediag) can be reused directly. The `LICENSE` file is added when the first
code lands.
