# Sentinel Design Rationale

## Purpose

Specifications describe **what** Sentinel is.

The implementation describes **how** Sentinel works.

This document explains **why** Sentinel was designed the way it is.

Engineering decisions are rarely obvious years later. This document exists to preserve the reasoning behind those decisions so future contributors (including future versions of ourselves) understand the intent behind the architecture.

This document is intentionally narrative rather than normative.

---

# Design Philosophy

Sentinel is developed using an architecture-first engineering process.

The preferred order of development is:

```
Vision

↓

Architecture

↓

Specification

↓

Implementation

↓

Testing

↓

Optimization
```

This intentionally delays implementation until the problem has been clearly defined.

The goal is not to avoid refactoring.

The goal is to ensure that refactoring improves implementation rather than changing the underlying design.

---

# Preserve Knowledge

One of the primary goals of Sentinel is to preserve engineering knowledge.

Code alone cannot explain why architectural decisions were made.

Whenever practical, important design reasoning should be captured within the repository rather than existing only in conversations or developer memory.

Sentinel should eventually become self-documenting.

A new engineer should be able to understand the project's evolution by reading the repository alone.

---

# Separate Measurement From Interpretation

One of the most significant architectural decisions made during Sentinel's development was separating electrical measurement from electrical interpretation.

Originally, it was tempting to build a scanner that directly detected touches.

Instead, Sentinel now treats these as separate problems.

The Continuity Scanner performs only one task:

> Measure electrical continuity.

It does not understand:

- foil
- épée
- sabre
- scoring
- timing
- touches

Those concepts belong to higher architectural layers.

This separation makes the scanner reusable, testable, and independent of game rules.

---

# Measure Everything

An early design question asked whether the scanner should measure only the electrical relationships required by the current rule engines.

Although this would reduce the number of measurements, it tightly couples the scanner to specific games.

Instead, Sentinel measures every unique electrical relationship between the seven logical lines.

Advantages include:

- scanner independence
- simpler diagnostics
- easier replay
- easier testing
- future game support
- future rule changes

The small increase in scan time is considered insignificant on modern processors.

---

# The Scanner Is a Measurement Instrument

The scanner behaves like a voltmeter or oscilloscope.

It reports what it measures.

It does not attempt to correct inconsistent measurements.

It performs no inference.

If an impossible electrical condition is observed, that condition is reported exactly as measured.

Higher architectural layers may choose to interpret such conditions as faults.

The scanner shall not.

---

# Use Domain Terminology

Whenever practical, Sentinel adopts terminology already used within the sport of fencing.

Examples include:

- Line
- A-Line
- B-Line
- C-Line

rather than inventing new software terminology.

This makes the documentation easier for armorers, coaches, officials, and experienced fencers to understand.

Processor-specific terminology such as GPIO appears only within hardware abstraction layers.

---

# Documentation Is Part of the Design

Documentation is treated as part of the engineering process rather than an afterthought.

Specifications define contracts.

Rationale explains decisions.

Source code implements specifications.

The repository should remain understandable even if individual developers are no longer available.

---

# Git Commits Represent Ideas

Sentinel favors small commits that each represent one complete engineering idea.

Examples include:

- Define Sentinel electrical continuity model
- Specify Sentinel continuity scanner
- Define scanner line and continuity map interfaces

This approach makes the Git history an engineering narrative rather than simply a record of file changes.

---

# Evolve Through Refactoring

The project intentionally embraces refactoring.

Initial implementations are expected to improve over time.

Architectural decisions should be relatively stable.

Implementations are expected to evolve.

The preferred approach is:

```
Make it correct.

↓

Make it understandable.

↓

Make it fast.
```

rather than attempting to optimize prematurely.

---

# Design For Portability

Sentinel is intended to outlive any single processor family.

The architecture therefore separates:

- electrical concepts
- hardware interfaces
- processor implementations
- game logic

The long-term goal is for the same core library to support multiple processor families without modification.

---

# Long-Term Goal

The ultimate objective is not simply to build a fencing scoring machine.

The goal is to build an engineering platform that is:

- understandable
- maintainable
- portable
- deterministic
- well documented
- enjoyable to work on

Every significant design decision should support those goals.
