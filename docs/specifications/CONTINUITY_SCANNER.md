# Continuity Scanner

## Status

**Draft**

This specification defines the Continuity Scanner subsystem.

---

# Purpose

The Continuity Scanner is responsible for measuring electrical continuity between Sentinel's seven logical lines.

Its sole responsibility is to produce one complete electrical snapshot.

The scanner performs no electrical interpretation and contains no knowledge of fencing rules.

---

# Responsibilities

The Continuity Scanner shall:

- Measure continuity between all unique line pairs.
- Produce one complete electrical snapshot.
- Operate independently of game rules.
- Operate independently of processor architecture.
- Operate independently of board layout.

The scanner shall not:

- Interpret electrical continuity.
- Apply timing rules.
- Detect touches.
- Perform scoring.
- Detect electrical faults.
- Maintain historical state.

---

# Inputs

The scanner operates on the seven logical lines defined by the Electrical Model.

```
RA
RB
RC
MT
GC
GB
GA
```

The scanner does not know how these lines are mapped to processor GPIOs.

---

# Output

Each scan produces one complete electrical snapshot.

The electrical representation of that snapshot is defined by the Electrical Model specification.

---

# Scan Sequence

Each scan consists of six measurement phases.

```
Drive RA

Measure:
RB
RC
MT
GC
GB
GA
```

```
Drive RB

Measure:
RC
MT
GC
GB
GA
```

```
Drive RC

Measure:
MT
GC
GB
GA
```

```
Drive MT

Measure:
GC
GB
GA
```

```
Drive GC

Measure:
GB
GA
```

```
Drive GB

Measure:
GA
```

GA is never driven because all unique line pairs have already been measured.

---

# Scan Invariants

During every measurement cycle:

- Exactly one line shall be actively driven.
- All remaining lines shall be observed.
- Every unique line pair shall be measured exactly once.
- Every scan shall be independent of all previous scans.

---

# Measurement Philosophy

The scanner is a measurement instrument.

It reports observed electrical continuity.

It performs no inference.

For example,

```
RA-GC = 1

GC-MT = 1

RA-MT = 0
```

shall be reported exactly as measured.

The scanner shall not infer or modify measured continuity.

---

# Hardware Independence

The scanner operates entirely on logical lines.

Processor GPIO numbering is outside the scope of this specification.

Board-specific mappings are defined by the Hardware Abstraction Layer.

---

# Relationship to Higher Layers

The scanner produces electrical measurements.

Higher architectural layers interpret those measurements.

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
Rule Engine
```

---

# Relationship to Lower Layers

The scanner depends upon a Hardware Abstraction Layer (HAL).

The HAL is responsible for:

- GPIO configuration
- GPIO direction
- GPIO sampling
- Processor-specific register access

The scanner remains completely processor independent.

---

# Determinism

Given identical electrical conditions, the scanner shall always produce identical electrical snapshots.

The scanner contains no internal state.

---

# Future Compatibility

This specification intentionally does not define:

- GPIO assignments
- Scan timing
- Processor implementation
- Hardware registers

Those details may change without affecting higher architectural layers.

---

# Summary

The Continuity Scanner transforms the physical electrical state of the fencing apparatus into the canonical electrical representation defined by the Electrical Model.

It performs measurement only.

Interpretation belongs to higher layers.
