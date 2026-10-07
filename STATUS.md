# Sentinel Project Status

## Purpose

This document is the authoritative current-state summary for Sentinel.

Use it to answer four questions:

1.  Where is the project now?
2.  What has been established?
3.  What remains uncertain?
4.  What should be worked on next?

Detailed experimental methods and raw evidence belong under
`experiments/`. Architectural reasoning belongs in
`docs/rationale/DESIGN_RATIONALE.md`. Normative behavior belongs in
`docs/specifications/`.

------------------------------------------------------------------------

## Current Phase

**ESP32-S3 scanner integration and transition to the game engine**

The scanner-characterization phase has answered the major electrical and
performance questions that originally blocked implementation.

The next major task is to promote the experimentally validated ESP32-S3
scanner into Sentinel's hardware layer while preserving the
processor-independent `ContinuityMap` boundary, then move upward into
game-engine work.

Further optimization of minimum scan time is not currently a project
requirement.

------------------------------------------------------------------------

## Current Version

``` text
v0.1.0
```

------------------------------------------------------------------------

## Current Repository Checkpoint

The latest completed scanner experiment and documentation were committed
and pushed through:

``` text
610c8b7  Document ESP32-S3 direct GPIO experiment
```

At that checkpoint:

``` text
branch: main
local: synchronized with origin/main
working tree: clean
```

------------------------------------------------------------------------

## Core Architecture

Sentinel separates electrical measurement from fencing interpretation.

The continuity scanner measures the electrical system and produces a
canonical `ContinuityMap`.

It does not determine:

-   foil, épée, or sabre rules
-   touches
-   blocking timing
-   scoring
-   display state
-   audible-signal behavior

Those belong to higher layers.

The processor-independent scanner is intentionally independent of:

-   processor family
-   GPIO numbering
-   board layout
-   electrical polarity
-   physical scan order
-   fencing game rules
-   scoring logic

Hardware-specific behavior belongs in the hardware-facing
implementation.

------------------------------------------------------------------------

## Completed Scanner Specifications

The current scanner architecture is defined by:

-   `docs/specifications/ELECTRICAL_MODEL.md`
-   `docs/specifications/CONTINUITY_SCANNER.md`

The reasoning behind the architecture is preserved in:

-   `docs/rationale/DESIGN_RATIONALE.md`

These documents should be reviewed before changing the
processor-independent scanner model.

------------------------------------------------------------------------

## Implemented Processor-Independent Scanner

The following components have been implemented:

-   `firmware/scanner/Line.h`
-   `firmware/scanner/ContinuityMap.h`
-   `firmware/scanner/ContinuityScanner.h`

Desktop validation includes:

-   `tests/scanner_smoke_test.cpp`
-   `tests/MockNodeIO.h`
-   `tests/continuity_scanner_test.cpp`

Verified behavior includes:

-   all 21 canonical line-pair bit assignments
-   symmetry of continuity queries
-   isolated continuity connections
-   transitive electrical connectivity
-   multiple independent connected components
-   correct isolation between components

`MockNodeIO` models physical electrical connectivity rather than merely
returning predetermined scanner answers. This allows the logical scanner
to remain independently testable as the physical hardware implementation
evolves.

------------------------------------------------------------------------

## Continuity Model

Sentinel models seven logical fencing lines:

``` text
RA
RB
RC
CP
GC
GB
GA
```

These produce 21 unique unordered electrical relationships.

One complete electrical snapshot is represented by a canonical
`ContinuityMap`.

Physical measurement order does not determine canonical bit position.
For example:

``` text
drive RA -> sense RC
```

and:

``` text
drive RC -> sense RA
```

both represent:

``` text
RA-RC
```

This separation remained valid throughout hardware characterization and
should be preserved.

------------------------------------------------------------------------

## Current Hardware Platform

The current development platform is:

``` text
LILYGO T-Display S3
ESP32-S3
```

Experiment 08 established this board as a viable Sentinel scanner
platform.

The current continuity GPIO assignment is:

  Sentinel line     GPIO
  --------------- ------
  RA                   1
  RB                   2
  RC                  10
  CP                  16
  GC                  11
  GB                  12
  GA                  13

The current proposed buzzer assignment is:

``` text
BZ = GPIO21
```

BZ was not part of Experiments 08 or 09 scanner characterization.

All seven continuity GPIOs reside in the ESP32-S3 GPIO0-31 bank. This
allows one GPIO input-register read to capture the physical state of all
seven continuity lines.

### Development-board resource tradeoffs

The current non-touch T-Display S3 allocation intentionally preserves:

``` text
GPIO17 / GPIO18
    I2C

GPIO43 / GPIO44
    UART / general expansion
```

The scanner consumes GPIO11/12/13, so the optional T-Display S3 SD-card
interface is not preserved.

The current allocation targets the non-touch T-Display S3. Touch-board
support would require reconsidering GPIO16/17/18/21.

These are development-board implementation constraints, not Sentinel
architectural requirements. A future custom Sentinel PCB may use a
different physical GPIO allocation.

------------------------------------------------------------------------

## Current Electrical Interface

The leading scanner-interface candidate uses:

``` text
3.9 kOhm external pull-up
```

on each of the seven continuity lines.

ESP32 internal pull-ups are disabled.

Measurement remains active-low:

``` text
released line
    output driver disabled
    external pull-up restores HIGH

selected line
    output latch LOW
    output driver enabled
    line driven LOW

connected sense line
    LOW

unconnected sense line
    HIGH
```

Experiment 07 demonstrated that pull-up resistance strongly affects
settling behavior. Experiments 08 and 09 then carried 3.9 kOhm forward
onto the ESP32-S3 platform.

3.9 kOhm is the current leading candidate, not an immutable production
specification.

------------------------------------------------------------------------

## Current ESP32-S3 Measurement Implementation

Experiment 09 replaced Arduino per-measurement GPIO operations with
direct ESP32-S3 GPIO register access.

The seven GPIO pads are configured once. Their output latches remain
LOW. A driven phase is then conceptually:

``` text
enable selected output driver
        |
        v
wait explicit drive-to-sample interval
        |
        v
read GPIO0-31 input register once
        |
        v
interpret required sense bits
        |
        v
disable selected output driver
```

This removes repeated `pinMode()`, `digitalWrite()`, and `digitalRead()`
operations from the measurement loop.

It also captures the continuity-line states from one physical
input-register snapshot rather than sequential sense reads.

The principal advantage is not simply speed. It makes the electrical
measurement interval substantially more explicit and deterministic.

------------------------------------------------------------------------

## Current Timing Baseline

The current conservative candidate is:

``` text
explicit drive-to-sample settling: 20 us
explicit release settling:          0 us
```

This requires careful interpretation.

Experiment 09 did **not** establish that 20 us is the exact minimum
reliable settling interval.

It established that:

-   10 us still produced failures in some tested conditions.
-   20 us was the first tested point with no observed errors across the
    complete Experiment 09 matrix.
-   50 us was also error-free across that matrix.

The actual transition for the most difficult tested conditions lies
somewhere between the tested 10 us and 20 us points.

Finding the exact minimum is not currently necessary for scanner
performance.

------------------------------------------------------------------------

## Experiment 08 --- ESP32-S3 Hardware Baseline

Directory:

``` text
experiments/08-esp32s3-hardware-baseline/
```

Experiment 08 migrated the existing Arduino-based scanner to the LILYGO
T-Display S3 using 3.9 kOhm external pull-ups.

It tested:

``` text
28 topologies
3 scan orders
10,000 scans per condition
```

at explicit drive-settling points:

``` text
0, 1, 2, 5, 10, 20, 50, 100 us
```

All 28 topology files established valid 1000/1000 trusted references.

Every tested condition at 1 us explicit drive settling or greater
completed without observed error.

At 1 us alone:

``` text
28 topologies
x 3 scan orders
x 10,000 scans
=
840,000 complete scans
```

were observed without error.

Zero explicit delay was nearly reliable but produced small error counts
in some topology/order combinations.

The important limitation was that Arduino GPIO operations themselves
consumed substantial execution time. Therefore the configured explicit
delay did not represent the complete physical drive-to-sample interval.

See:

``` text
experiments/08-esp32s3-hardware-baseline/README.md
```

for detailed methods, results, and raw evidence.

