---
title: System Model
version: 1.1
status: Draft
last_updated: 2026-10-07
authors:
  - Phillip Toone
  - ChatGPT
---

# System Model

## Purpose

This document describes the conceptual flow of information and decisions through Sentinel. Rather than specifying software classes, processor assignments, or hardware interfaces, it explains how electrical measurements and other inputs contribute to fencing-apparatus behavior and bout state.

The detailed contracts are defined by the corresponding Sentinel specifications. In particular, the Electrical Model defines `ContinuityMap`, the Continuity Scanner produces complete maps, and the Continuity Observation specification associates completed maps with monotonic time.

---

# Overview

Sentinel transforms observations of the physical fencing apparatus into weapon-specific interpretations, scoring-apparatus behavior, and presentation of the bout state.

```text
Physical Fencing Apparatus
          |
          v
Observation Layer
  Continuity Scanner -> ContinuityMap
                     -> ContinuityObservation
          |
          v
Interpretation Layer
          |
          v
Scoring Layer
          |
          v
Presentation Layer
```

Other inputs, including referee commands and bout-clock controls, enter through their own appropriate interfaces; they are not electrical continuity observations.

These layers describe responsibilities, not necessarily distinct classes, tasks, processors, or devices. A standalone Sentinel may perform all of them on one ESP32-S3; other implementations may distribute them without changing their conceptual contracts.

---

# Physical World

The physical world includes the fencing equipment and interactions that Sentinel measures but does not control. Examples include:

- Depression and release of a foil or épée pointe d'arrêt.
- Contact between blades, guards, and other weapon components.
- Contact with an opponent's conductive jacket or other valid conductive surface.
- Contact with the conductive piste and weapon earth circuits.
- Changes in electrical continuity caused by equipment faults or movement.

Referee actions and the passage of time also affect a bout, but they are not themselves continuity relationships. Their input and timing interfaces are distinct from the electrical observation stream.

---

# Observation Layer

The Observation Layer represents measured electrical facts without assigning fencing meaning.

The Continuity Scanner produces one complete `ContinuityMap` describing the canonical twenty-one continuity relationships among Sentinel's seven logical lines:

```text
RA  RB  RC  CP  GC  GB  GA
```

`CP` designates the conductive piste. The map represents a complete scanner result, not necessarily twenty-one measurements acquired simultaneously.

A `ContinuityObservation` associates that completed map with the monotonic time at which it became available:

```cpp
struct ContinuityObservation
{
    ContinuityMap map;
    Timestamp timestamp;
};
```

This is a conceptual representation, not a prescribed C++ interface or timestamp type.

For the current architecture, each completed scanner cycle conceptually produces one observation, including when the map is unchanged. The observation layer does not debounce, suppress duplicate maps, infer missing states, identify hits, or apply fencing timing rules.

A sequence of these observations forms an Observation Stream. Its timing guarantees and unresolved limits are defined in `CONTINUITY_OBSERVATION.md`.

---

# Interpretation Layer

The Interpretation Layer gives weapon-specific electrical meaning to the observed topology and its history. The same continuity relationship may mean different things in foil, épée, and sabre.

For example, the interpreter may recognize:

- Opening of a foil's normally closed pointe d'arrêt circuit.
- Closure of an épée's normally open pointe d'arrêt circuit.
- Contact between a sabre and an opponent's valid conductive surface.
- Contact involving a weapon earth circuit or the conductive piste.
- Relationships relevant to equipment faults or weapon-to-weapon contact.

Interpretation may require temporal history to evaluate the persistence, sequence, and overlap of measured conditions. A single map does not by itself establish that an electrical condition has satisfied a specified duration.

The exact boundary between electrical interpretation, contact qualification, scoring-apparatus blocking behavior, and game rules remains to be specified. This document does not assign those timers or decisions to a particular class or execution task.

An interpreted electrical condition is not automatically an awarded touch.

---

# Scoring Layer

The Scoring Layer applies the relevant fencing and bout rules and owns the authoritative bout state. Its responsibilities may include:

- Score and awarded touches.
- Bout clock and periods.
- Cards, priority, and other bout status.
- Referee commands and their effects.
- State transitions resulting from interpreted fencing events.

The apparatus's registration and signaling of a hit must be distinguished from a referee's decision to award a touch. The detailed relationship between registered hits, scoring-apparatus behavior, and awarded touches will be defined in later specifications.

The Scoring Layer should not depend on GPIO numbers, scan phases, or the particular processor used to acquire continuity measurements.

