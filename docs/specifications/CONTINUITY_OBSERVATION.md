# Sentinel Continuity Observation Specification

## Status

Draft

------------------------------------------------------------------------

# Purpose

This specification defines the temporal observation boundary between
Sentinel's continuity measurement subsystem and higher architectural
layers.

The Continuity Scanner produces one complete electrical snapshot
represented by a `ContinuityMap`.

A `ContinuityObservation` associates one such scanner result with a
defined point on a monotonic timeline.

This specification defines:

-   what constitutes a `ContinuityObservation`,
-   the meaning of its timestamp,
-   the guarantees provided by an observation stream,
-   the responsibilities explicitly excluded from this layer, and
-   the timing properties that higher layers may rely upon once their
    required values are established.

This specification does not define fencing rules, touch interpretation,
scoring, processor scheduling, communication protocols, or physical GPIO
behavior.

------------------------------------------------------------------------

# Architectural Position

The relevant architectural flow is:

``` text
Physical Electrical Apparatus
            |
            v
Continuity Scanner
            |
            v
ContinuityMap
            |
            v
ContinuityObservation
{ map + timestamp }
            |
            v
Observation Stream
            |
            v
Higher Interpretation Layers
```

The Electrical Model defines the meaning of a `ContinuityMap`.

The Continuity Scanner defines how the processor-independent scanner
produces one complete electrical snapshot.

This specification begins after a complete scanner result exists.

It adds temporal association without adding electrical or fencing
interpretation.

------------------------------------------------------------------------

# Terminology

## Electrical Snapshot

An Electrical Snapshot is one complete observation of the electrical
state of the fencing apparatus as defined by the Electrical Model.

The processor-independent representation of that snapshot is a
`ContinuityMap`.

The measurements that form a snapshot may be acquired over a finite scan
interval.

------------------------------------------------------------------------

## ContinuityMap

A `ContinuityMap` is the canonical representation of the twenty-one
unique unordered continuity relationships among Sentinel's seven logical
electrical lines.

A `ContinuityMap` contains electrical state only.

It contains no timestamp, historical state, fencing interpretation, or
scoring information.

------------------------------------------------------------------------

## ContinuityObservation

A `ContinuityObservation` associates one complete `ContinuityMap` with a
monotonic timestamp.

Conceptually:

``` cpp
struct ContinuityObservation
{
    ContinuityMap map;
    Timestamp timestamp;
};
```

This example is conceptual.

This specification does not yet define the concrete C++ representation
of `Timestamp` or `ContinuityObservation`.

------------------------------------------------------------------------

## Observation Stream

An Observation Stream is a chronologically ordered sequence of
`ContinuityObservation` values.

Conceptually:

``` text
timestamp    map
---------    ---------------------
t1           A
t2           A
t3           A
t4           B
t5           B
t6           A
```

Repeated maps are valid observations.

An unchanged electrical topology does not eliminate the observation that
it was measured again.

------------------------------------------------------------------------

## Timestamp

A timestamp identifies one observation on Sentinel's monotonic
observation timeline.

It exists so higher layers can reason about elapsed time between
electrical observations.

It is not civil or wall-clock time.

------------------------------------------------------------------------

# ContinuityObservation Contents

Each `ContinuityObservation` contains exactly two conceptual pieces of
information:

``` text
ContinuityMap
Timestamp
```

The `ContinuityMap` answers:

> What complete electrical topology did the scanner observe?

The timestamp answers:

> When did that completed observation become available on the monotonic
> observation timeline?

No fencing meaning is attached to either field by this layer.

------------------------------------------------------------------------

# Map Semantics

The `map` contained in a `ContinuityObservation` shall be the complete
`ContinuityMap` produced by one completed scanner cycle.

The observation layer shall preserve that map without modification.

It shall not:

-   add continuity relationships,
-   remove continuity relationships,
-   correct a topology,
-   infer a more likely topology,
-   combine multiple maps into one map,
-   substitute the previous map,
-   apply fencing-specific expectations.

A map that appears unusual or impossible to a higher layer is still
preserved as measured.

------------------------------------------------------------------------

# Timestamp Semantics

The timestamp shall use a monotonic time domain.

For the initial architectural definition, the timestamp represents:

> The monotonic time at which the complete electrical snapshot became
> available from the scanner.

This definition deliberately refers to completion of the complete
observation.

It does not claim that all twenty-one electrical relationships were
physically sampled at one mathematical instant.

The underlying `ContinuityMap` may have been assembled over a finite
scan interval.

------------------------------------------------------------------------

# Monotonic Time

Observation timing shall not depend on civil time.

The observation timeline shall not be affected by:

-   calendar date,
-   time zone,
-   daylight-saving changes,
-   manual clock adjustment,
-   network time synchronization,
-   real-time-clock correction.

Higher layers need elapsed-time relationships between observations, not
the human-readable time of day.

