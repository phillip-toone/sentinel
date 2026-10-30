# Electrical Interpreter

**Status:** Draft 0.1 — architectural contract under development
**Scope:** Sentinel electrical interpretation boundary
**Implementation status:** Not implemented or validated by this document

## 1. Purpose

The **Electrical Interpreter** translates measured electrical topology into weapon-specific electrical facts for use by the Game Engine. It does not decide whether a hit registers, whether a touch is awarded, or how a bout proceeds.

The initial architecture is:

```text
Physical fencing apparatus
          |
          v
Continuity Scanner
          |
          v
ContinuityObservation (ContinuityMap + timestamp)
          |
          v
Electrical Interpreter (weapon-specific interpretation)
          |
          v
ElectricalObservation (complete interpreted state + timestamp)
          |
          v
Game Engine (temporal qualification and scoring rules)
```

The scanner remains a useful independent capability for equipment diagnostics, troubleshooting, and other non-scoring functions even if future scoring acquisition uses hardware latches or watchers.

## 2. Terminology and dependencies

This document uses the terminology defined in `docs/GLOSSARY.md` and the logical electrical relationships defined in `docs/specifications/ELECTRICAL_MODEL.md`. The scanner and its acquisition contract are described in `docs/specifications/CONTINUITY_SCANNER.md` and `docs/specifications/CONTINUITY_OBSERVATION.md`.

Sentinel's seven logical lines are ordered:

```text
RA RB RC CP GC GB GA
```

**RedEarth** means the Red C circuit (`RC`); **GreenEarth** means the Green C circuit (`GC`). In foil and épée, C is associated with the metallic weapon structure, including blade and guard. **Earth does not mean the conductive jacket (A), conductive piste (`CP`), building ground, or protective earth.** These circuits may be electrically connected in some circumstances; they are not assumed to be identical nodes.

An observed electrical connection is not proof of direct physical contact between the named objects: indirect conductive paths can produce the same topology.

## 3. Responsibilities

The Electrical Interpreter shall:

- Accept a `ContinuityObservation` and the currently selected weapon configuration.
- Interpret the 21 canonical continuity relationships into weapon-specific electrical conditions.
- Produce a **complete, self-contained** `ElectricalObservation` on each publication, rather than a partial update or delta.
- Preserve the timestamp from the source continuity observation; never substitute publication time for measurement time.
- Publish an initial interpreted state, every detected change in scoring-relevant interpreted state, and a full-state heartbeat when the configured maximum reporting interval is reached without such a change.
- Preserve the order of scoring-relevant state changes and expose any known loss of observation continuity to the consumer. The precise loss-reporting mechanism is TBD.

The Electrical Interpreter shall **not**:

- Award touches or register hits.
- Apply weapon-specific contact qualification durations, blocking intervals, anti-whip timing, or bout rules.
- Control GPIOs, choose pull resistors, or set scanner settling delays.
- Assert that a detected electrical state was continuously present between successive physical samples.

The **Game Engine** owns temporal scoring interpretation, including contact qualification, hit registration, blocking, and applicable bout rules. Internal subdivision of those responsibilities within the Game Engine remains open.

## 4. Input and configuration

The initial input is a complete `ContinuityObservation` consisting of a `ContinuityMap` and its monotonic observation timestamp, as defined by the existing observation specification.

Weapon selection and the heartbeat interval are configuration inputs. A system controller may own the configuration lifecycle; the Game Engine may express requirements, but it need not directly configure the scanner or hardware.

Illustrative C++ only (not a finalized ABI):

```cpp
enum class Weapon
{
    Foil,
    Epee,
    Sabre
};

struct ElectricalInterpreterConfig
{
    Weapon weapon;
    Duration heartbeatInterval;
};
```

A single selected weapon for both fencers is the default bout model. Diagnostic or training modes requiring different configurations on each side are outside this draft's defined scope.

**The heartbeat interval is a publication parameter, not a scan interval, sampling period, or contact qualification duration.** The chosen value, bounds, configuration-change behavior, and time-unit types are TBD.

## 5. Output contract

Every published `ElectricalObservation` contains:

1. The original observation timestamp, with the same meaning and time domain as its source `ContinuityObservation`.
2. A complete interpreted electrical state appropriate to the selected weapon.
3. An `ObservationReason` identifying initial publication, state change, or heartbeat.

Illustrative C++ only:

```cpp
enum class ObservationReason
{
    Initial,
    StateChange,
    Heartbeat
};

struct EpeeElectricalState
{
    bool redTipDepressed;
    bool greenTipDepressed;

    // Candidate fields; precise electrical derivation is TBD.
    bool redTipConnectedToGreenEarth;
    bool greenTipConnectedToRedEarth;
    bool redTipConnectedToPiste;
    bool greenTipConnectedToPiste;
};

struct EpeeElectricalObservation
{
    Timestamp timestamp;
    ObservationReason reason;
    EpeeElectricalState state;
};
```

`EpeeElectricalObservation` is used here only to illustrate the contract. The intended public concept is **`ElectricalObservation`**; the final mechanism for carrying foil, épée, and sabre state (for example, a tagged variant) is TBD. No C++ layout or binary serialization format is specified here.