------------------------------------------------------------------------

## Experiment 09 --- Direct GPIO Characterization

Directory:

``` text
experiments/09-esp32s3-direct-gpio/
```

Experiment 09 retained the Experiment 08 hardware and 3.9 kOhm pull-ups
while replacing Arduino per-measurement GPIO operations with direct
ESP32-S3 register access.

It tested:

``` text
32 topologies
3 scan orders
10,000 scans per condition
```

at explicit drive-settling points:

``` text
0, 1, 2, 3, 5, 10, 20, 50 us
```

All 32 topology files established valid trusted references.

The important aggregate result was:

  -----------------------------------------------------------------------
     Explicit drive   Conditions with   Total incorrect              Mean
              delay            errors             scans     complete-scan
                                                                     time
  ----------------- ----------------- ----------------- -----------------
               0 us           96 / 96           959,526           6.09 us

               1 us           90 / 96           899,907          16.44 us

               2 us           90 / 96           895,278          21.70 us

               3 us           90 / 96           887,482          27.05 us

               5 us           82 / 96           514,109          38.56 us

              10 us           18 / 96            76,680          68.38 us

          **20 us**        **0 / 96**             **0**     **128.23 us**

          **50 us**        **0 / 96**             **0**     **307.89 us**
  -----------------------------------------------------------------------

At the tested 20 us point:

``` text
32 topologies
x 3 scan orders
x 10,000 scans
=
960,000 complete scans
```

were observed without error.

The mean complete 21-pair scan time was approximately:

``` text
128 us
```

or roughly:

``` text
7,800 complete maps per second
```

See:

``` text
experiments/09-esp32s3-direct-gpio/README.md
```

for detailed methods, interpretation, and raw results.

------------------------------------------------------------------------

## What Experiments 08 and 09 Established

Experiment 08 appeared nearly reliable with zero explicit delay and
fully reliable within its tested matrix at 1 us.

Experiment 09 showed why those explicit-delay numbers could not be
interpreted as the actual electrical settling requirement.

Removing Arduino GPIO overhead changed the observed behavior
dramatically:

``` text
0-5 us
    broadly unreliable

10 us
    substantially improved but still insufficient

20 us
    first universally error-free tested point

50 us
    universally error-free tested point
```

The central conclusion is:

> Software overhead must not be confused with an electrical timing
> guarantee.

The direct-register implementation is preferable because the
drive-to-sample interval is substantially more explicit.

------------------------------------------------------------------------

## Scanner Performance Conclusion

The historical scoring apparatus used approximately:

``` text
1 ms
```

of settling per driven-line phase.

A six-phase complete scan at that timing would spend approximately:

``` text
6 ms
```

in explicit settling alone.

Experiment 09 produced complete 21-pair maps at the tested 20 us
settling point in approximately:

``` text
128 us
```

on average.

The complete 21-pair measurement architecture therefore has substantial
performance margin on the ESP32-S3.

There is no current performance justification for reducing the
measurement model or continuing to optimize settling merely for scan
speed.

------------------------------------------------------------------------

## Current Experiment Record

The hardware-characterization sequence is:

``` text
01  ESP32 settling time
02  Full 21-pair scan
03  Full-scan settling characterization
04  Drive vs. release settling
05  Scan-order characterization
06  GPIO reassignment
07  External pull-up characterization
08  ESP32-S3 hardware baseline
09  ESP32-S3 direct GPIO characterization
```

Each experiment directory is intended to preserve:

``` text
question
    |
    v
method
    |
    v
raw evidence
    |
    v
interpretation
```

Later evidence may refine an interpretation without invalidating the
earlier experimental record.

------------------------------------------------------------------------

## What Is Established

The current evidence strongly supports the following conclusions:

1.  The complete canonical 21-pair continuity model is practical.
2.  The historical 1 ms delay is not an inherent requirement.
3.  Drive-to-sample settling is important for the observed failure
    mechanism.
4.  Explicit release settling did not provide the same corrective effect
    in the tested conditions.
5.  Scan-order sensitivity can reveal marginal electrical settling.
6.  Pull-up resistance strongly controls settling behavior.
7.  3.9 kOhm external pull-ups provide a useful tested operating region.
8.  The LILYGO T-Display S3 is a viable Sentinel development platform.
9.  Direct ESP32-S3 register access successfully implements the scanner.
10. Arduino GPIO overhead had been providing substantial implicit
    settling time.