Civil timestamps may be associated with logs or external records
elsewhere in the system, but they are outside this specification.

------------------------------------------------------------------------

# Finite Acquisition Interval

A complete `ContinuityMap` may require multiple physical measurements
and may therefore be assembled over a finite interval.

The observation timestamp identifies the completion of the resulting
complete snapshot.

This specification does not require the physical continuity
relationships to be sampled simultaneously.

It also does not expose the individual physical measurement times that
contributed to the map.

If future game-engine requirements demonstrate that phase-level timing
is necessary, that requirement shall be evaluated explicitly rather than
assumed here.

------------------------------------------------------------------------

# One Scanner Cycle Produces One Observation

For the current architecture:

> Every completed scanner cycle conceptually produces one
> `ContinuityObservation`.

This remains true when the resulting `ContinuityMap` is identical to the
previous map.

For example:

``` text
scan 1 -> A -> observation A @ t1
scan 2 -> A -> observation A @ t2
scan 3 -> A -> observation A @ t3
scan 4 -> B -> observation B @ t4
```

The observation boundary therefore does not suppress duplicate maps.

This preserves the fact that the electrical state was measured
repeatedly.

A future implementation may revisit delivery or transport optimization
if a concrete requirement justifies doing so.

Such optimization shall not silently change the semantic meaning of a
`ContinuityObservation`.

------------------------------------------------------------------------

# Required Guarantees

An observation source shall provide the following guarantees.

## Completeness

Every `ContinuityObservation` contains one complete `ContinuityMap`.

Partial scanner results are not `ContinuityObservation` values.

------------------------------------------------------------------------

## Chronological Ordering

Observations shall be presented in monotonic timestamp order.

A later-delivered observation shall not represent an earlier point on
the observation timeline than an observation already delivered through
the same ordered stream.

The exact requirement for strictly increasing timestamp values depends
on the eventual timestamp resolution and representation and remains to
be finalized.

------------------------------------------------------------------------

## Fidelity

The observation layer shall preserve the scanner-produced
`ContinuityMap` exactly.

No filtering, inference, correction, or fencing interpretation is
performed at this boundary.

------------------------------------------------------------------------

## Temporal Association

Every observation shall be associated with a timestamp having the
semantics defined by this specification.

Higher layers shall not need to obtain platform-specific time merely to
determine when the observation occurred on the observation timeline.

------------------------------------------------------------------------

## Independence

Each `ContinuityObservation` is a complete measurement record for one
scanner cycle.

The observation layer does not require historical state to determine the
contents of the current observation.

Historical interpretation belongs to higher layers.

------------------------------------------------------------------------

## Bounded Observation Interval

While active observation is required, the observation source shall
produce complete observations frequently enough to satisfy a defined
maximum observation interval.

The required maximum interval is not yet specified.

It shall be derived from higher-level fencing timing requirements rather
than from the maximum speed of a particular processor implementation.

``` text
Maximum observation interval: TBD
```

------------------------------------------------------------------------

# Prohibited Responsibilities

The Continuity Observation layer shall not perform:

-   electrical topology correction,
-   debounce,
-   majority voting,
-   duplicate suppression,
-   noise filtering based on historical observations,
-   minimum-contact qualification,
-   weapon interpretation,
-   touch detection,
-   blocking timing,
-   scoring,
-   game-state transitions,
-   display behavior,
-   audible-signal behavior.

If a scanner produces:

``` text
A
A
B
A
A
```

the conceptual observation stream preserves:

``` text
A @ t1
A @ t2
B @ t3
A @ t4
A @ t5
```

even if `B` existed for only one scanner cycle.

Whether `B` is meaningful belongs to a higher layer.

------------------------------------------------------------------------

# No Inference

The observation layer reports measured results.

It shall not manufacture intermediate observations or infer electrical
states that were not produced by completed scanner cycles.

For example:

``` text
A -> B
```

shall not be transformed into:

``` text
A -> X -> B
```

because another state is believed to have existed between the measured
observations.

------------------------------------------------------------------------

# No Validity Flag Is Defined

This specification does not currently define:

``` text
valid
invalid
```

as a property of an individual `ContinuityObservation`.

The current scanner architecture reports what it measures, and no
processor-independent criterion has been defined by which an individual
completed scan declares itself invalid.

A validity concept shall not be introduced until its semantics and
source of evidence are defined.

------------------------------------------------------------------------

# No Sequence Number Is Required

A `ContinuityObservation` does not currently require a sequence number.

Chronological identity is provided by the observation timeline.

Sequence numbers may be useful in communication protocols for purposes
such as detecting dropped packets.

That is a transport concern and does not, by itself, justify adding
sequence identity to the processor-independent observation model.

If a future processor-independent requirement emerges, this decision may
be revisited.

------------------------------------------------------------------------

# Timing Requirements

