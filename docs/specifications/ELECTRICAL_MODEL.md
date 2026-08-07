# Electrical Model

## Status

**Draft**

This specification defines Sentinel's electrical domain model.

---

# Purpose

Sentinel observes the physical fencing apparatus by measuring electrical continuity between a fixed set of logical lines.

This document defines the electrical concepts used throughout Sentinel. It establishes the vocabulary, data model, and invariants upon which all higher-level software components are built.

This specification intentionally does **not** define:

- Game rules
- Timing requirements
- Scoring behavior
- Hardware implementation
- User interface behavior

Those topics are defined elsewhere.

---

# Design Principles

The electrical model shall satisfy the following principles.

1. **Hardware Independence**

   The electrical model shall be independent of any processor, microcontroller, GPIO assignment, or PCB layout.

2. **Game Independence**

   The electrical model shall not contain knowledge of foil, épée, sabre, or any other game mode.

3. **Measurement Without Interpretation**

   The electrical subsystem shall report measured electrical continuity only.

   It shall perform no inference or interpretation.

4. **Deterministic Representation**

   Identical electrical conditions shall always produce identical electrical representations.

---

# Terminology

The following terms are used throughout Sentinel.

| Term                 | Definition                                                         |
| -------------------- | ------------------------------------------------------------------ |
| **Line**             | One of Sentinel's seven logical electrical circuits.               |
| **Continuity**       | A measured conductive path between two lines.                      |
| **Connection**       | A unique unordered pair of lines whose continuity may be measured. |
| **Snapshot**         | One complete measurement of all unique line-to-line continuities.  |
| **Electrical Model** | The canonical representation of one electrical snapshot.           |

---

# Logical Lines

Sentinel models the fencing apparatus using seven logical lines.

| Index | Name | Description    |
| ----: | ---- | -------------- |
|     0 | RA   | Red A Line     |
|     1 | RB   | Red B Line     |
|     2 | RC   | Red C Line     |
|     3 | MT   | Metallic Strip |
|     4 | GC   | Green C Line   |
|     5 | GB   | Green B Line   |
|     6 | GA   | Green A Line   |

These names describe logical electrical circuits.

They are **not** processor GPIO pins.

They are **not** connector pins.

They are **not** physical wires.

Board-specific hardware maps these logical lines onto processor GPIOs.

---

# Electrical Continuity

Electrical continuity is defined as the presence of a conductive path between two logical lines at the instant of measurement.

Continuity is represented as a Boolean value.

| Value | Meaning                  |
| ----: | ------------------------ |
|     0 | No measurable continuity |
|     1 | Continuity detected      |

The scanner reports measured continuity only.

No attempt is made to determine why continuity exists.

---

# Unique Connections

A system containing **N** lines contains

```
N(N-1)/2
```

unique unordered line pairs.

Sentinel defines seven logical lines.

Therefore,

```
7 × 6 / 2 = 21
```

unique electrical connections exist.

Each connection shall be represented exactly once.

---

# Canonical Connection Ordering

The ordering below defines Sentinel's canonical electrical representation.

This ordering is considered part of Sentinel's public interface.

|  Bit | Connection |
| ---: | ---------- |
|    0 | RA-RB      |
|    1 | RA-RC      |
|    2 | RA-MT      |
|    3 | RA-GC      |
|    4 | RA-GB      |
|    5 | RA-GA      |
|    6 | RB-RC      |
|    7 | RB-MT      |
|    8 | RB-GC      |
|    9 | RB-GB      |
|   10 | RB-GA      |
|   11 | RC-MT      |
|   12 | RC-GC      |
|   13 | RC-GB      |
|   14 | RC-GA      |
|   15 | MT-GC      |
|   16 | MT-GB      |
|   17 | MT-GA      |
|   18 | GC-GB      |
|   19 | GC-GA      |
|   20 | GB-GA      |

Future implementations shall preserve this ordering.

---

# Symmetry

Electrical continuity is symmetric.

The following identities always hold.

```
RA-GC == GC-RA

RB-MT == MT-RB

GA-RC == RC-GA
```

Sentinel therefore stores each unique connection exactly once.

---

# Electrical Snapshot

An electrical snapshot represents the complete electrical state of the fencing apparatus at one instant in time.

A snapshot consists of twenty-one Boolean continuity measurements.

Every snapshot is independent.

No historical information is retained by the electrical model.

---

# Measurement Philosophy

The electrical subsystem behaves as a measurement instrument.

Its responsibility is limited to observing electrical continuity.

It shall **not**:

- infer missing connections
- validate electrical topology
- interpret game rules
- detect faults
- apply timing rules
- perform debounce

Those responsibilities belong to higher architectural layers.

---

# Architectural Context

The electrical model occupies the following position within Sentinel.

```
Physical Apparatus
        │
        ▼
Continuity Scanner
        │
        ▼
Electrical Model
        │
        ▼
Electrical Interpreter
        │
        ▼
Game Rule Engine
        │
        ▼
Presentation
```

---

# Compatibility

The canonical connection ordering defined by this specification shall remain stable.

Maintaining this ordering preserves compatibility with:

- Rule engines
- Diagnostic tools
- Replay files
- Unit tests
- Future hardware implementations

---

# Summary

The electrical model defines **what Sentinel measures**.

It intentionally avoids defining **what those measurements mean**.

Electrical interpretation belongs to higher architectural layers.

---

# Related Specifications

- Vision
- SYSTEM_MODEL
- DOMAIN_MODEL
- Architecture