11. One simultaneous GPIO register snapshot can capture all seven
    continuity lines.
12. The tested 20 us direct-register condition was error-free across the
    complete Experiment 09 matrix.
13. The complete scanner is fast enough that reducing the logical
    measurement model is unnecessary.

------------------------------------------------------------------------

## What Is Not Yet Established

Do not currently claim that:

-   20 us is the exact minimum reliable settling interval.
-   20 us is the permanent production specification.
-   3.9 kOhm is the only acceptable production pull-up value.
-   the RC-like physical explanation has been directly measured.
-   every possible fencing wiring configuration has been tested.
-   the T-Display S3 GPIO assignment is a permanent Sentinel
    architecture.
-   the current implementation has been validated across production
    temperature, supply-voltage, hardware-unit, or manufacturing
    variation.

These remain implementation and validation questions rather than reasons
to change the logical scanner architecture.

------------------------------------------------------------------------

## Current Candidate Hardware Baseline

``` text
Platform:
    LILYGO T-Display S3 / ESP32-S3

Scanner GPIO:
    RA = GPIO1
    RB = GPIO2
    RC = GPIO10
    CP = GPIO16
    GC = GPIO11
    GB = GPIO12
    GA = GPIO13

Proposed BZ:
    GPIO21

External pull-ups:
    3.9 kOhm

Measurement implementation:
    direct ESP32-S3 GPIO registers

Input acquisition:
    one GPIO0-31 register snapshot per driven phase

Explicit drive settling:
    20 us conservative candidate

Explicit release settling:
    0 us
```

These values define the current experimental baseline, not permanent
architectural requirements.

------------------------------------------------------------------------

## Current Development Direction

The scanner-characterization phase is mature enough that further
minimum-delay optimization is not the highest-value next task.

The preferred progression is:

``` text
validated Experiment 09 scanner
        |
        v
promote ESP32-S3 measurement implementation
into Sentinel hardware layer
        |
        v
preserve processor-independent ContinuityMap boundary
        |
        v
review / complete game-engine specification
        |
        v
implement and desktop-test game engine
        |
        v
integrate scanner -> game engine
        |
        v
LCD and BZ scoring presentation
```

Further hardware experiments should be driven by a concrete unanswered
product question rather than by a desire to continue reducing scan time.

------------------------------------------------------------------------

## Immediate Next Tasks

1.  Review the existing hardware abstraction between the
    processor-independent scanner and physical GPIO implementation.
2.  Determine how the proven Experiment 09 ESP32-S3 implementation
    should be promoted into the production firmware structure.
3.  Keep ESP32 register definitions and GPIO numbering in
    hardware-specific code.
4.  Preserve the canonical `ContinuityMap` as the boundary between
    measurement and interpretation.
5.  Review the existing game-engine architecture and specifications.
6.  Identify any missing rule/timing specification work before
    implementation.
7.  Begin integration along this boundary:

``` text
physical fencing wiring
        ->
ESP32-S3 hardware measurement
        ->
ContinuityMap
        ->
game engine
```

8.  Treat LCD and BZ as presentation/output concerns above the
    electrical scanner.

------------------------------------------------------------------------

## Documentation Roles

To avoid duplicating project knowledge:

``` text
STATUS.md
    current authoritative project state

docs/specifications/
    normative system behavior and interfaces

docs/rationale/DESIGN_RATIONALE.md
    durable explanation of why design decisions were made

experiments/
    detailed experimental methods, raw evidence, and interpretations
```

`STATUS.md` should remain current rather than becoming a complete
historical diary. Git history preserves older status snapshots.

------------------------------------------------------------------------

## Engineering Principles

The scanner investigation has repeatedly reinforced:

> **Measure first. Optimize from evidence.**

Experiment 09 adds an important refinement:

> **Make timing explicit before treating it as a design parameter.**

The next phase should use the experimentally validated scanner rather
than continue optimizing a performance problem that no longer constrains
the architecture.

------------------------------------------------------------------------

## Last Updated

2026-10-05