Several timing details intentionally remain unresolved.

## Maximum Observation Interval

``` text
TBD
```

This value shall be derived from the temporal requirements of fencing
interpretation.

------------------------------------------------------------------------

## Timestamp Resolution

``` text
TBD
```

The resolution shall be fine enough to represent the observation timing
required by higher layers.

------------------------------------------------------------------------

## Timestamp Representation

``` text
TBD
```

This specification does not currently require a particular integer
width, duration type, clock API, or C++ representation.

------------------------------------------------------------------------

## Timestamp Rollover

``` text
TBD
```

The eventual representation shall define behavior that permits higher
layers to determine elapsed time correctly across any supported rollover
condition.

------------------------------------------------------------------------

# Execution Independence

This specification defines information and guarantees, not execution
architecture.

It does not require:

-   one processor core,
-   multiple processor cores,
-   one operating-system task,
-   multiple tasks,
-   FreeRTOS,
-   queues,
-   shared memory,
-   callbacks,
-   polling,
-   interrupts,
-   USB,
-   UART,
-   network transport,
-   a particular processor family.

For example, all of the following may satisfy the same architectural
contract:

``` text
scan
 |
 v
ContinuityObservation
 |
 v
game processing
```

``` text
scanner task
     |
     v
    queue
     |
     v
game task
```

``` text
scanner device
      |
      v
 communication transport
      |
      v
remote consumer
```

The choice among these belongs to implementation design after the
required information and timing guarantees are known.

------------------------------------------------------------------------

# Scanner Independence

The Continuity Scanner itself does not need to know about clocks or
`ContinuityObservation`.

Its processor-independent responsibility remains:

``` text
scan
  |
  v
ContinuityMap
```

An implementation may associate the completed map with monotonic time at
an appropriate boundary above or around the scanner:

``` text
ContinuityMap + monotonic time
              |
              v
ContinuityObservation
```

This preserves the scanner's independence from platform-specific timing
facilities.

------------------------------------------------------------------------

# Higher-Layer Independence

A consumer of `ContinuityObservation` shall not need to know:

-   how GPIOs were measured,
-   how many physical measurement phases were required,
-   which processor produced the map,
-   which GPIO numbers were used,
-   which pull-up resistance was used,
-   whether Arduino calls or direct registers were used,
-   which processor core performed the scan,
-   whether the observation crossed a communication link.

Those are physical and execution implementation details.

The observation boundary exposes electrical topology and monotonic time.

------------------------------------------------------------------------

# Testing and Replay

Including time in the observation boundary enables deterministic
higher-layer testing.

A test may provide synthetic observations such as:

``` text
A @ 0
A @ 1000
B @ 14999
B @ 15000
```

without waiting for those durations in real execution.

This allows higher-layer timing behavior to be tested at exact
boundaries.

Likewise, a recorded observation stream may be replayed into higher
layers:

``` text
recorded observations
        |
        v
higher-layer interpretation
```

This supports:

-   deterministic unit tests,
-   timing boundary tests,
-   regression tests,
-   recorded-match replay,
-   diagnosis of unexpected scoring behavior,
-   comparison of different interpretation implementations.

The higher layer should consume observation time rather than obtain
platform-specific time internally when interpreting the observation
history.

------------------------------------------------------------------------

# Relationship to Fencing Timing

This specification intentionally does not define:

-   minimum contact times,
-   touch qualification intervals,
-   blocking periods,
-   simultaneous-touch windows,
-   weapon-specific timing rules.

Those requirements belong to higher-level fencing interpretation and
game-rule specifications.

They will be used to determine the unresolved observation timing
requirements, particularly:

``` text
Maximum observation interval
Timestamp resolution
```

------------------------------------------------------------------------

# Future Compatibility

The observation model is intended to remain valid across different
physical and execution implementations.

A future Sentinel implementation may use:

-   a different processor,
-   a different scanner implementation,
-   a custom PCB,
-   different GPIO assignments,
-   different electrical timing,
-   a different task structure,
-   an external diagnostic consumer,
-   recorded or simulated observation sources.

As long as the source provides the information and guarantees defined
here, higher layers should not require modification merely because the
physical implementation changed.

------------------------------------------------------------------------

# Summary

A `ContinuityMap` answers:

> What complete electrical topology was observed?

A `ContinuityObservation` answers:

> What complete electrical topology was observed, and when did that
> completed observation become available on Sentinel's monotonic
> timeline?

An Observation Stream is the ordered sequence of those observations.

For the current architecture:

> One completed scanner cycle conceptually produces one
> `ContinuityObservation`, including when its map is identical to the
> previous observation.

The observation layer preserves measurement rather than interpreting it.

It adds time without adding fencing meaning.

This creates a processor-independent temporal boundary suitable for
deterministic game-engine interpretation, testing, replay, and future
execution architectures.
