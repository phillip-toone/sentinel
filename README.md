# Sentinel

> A modern, open-source, modular fencing scoring platform centered around a reusable, deterministic scoring engine.

> ⚠️ **Project Status:** Sentinel is currently in the architectural design phase. No production firmware has been implemented.

## Vision

Sentinel is an architecture-first engineering project whose goal is to build a modern fencing scoring platform that is:

- Reliable
- Modular
- Testable
- Portable
- Well documented

The core of Sentinel is a platform-independent scoring engine that implements the rules of fencing independently of any specific hardware, operating system, or user interface.

The same engine should be capable of running on embedded devices, desktop simulators, and future platforms without modification.

---

## Project Philosophy

The project follows several guiding principles:

- Architecture before implementation.
- Documentation before code.
- Testability by design.
- Hardware abstraction.
- One source of truth.
- Small, understandable components.
- Decisions should be documented.

---

## Goals

### Core Engine

- Platform-independent C++ scoring engine
- Deterministic event-driven architecture
- Comprehensive automated testing
- Support for foil, épée, and sabre

### Firmware

- ESP32-S3 reference implementation
- Standalone operation
- Built-in display
- Bluetooth Low Energy
- Wi-Fi connectivity
- OTA firmware updates

### Clients

- Built-in display
- Android application
- Web interface
- Desktop simulator

---

## Project Status

🚧 Planning and architecture.

No production code has been written.

The project is currently focused on research, requirements, and system architecture.

---

## Repository Structure

```
docs/
firmware/
simulator/
tests/
```

---

## License

MIT License
