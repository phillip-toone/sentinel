# Status

## Current Phase

Hardware Integration

## Current Milestone

ESP32 Continuity Scanner Electrical Interface Characterization

## Current Version

v0.1.0

## Project State

Sentinel has progressed from architecture and domain modeling through
processor-independent scanner implementation, desktop validation, and initial
ESP32 hardware characterization.

The processor-independent continuity scanner remains intact.

A complete seven-line scan measuring all 21 unique electrical relationships has
now been demonstrated on real ESP32 hardware with realistic fencing floor cords
and reels.

Early hardware testing reproduced the historical observation that long fencing
cabling can cause unreliable measurements when the scanner is operated with
insufficient electrical settling time.

Experiments 01 through 07 progressively characterized that behavior.

The most important current finding is that the full 21-pair scan remains
viable. The observed reliability problem is strongly influenced by the
electrical interface, particularly pull-up resistance and drive-to-sample
settling behavior.

No production pull-up resistance or production settling-time constant has yet
been selected.

---

## Completed Specifications

The following specifications define the current scanner architecture:

- `docs/specifications/ELECTRICAL_MODEL.md`
- `docs/specifications/CONTINUITY_SCANNER.md`

The design reasoning leading to the current architecture is preserved in:

- `docs/rationale/DESIGN_RATIONALE.md`

These documents should be reviewed before making architectural changes to the
scanner.

---

## Implemented Scanner Components

The following processor-independent firmware components have been implemented:

- `firmware/scanner/Line.h`
- `firmware/scanner/ContinuityMap.h`
- `firmware/scanner/ContinuityScanner.h`

The scanner is intentionally independent of:

- processor family
- GPIO numbering
- board layout
- fencing game rules
- electrical polarity
- scoring logic

Hardware-specific behavior is supplied through a hardware-facing interface.

---

## Continuity Representation

Sentinel models seven logical fencing lines:

```text
RA
RB
RC
MT
GC
GB
GA
```

These produce 21 unique unordered line pairs.

The canonical 21-pair ordering is defined in:

```text
docs/specifications/ELECTRICAL_MODEL.md
```

One complete electrical snapshot is represented by a `ContinuityMap`.

Measurement order is intentionally independent of canonical bitmap order.

For example:

```text
drive RA -> sense RC
```

and:

```text
drive RC -> sense RA
```

both represent the same canonical relationship:

```text
RA-RC
```

This distinction became important during hardware characterization because
different physical measurement orders can exhibit different transient
behavior while still representing the same logical continuity relationship.

---

## Desktop Validation

The processor-independent scanner has been compiled and tested successfully
using C++20 with warnings enabled.

The following behavior has been verified:

- all 21 canonical line-pair bit assignments
- symmetry of continuity queries
- single isolated continuity connections
- transitive electrical connectivity
- multiple independent connected components
- correct isolation between independent components

The current test infrastructure includes:

- `tests/scanner_smoke_test.cpp`
- `tests/MockNodeIO.h`
- `tests/continuity_scanner_test.cpp`

`MockNodeIO` models physical electrical connectivity rather than simply
returning predetermined scanner answers.

For example:

```text
RA ----- GC ----- MT
```

is treated as one connected electrical component, so the simulated electrical
model reports:

```text
RA-GC = continuity
RA-MT = continuity
GC-MT = continuity
```

This allows the processor-independent scanner to be validated independently of
the physical ESP32 interface.

---

## ESP32 Development Environment

A working ESP32 development environment has been established on the Linux Mint
development machine using PlatformIO and Visual Studio Code.

The workflow has been demonstrated through repeated experimental development:

```text
Sentinel repository
        |
        v
PlatformIO build
        |
        v
ESP32 upload
        |
        v
serial monitor
        |
        v
raw experimental results
        |
        v
repository documentation
```

The current experimental board is a TTGO T-Display based on the classic ESP32.

The current experimental GPIO mapping used by the principal scanner tests is:

```text
RA = GPIO21
RB = GPIO22
RC = GPIO17
MT = GPIO32
GC = GPIO25
GB = GPIO26
GA = GPIO27
```

These assignments are experimental and are not considered part of Sentinel's
processor-independent architecture.

A future board may use different GPIO assignments.

---

## Historical Measurement Method

The earlier fencing scoring apparatus used an active-low continuity
measurement method equivalent to:

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

The historical implementation therefore waited approximately 1 millisecond
after driving a line LOW before evaluating continuity.

That delay was selected empirically because it worked with realistic fencing
cabling.

It was not originally characterized or optimized.

The Sentinel hardware experiments began by investigating whether that
historical delay was necessary for a complete 21-pair scan.

---

## Full 21-Pair Scan

A complete seven-line scan requires six driven-line phases and 21 unique
measurements.

One canonical ordering is:

```text
Drive RA -> observe RB, RC, MT, GC, GB, GA
Drive RB -> observe RC, MT, GC, GB, GA
Drive RC -> observe MT, GC, GB, GA
Drive MT -> observe GC, GB, GA
Drive GC -> observe GB, GA
Drive GB -> observe GA
```

This produces:

```text
6 + 5 + 4 + 3 + 2 + 1 = 21
```

unique unordered relationships.

The original concern was that reproducing the historical 1 millisecond delay
for all six driven phases would require approximately:

```text
6 ms
```

of settling time per complete electrical snapshot.

Hardware characterization was therefore undertaken before changing the
architecture or abandoning the full scan.

The experimental evidence now supports retaining the full 21-pair model.

---

# Hardware Experiment History

The experimental record is preserved under:

```text
experiments/
```

Each completed experiment contains source code and, where applicable,
methodology, raw serial output, and a README describing the current
interpretation of the evidence.

The README conclusions are intentionally revisable as later experiments
provide additional information.

---

## Experiment 01 — ESP32 Settling Time

Directory:

```text
experiments/01-esp32-settling-time/
```

Purpose:

Characterize a simple driven-line/sense-line measurement and determine whether
an explicit settling delay is required for an isolated continuity
measurement.

Short jumper wiring and realistic cable loading were tested.

### Finding

The simple isolated measurement could operate with little or no explicit
settling delay under the tested conditions.

### Importance

This demonstrated that the ESP32 GPIO transition itself did not justify the
historical 1 millisecond delay.

However, the experiment did not reproduce the electrical history of a complete
rapidly switched 21-pair scan.

---

## Experiment 02 — ESP32 Full Scan

Directory:

```text
experiments/02-esp32-full-scan/
```

Purpose:

Test the complete six-phase / 21-pair scan on real hardware.

### Finding

Short wiring behaved well, but realistic floor cords and reels produced
substantial structured false-positive continuity measurements at short
settling intervals.

The errors were not random.

Certain logical relationships repeatedly appeared together.

### Methodological Finding

The initial expected-map methodology was not sufficiently trustworthy because
the scanner could establish an expected map using the same potentially
unsettled measurement process being evaluated.

Later experiments therefore adopted a conservative trusted-reference
procedure.

---

## Experiment 03 — Full-Scan Characterization

Directory:

```text
experiments/03-esp32-full-scan-characterization/
```

Purpose:

Characterize drive-to-sample settling systematically using a trusted reference.

### Trusted Reference

A reference map was established using conservative 1 millisecond settling and
required repeated unanimous scans.

### Finding

The settling interval required for reliable scanning depended strongly on
external cabling and electrical topology.

Realistic floor cords and reels required substantially more settling than
short local wiring.

The experiment confirmed that the long-cable problem was real and
repeatable.

---

## Experiment 04 — Drive vs. Release Settling

Directory:

```text
experiments/04-esp32-drive-release-settling/
```

Purpose:

Separate two possible timing requirements:

```text
drive LOW
    |
    v
wait
    |
    v
sample
```

versus:

```text
release previous line
    |
    v
wait
    |
    v
drive next line
```