`redTipDepressed` and `greenTipDepressed` denote **electrically inferred tip depression**, not independently verified mechanical movement. The names `redTipOpen`, `redTipClosed`, and `redTipTouch` are deliberately not used as parallel outputs. One Boolean represents the interpreted two-state condition; fault or unknown states may require a richer type in a future revision.

The word **touch** is reserved for fencing/scoring meaning rather than an electrical switch state.

## 6. Publication and timing semantics

The initial periodic-scanner implementation shall consider **every completed** `ContinuityObservation` for interpreted state changes, even when it does not publish a corresponding `ElectricalObservation`.

An interpreted state change triggers publication without waiting for a heartbeat. A heartbeat republishes the complete latest state based on a **fresh underlying observation**; it must not fabricate a new measurement timestamp. The maximum end-to-end publication latency and scheduling guarantees are TBD.

A heartbeat indicates continued operation and reports the latest measured interpreted state. It does **not** prove that the physical topology remained unchanged continuously since the previous report.

For example:

```text
Input scans:       0 us   128 us   256 us   384 us   512 us ...
Red epee tip:        0       0        1        1        1  ...
Output:           Initial          StateChange       ... Heartbeat
```

The timestamps carried by outputs refer to the relevant **source observations**, not to exact physical transition times. A sequentially acquired `ContinuityMap` can contain measurements taken at different instants within one scan interval. This draft does not assign a simultaneous physical instant to every relationship in a map.

State changes that occur entirely between physical measurements may be missed. A heartbeat or faster publication schedule does not remove that acquisition limitation.

## 7. Delivery and reliability

Because the Game Engine reconstructs temporal relationships, detected scoring-relevant state changes must reach it **in observation order**, or it must be informed that continuity of the delivered sequence cannot be trusted.

This is a logical contract, independent of transport. A direct in-process C++ call, a task queue, and a serialized link are all possible implementation choices.

Queue capacity, backpressure, sequence numbers, fault reporting, reconfiguration, recovery after loss, and whether heartbeats may be coalesced are **TBD**. No claim of lossless delivery is made until those mechanisms are designed and tested.

## 8. Épée interpretation — first design target

The first implementation target is épée because its basic tip circuit is straightforward:

- Red épée: `RA-RB` closed is interpreted as `redTipDepressed = true`.
- Green épée: `GA-GB` closed is interpreted as `greenTipDepressed = true`.

The two relationships correspond to canonical bits 0 and 20, respectively. These mappings express inferred switch state, not hit validity.

Further candidate conditions are electrical connectivity between the active tip circuit and the opponent's C circuit, and between the active tip circuit and `CP`. These must be **reported independently** of tip depression; a seemingly disqualifying connection is not suppressed by the interpreter.

The exact definition of *tip connected to earth/piste* remains unresolved because it depends on which active épée conductor is electrically exposed at the pointe d'arrêt, whether this varies across weapons, and the treatment of indirect conductive paths. Do not finalize those Boolean derivations by assuming A and B are interchangeable before the tip switch closes.

In particular, the interpreter must not conclude that a tip physically struck a guard or the piste merely from an electrically connected network.

## 9. Foil and sabre — reserved

Foil and sabre electrical-state contracts remain to be developed. Known starting facts include:

- Foil: B–C is normally closed and opens when the pointe d'arrêt is depressed; A represents the fencer's conductive jacket.
- Sabre: B and C are normally joined through the weapon; A represents the fencer's valid conductive equipment.

These are starting points for later interpretation specifications, **not** complete hit-detection algorithms. The same continuity relationship may have different meanings for different weapons.

## 10. Future acquisition mechanisms

A future continuous-time watcher or hardware-latching system might bypass or replace the current path from `ContinuityObservation` through `ElectricalInterpreter` for scoring purposes. The universal continuity scanner should remain available for diagnostics and other features.

The intended long-lived boundary is the **meaning of the Game Engine's input**, not a guarantee that all future implementations emit exactly one interpreted observation per scan. A future capture mechanism may require additional temporal metadata or a revised output contract; that possibility is explicitly open.

## 11. Open design questions

- Precisely which épée connectivity fields are necessary, and how can each be derived from the full map without overstating physical certainty?
- How should unknown, inconsistent, or electrically abnormal states be represented?
- What is the final common `ElectricalObservation` type across the three weapons?
- Who sets heartbeat configuration, what intervals are supported, and how are changes applied?
- How are missed input observations, queue overflow, stale data, and restart/recovery communicated?
- What are the maximum acceptable interpretation and delivery latencies?
- How should timestamp precision and finite sequential-scan acquisition intervals be exposed to the Game Engine?
- How should the system validate contact qualification and brief-interruption behavior against applicable FIE requirements?

## 12. Next steps

1. Resolve the épée tip-conductor and earth/piste connectivity semantics using authoritative wiring information and, where useful, physical measurements.
2. Complete the `EpeeElectricalState` field definitions and synthetic topology examples.
3. Define and test a processor-independent interpreter prototype using existing `ContinuityMap` inputs.
4. Design the foil and sabre state contracts, then finalize the common output type and delivery guarantees.

**Drafting principle:** Specify observable meaning and consumer guarantees first; select masks, lookup tables, task scheduling, and hardware implementation later.