---

# Presentation Layer

The Presentation Layer communicates apparatus indications and bout state without becoming authoritative for that state.

Possible presentation mechanisms include:

- Built-in display.
- Visual signals, including lamps or LEDs.
- Audible signals.
- Connected applications and remote displays.
- Diagnostic or communication interfaces.

`BZ` remains the identifier for a physical buzzer output in implementations that use one; an audible signal is the functional apparatus indication and need not always be produced by that particular hardware.

Presentation components may maintain temporary display copies but do not independently award touches or change authoritative bout state.

---

# Other Inputs and Time

Not all inputs to Sentinel originate in the continuity scanner. Referee commands, user-interface actions, and bout-clock controls must be modeled through appropriate interfaces rather than fabricated as `ContinuityObservation` values.

The monotonic timestamps associated with continuity observations support elapsed-time reasoning about electrical conditions. The bout clock is a distinct domain concept; its operation and control are not defined by the observation timestamp alone.

This model does not require periodic synthetic events such as `ClockAdvanced(1 ms)` in the electrical observation stream.

---

# One Source of Truth

The Scoring Layer is the authoritative owner of bout state. Other components may derive or display information from that state, but they must not independently become authoritative for score, awarded touches, or bout status.

The electrical observation stream remains the source of measured topology and time association; it does not itself contain authoritative bout decisions.

---

# Design Principles

Each layer should know only what is necessary for its responsibility:

- The Observation Layer measures and timestamps topology without fencing interpretation.
- The Interpretation Layer understands weapon-specific electrical meaning without owning the bout score.
- The Scoring Layer applies bout rules without depending on physical GPIO assignments.
- The Presentation Layer presents indications and state without making scoring decisions.

These are conceptual boundaries. Their eventual execution arrangement may be a single loop, multiple tasks, multiple cores, or multiple devices, provided the relevant contracts are preserved.

---

# Example: Épée Electrical Contact

The following illustrates the distinction between measurement, interpretation, apparatus behavior, and awarded touches. It is conceptual, not a complete épée scoring algorithm.

```text
Physical action:
    Red épée pointe d'arrêt is depressed
                  |
                  v
Continuity Scanner:
    RA-RB is measured closed in a complete ContinuityMap
                  |
                  v
ContinuityObservation:
    completed map + monotonic completion timestamp
                  |
                  v
Interpretation:
    red épée tip circuit closure recognized over time
    (including relevant electrical exclusions and timing)
                  |
                  v
Scoring-apparatus behavior:
    a hit may qualify for registration under the applicable rules
                  |
                  v
Presentation:
    required visual and audible signals are produced
                  |
                  v
Bout decision:
    referee/game rules determine whether a touch is awarded
    and the authoritative bout state is updated accordingly
```

The ordering above illustrates conceptual responsibilities; it does not require presentation to finish before a bout decision, or prescribe when any particular internal timer begins. The exact timing and decision semantics belong to later weapon-specific and scoring-apparatus specifications.

A registered hit and an awarded touch are distinct concepts. For example, apparatus registration may provide information to the referee without independently determining the final score.

---

# Scope and Open Design Questions

This system model does not yet decide:

- The concrete Electrical Interpreter interface or output types.
- Ownership and implementation of contact qualification and blocking timers.
- The required maximum interval between continuity observations.
- Timestamp resolution, representation, or rollover behavior.
- Scheduling, FreeRTOS task structure, processor-core assignment, or queues.
- Transport protocols for optional external consumers.
- Detailed weapon-specific scoring and bout-state transitions.

These questions must be resolved by the appropriate requirements, specifications, and experimental evidence rather than assumed by this overview.

---

# Related Specifications

- `ELECTRICAL_MODEL.md` — canonical logical lines and `ContinuityMap`.
- `CONTINUITY_SCANNER.md` — complete electrical snapshot production.
- `CONTINUITY_OBSERVATION.md` — temporal association and observation-stream contract.
- `../GLOSSARY.md` — preferred Sentinel and FIE terminology.

The August 2026 FIE Material Rules and Technical Rules are retained as external reference documents under `docs/reference/fie/`. They are sources for requirements, not replacements for Sentinel's own architectural contracts.

---

# Future Work

Subsequent work will define weapon-specific electrical interpretation, authoritative scoring-apparatus timing requirements, bout/game state, and presentation behavior in greater detail.

This document establishes the conceptual model and preserves the boundaries between those responsibilities.