### Finding

The observed failure mechanism was corrected by sufficient
drive-to-sample settling.

Adding release-only settling through the tested range did not provide the same
benefit.

### Conclusion

For the observed failures, the important timing interval is:

> time after driving a line LOW and before sampling the sense lines.

Release settling was therefore retained at zero in subsequent
characterization unless a later experiment provided evidence otherwise.

---

## Experiment 05 — Scan-Order Characterization

Directory:

```text
experiments/05-esp32-scan-order/
```

Purpose:

Determine whether the structured false-positive measurements depend on scan
order.

Three orders were tested:

```text
Forward:
RA -> RB -> RC -> MT -> GC -> GB -> GA

Reverse:
GA -> GB -> GC -> MT -> RC -> RB -> RA

Interleaved:
RA -> GA -> RB -> GB -> RC -> GC -> MT
```

The program translated every physical measurement back into the same canonical
21-bit representation.

### Major Finding

Scan order can dramatically affect reliability when the electrical interface
is operating near its settling limit.

For the difficult topology:

```text
100000000000000000001
```

at 100 us drive settling, the recorded results included:

```text
Forward:       9988 / 10000 incorrect
Reverse:          0 / 10000 incorrect
Interleaved:  10000 / 10000 incorrect
```

The Forward and Interleaved failures were associated with false-positive
RA-RC measurements.

All three orders passed the tested 150 us condition.

### Interpretation

Scan order matters under marginal electrical conditions.

However, changing order also changes measurement direction for some unordered
pairs, so the experiment did not establish scan order itself as the physical
cause.

---

## Experiment 06 — GPIO Reassignment

Directory:

```text
experiments/06-esp32-gpio-reassignment/
```

Purpose:

Determine whether the observed structured behavior was strongly associated
with the original ESP32 GPIO assignment.

The experimental mapping was changed from:

```text
RA = 21
RB = 22
RC = 17
MT = 32
GC = 25
GB = 26
GA = 27
```

to:

```text
RA = 21
RB = 32
RC = 17
MT = 25
GC = 26
GB = 33
GA = 13
```

Five of seven logical lines therefore moved to different GPIOs.

### Finding

GPIO reassignment changed some detailed settling behavior but did not eliminate
the phenomenon.

Scan order and topology remained important under marginal settling
conditions.

### Important Limitation

RA and RC remained on GPIO21 and GPIO17.

Therefore persistence of RA-RC behavior could not distinguish between:

```text
logical/external RA-RC behavior
```

and:

```text
physical GPIO21-GPIO17 behavior
```

Experiment 06 therefore weakened a simple explanation based on the complete
original GPIO assignment but did not identify the physical mechanism.

---

## Experiment 07 — External Pull-Up Characterization

Directory:

```text
experiments/07-esp32-external-pullups/
```

Purpose:

Determine how external pull-up resistance affects scanner settling and
reliability.

### Electrical Change

Earlier experiments used ESP32:

```text
INPUT_PULLUP
```

for released and sensed lines.

Experiment 07 instead used:

```text
INPUT
```

and supplied external pull-up resistors.

Five resistor values were tested:

```text
1.8 kΩ
3.9 kΩ
8.2 kΩ
15 kΩ
33 kΩ
```

Three electrical topologies were tested with all five resistor values,
producing 15 raw result files.

Each configuration retained the Forward, Reverse, and Interleaved scan orders
and tested:

```text
0 us
10 us
20 us
50 us
100 us
150 us
200 us
```

of explicit drive-to-sample settling.

### Major Result

External pull-up resistance had a large and systematic effect on the required
settling interval.

The first tested drive-settling value at which all three scan orders completed
10,000 scans without observed error was:

| External pull-up | `000000000000000000000` | `000000000010000010000` | `100000000000000000001` |
|---:|---:|---:|---:|
| 1.8 kΩ | 0 us | 0 us | 0 us |
| 3.9 kΩ | 0 us | 0 us | 0 us |
| 8.2 kΩ | 10 us | 10 us | 20 us |
| 15 kΩ | 10 us | 20 us | 50 us |
| 33 kΩ | 50 us | 50 us | 100 us |

