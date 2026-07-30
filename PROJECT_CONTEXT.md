---
title: Project Context
version: 1.0
status: Living Document
last_updated: 2026-07-30
authors:
  - Phillip Toone
  - ChatGPT
---

# Project Context

## Purpose

This document provides the context necessary for a new contributor to become productive on the Sentinel project.

Contributors may include:

- Software engineers
- Embedded systems developers
- Fencing experts
- Documentation contributors
- Future maintainers
- AI assistants
- Future versions of the original authors

This document should be read before making architectural recommendations or implementing new functionality.

---

# What Is Sentinel?

Sentinel is a modern, open-source, modular fencing scoring platform centered around a reusable, deterministic scoring engine.

Unlike many fencing scoring projects, Sentinel is **not** organized around a particular hardware platform or user interface.

Instead, Sentinel separates the rules of fencing from the technology used to present them.

The scoring engine understands fencing.

Everything else is an adapter.

This allows the same engine to power:

- ESP32 firmware
- Desktop simulators
- Android applications
- Web interfaces
- Future hardware platforms

without changing the implementation of the fencing rules.

---

# Project Philosophy

Sentinel follows several core principles.

- Architecture before implementation.
- Documentation before code.
- One source of truth.
- Testability by design.
- Hardware abstraction.
- Deterministic behavior.
- Long-term maintainability.

The project intentionally favors thoughtful engineering over rapid implementation.

---

# Current Status

Sentinel is currently in the architectural design phase.

No production firmware has been written.

The project is focused on understanding the fencing domain before implementing software.

---

# Current Development Phase

**Phase 0 — Foundation**

## Completed

- Repository established
- Project vision defined
- Engineering standards established
- Architecture Decision Record (ADR) process established
- Knowledge transfer process established

## Current Milestone

Develop the **Domain Model**.

The Domain Model will define the concepts of fencing that Sentinel must represent before software architecture is designed.

---

# Repository Organization

```
docs/
    decisions/
    journal/
    research/
    specifications/
    standards/

firmware/
    app/
    drivers/
    engine/
    platform/

simulator/

tests/

tools/
```

The repository is organized by responsibility rather than implementation technology whenever practical.

---

# Repository Conventions

Sentinel intentionally follows several engineering conventions.

- Significant architectural decisions are documented using ADRs.
- Specifications describe what the system should do before implementation begins.
- Standards define project-wide conventions and practices.
- Research documents summarize existing systems and relevant technologies.
- Journal entries record the project's evolution.
- Markdown is used for project documentation.
- YAML front matter provides document metadata.
- The repository—not conversations—is the authoritative source of project knowledge.

---

# Important Documents

New contributors are encouraged to become familiar with the following documents.

| Document           | Purpose                        |
| ------------------ | ------------------------------ |
| README.md          | High-level project overview    |
| PROJECT_CONTEXT.md | Project orientation            |
| Vision.md          | Why Sentinel exists            |
| ROADMAP.md         | Long-term project direction    |
| STATUS.md          | Current project status         |
| STYLE_GUIDE.md     | Project conventions            |
| ADR-0001           | Architecture-first development |

---

# Current Architectural Direction

Although the detailed architecture has not yet been specified, several foundational decisions have already been made.

The scoring engine:

- owns all bout state
- is platform independent
- is deterministic
- is extensively testable
- does not depend on hardware

Displays, communication protocols, hardware interfaces, and user applications are clients of the scoring engine.

---

# Current Priorities

The next major deliverables are expected to be:

1. Domain Model
2. Requirements Specification
3. Architecture Specification
4. Engine Specification

Production implementation should not begin until these documents provide sufficient architectural guidance.

---

# Decision Making

Major architectural decisions should be documented using Architecture Decision Records (ADRs).

Each ADR should explain:

- The problem
- The decision
- Alternatives considered
- Consequences

Architecture should evolve intentionally rather than accidentally.

---

# Working With Sentinel

When contributing to Sentinel:

- Prefer improving architecture over adding features.
- Prefer improving documentation over relying on memory.
- Prefer simple designs over clever designs.
- Challenge assumptions respectfully.
- Document important reasoning.
- Keep responsibilities clearly separated.

If implementation and documentation disagree, investigate the discrepancy rather than assuming either is correct.

---

# Knowledge Preservation

One of Sentinel's primary goals is to preserve engineering knowledge.

Important project knowledge should exist within the repository rather than within conversations or individual contributors' memories.

If an important design decision exists only in discussion, the documentation is incomplete.

---

# First-Day Reading

A new contributor should read the following documents in order:

1. README.md
2. PROJECT_CONTEXT.md
3. Vision.md
4. ROADMAP.md
5. STATUS.md
6. ADR-0001

After becoming familiar with these documents, contributors should continue with the current project specifications.

---

# Looking Ahead

The long-term objective of Sentinel is to produce a reusable scoring engine capable of powering multiple applications and hardware platforms while preserving a single, deterministic implementation of the rules of fencing.

Sentinel is intended to demonstrate that embedded software can be:

- Modular
- Understandable
- Well documented
- Testable
- Maintainable

The scoring engine is the product.

Everything else is an adapter.
