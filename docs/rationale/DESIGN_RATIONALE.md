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

The computational cost of processing the additional measurements is expected to be small on modern processors. The total scan time, however, may be constrained by the electrical settling time required between measurement phases. This must be characterized on real fencing hardware before the performance cost of a complete scan can be considered insignificant.

---

# Preserve the Full Scan Until Measurement Proves Otherwise

The decision to measure every unique line pair was revisited when the behavior of the original scoring apparatus was examined.

The proven ESP32 implementation used an active-low continuity measurement technique:

```cpp
void setPIN(short pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(1);
}

void unsetPIN(short pin) {
    pinMode(pin, INPUT_PULLUP);
    digitalWrite(pin, HIGH);
}
```

Under normal conditions, the lines were configured as inputs with internal pull-ups. During a measurement, one selected line was configured as an output and driven LOW. Electrical continuity caused connected lines to be pulled LOW as well.

The 1 millisecond delay in `setPIN()` was not derived from ESP32 GPIO switching specifications. It was retained because it produced reliable operation during development.

Earlier versions appeared functional with less conservative timing until realistic fencing cabling was introduced. Long floor cables and, particularly, fencing reels with their associated connecting cables exposed reliability problems that were not apparent with shorter wiring.

This experience is important because the relevant settling behavior belongs to the complete electrical system, not merely the microcontroller GPIO.

Potential contributors include:

- cable capacitance
- cable length
- reel wiring
- connector resistance
- contact resistance
- internal pull-up characteristics
- external electrical loading

Consequently, the nanosecond-scale switching speed of an ESP32 GPIO does not by itself establish a safe measurement interval.

A complete seven-line scan requires six driven-line measurement phases. If every phase requires a 1 millisecond settling interval, settling time alone requires approximately 6 milliseconds per complete snapshot.

This means the performance concern that originally discouraged a complete scan was legitimate.

However, the minimum reliable settling time was never systematically measured.

Sentinel will therefore retain the complete 21-pair scan unless empirical testing on realistic fencing hardware demonstrates that it cannot satisfy the required timing constraints.

The planned approach is to reproduce the known-good measurement behavior first, make the settling interval configurable, and then characterize reliability at progressively shorter intervals using realistic cables, reels, weapons, and connectors.

This reflects a broader engineering principle used by Sentinel:

> **Measure first. Optimize from evidence.**

Processor-level optimization, direct GPIO register access, or changes to the scanner architecture should not be used to compensate for an assumed bottleneck before the electrical behavior of the real system has been measured.

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