These are tested points, not exact electrical threshold measurements.

### Strong Pull-Up Result

With both:

```text
1.8 kΩ
```

and:

```text
3.9 kΩ
```

all three tested topologies and all three scan orders completed the
zero-explicit-delay conditions without observed errors.

"Zero delay" means:

> no additional explicit `delayMicroseconds()` was inserted.

It does **not** mean that the physical electrical settling time is zero.

GPIO framework calls, processor execution, and digital reads still consume
finite time.

### Interpretation

The systematic relationship between pull-up resistance and required settling
time strongly supports an electrical-settling interpretation.

A plausible model is that cable and circuit capacitance interact with pull-up
resistance to determine how quickly sensed lines return toward a valid HIGH
level.

Lower pull-up resistance would therefore be expected to reduce the charging
time and reduce the required settling interval.

However, Experiment 07 did not directly measure:

- cable capacitance
- GPIO voltage versus time
- logic-threshold crossing time
- transient waveform shape
- an equivalent RC time constant

Therefore an RC-like interpretation is strongly supported but has not yet been
quantitatively established.

---

# Current Experimental Conclusions

The hardware experiments currently support the following conclusions.

## The Full Scan Remains Viable

There is currently no experimental reason to abandon Sentinel's complete
21-pair continuity scan.

The long-cable reliability problem can be strongly influenced by the electrical
interface without changing the processor-independent continuity model.

---

## The Historical 1 ms Delay Is Not Fundamentally Required

The historical 1 millisecond delay was a conservative empirical solution.

Experiment 07 demonstrated configurations in which no additional explicit
drive-settling delay was required for the tested full scans.

This does not yet establish the minimum timing requirement for production
hardware.

---

## Drive-to-Sample Settling Matters

For the observed failure mechanism, sufficient time between driving a line LOW
and sampling the sense lines eliminates the false-positive measurements.

Release-only settling did not provide the same benefit in Experiment 04.

---

## Pull-Up Resistance Matters

External pull-up resistance has emerged as a major electrical design
parameter.

Stronger pull-ups substantially reduced the tested settling requirement.

The resistor value cannot be selected solely by minimizing settling time
because stronger pull-ups also increase current when continuity causes a
driven LOW line to sink current through one or more pull-ups.

---

## Scan Order Is a Secondary Electrical Interaction

Scan order can dramatically affect measurements when the electrical interface
is near its settling limit.

With sufficiently strong pull-ups in Experiment 07, the tested scan-order
differences disappeared even at zero additional explicit settling delay.

This suggests that scan-order sensitivity is strongly coupled to electrical
settling rather than necessarily representing a fundamental flaw in the scan
algorithm.

---

## Errors Are Predominantly False Positive

Across the characterized failure conditions, the observed errors have been
dominated by false-positive continuity measurements.

The recurring pattern is consistent with lines being sampled LOW when the
trusted topology says they should be HIGH.

The physical waveform responsible has not yet been measured directly.

---

# Current Engineering Question

The principal engineering question has changed.

It is no longer:

> Can Sentinel perform the complete 21-pair scan quickly enough?

The evidence now supports retaining the full scan.

The current question is:

> What electrical interface should Sentinel use to provide reliable logic
> levels, adequate margin, acceptable GPIO current, and sufficient scan speed
> with realistic fencing cabling?

Important design variables now include:

- external pull-up resistance
- cable capacitance
- cable and connector resistance
- ESP32 input thresholds
- GPIO sink current
- simultaneous connected pull-up current
- protection circuitry
- noise margin
- measurement timing
- eventual processor/platform differences

---

# Immediate Next Goal

Evaluate the electrical engineering tradeoff for the Sentinel input interface
before selecting production values.

In particular, determine an appropriate external pull-up range by considering:

```text
settling speed
        |
        +---- GPIO sink current
        |
        +---- multiple connected lines
        |
        +---- noise margin
        |
        +---- cable resistance/capacitance
        |
        +---- power consumption
        |
        +---- electrical protection
```

The 1.8 kΩ and 3.9 kΩ experimental results are highly encouraging, but neither
value should yet be promoted automatically to a production design.

---

# Recommended Next Investigation

Before optimizing scanner software, investigate the electrical interface more
directly.

Useful next measurements may include:

- actual line voltage versus time after a drive transition
- approximate cable capacitance
- current through the driven GPIO for representative continuity topologies
- effect of multiple simultaneously connected pull-ups
- logic HIGH and LOW margins
- behavior with representative floor cords, reels, body cords, and weapons
- comparison of candidate external pull-up values under longer-duration tests

Oscilloscope measurements may be particularly valuable because they could
directly connect the observed software error thresholds with the physical
voltage transient.

The exact next experiment should be selected after reviewing the electrical
current and protection requirements.

---

# Current Architectural Direction

The processor-independent scanner should remain unchanged unless future
evidence demonstrates an architectural problem.

Hardware-specific code should continue to normalize physical active-low
measurements into Sentinel's logical continuity representation.

Conceptually, the hardware-facing interface remains equivalent to:

```text
beginMeasurement(line)
snapshot()
endMeasurement()
```

For the current ESP32 experimental method:

```text
beginMeasurement(line)
    -> configure selected line OUTPUT
    -> drive selected line LOW
    -> allow required electrical settling

snapshot()
    -> read sense lines
    -> interpret LOW as physical continuity
    -> convert results to canonical logical continuity

endMeasurement()
    -> release driven line back to input state
```

Whether the production input state uses internal pull-ups, external pull-ups,
or additional interface circuitry is a hardware design decision and should not
leak into the processor-independent scanner.

---

# Guidance for Future Development

Do not abandon the full 21-pair scan based on the historical 1 millisecond
delay.

Do not assume ESP32 GPIO transition specifications determine the complete
electrical settling requirement.

Do not treat scan order as the fundamental cause merely because order affects
marginal measurements.

Do not treat GPIO adjacency or capacitive GPIO coupling as an established root
cause without direct evidence.

Do not interpret zero explicit delay as zero physical settling time.

Do not select the lowest pull-up resistance solely because it produces the
fastest settling.

Do not optimize low-level GPIO access before accounting for the fact that
software overhead currently contributes some implicit settling time.

Continue to preserve raw experimental evidence and revise interpretations when
later experiments provide better explanations.

The engineering principle remains:

> **Measure first. Optimize from evidence.**

---

# Experimental Record

The current hardware characterization sequence is:

```text
01  ESP32 settling time
        |
        v
02  Full 21-pair scan
        |
        v
03  Full-scan settling characterization
        |
        v
04  Drive vs. release settling
        |
        v
05  Scan-order characterization
        |
        v
06  GPIO reassignment
        |
        v
07  External pull-up characterization
```

The experiment directories are intended to preserve:

```text
question
    |
    v
test method
    |
    v
raw evidence
    |
    v
current interpretation
```

Later evidence may revise an interpretation without invalidating the historical
experimental record.

---

# Last Known Clean Repository State

Experiments 01 through 07 and their current documentation have been committed
and pushed through:

```text
2a240c8  Document ESP32 external pull-up experiment
```

At that point:

```text
branch: main
local: synchronized with origin/main
working tree: clean
```

---

# Next Documentation Task

Update:

```text
docs/rationale/DESIGN_RATIONALE.md
```

to preserve the architectural lessons from hardware characterization,
particularly:

- retain the complete measurement model while improving the electrical
  interface,
- distinguish logical measurement architecture from physical electrical
  implementation,
- treat settling behavior as a system-level electrical property rather than a
  GPIO transition-time property,
- preserve experimental evidence before optimizing implementation.

---

# Last Updated

2026-08-31