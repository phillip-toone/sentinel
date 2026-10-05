# Experiment 09 — ESP32-S3 Direct GPIO Characterization

## Purpose

Experiment 09 determines how replacing Arduino per-measurement GPIO operations
with direct ESP32-S3 GPIO register access affects:

- complete-scan execution time,
- electrical settling behavior,
- scan-order sensitivity, and
- the explicit drive-to-sample delay required for reliable continuity
  measurement.

Experiment 08 established that the complete Sentinel 21-pair scanner operates
successfully on the LILYGO T-Display S3 with 3.9 kΩ external pull-ups.

However, Experiment 08 still used Arduino GPIO operations such as:

```cpp
pinMode()
digitalWrite()
digitalRead()
```

Those operations consume significant processor time.

Therefore an Experiment 08 condition labeled:

```text
0 us explicit drive settling
```

did not represent zero physical settling time.

Experiment 09 removes much of that software overhead and makes the electrical
timing substantially more explicit and deterministic.

---

## Experimental Question

The primary question is:

> How much explicit drive-to-sample settling is required when Arduino
> per-measurement GPIO operations are replaced by direct ESP32-S3 GPIO
> register access?

A secondary question is:

> How much faster can the complete 21-pair scan execute when the GPIO
> implementation is reduced to direct drive/release register operations and
> one simultaneous GPIO input-register snapshot per driven phase?

---

## Hardware

Board:

```text
LILYGO T-Display S3
ESP32-S3
```

External fencing hardware:

```text
floor cords + reels
```

External pull-ups:

```text
3.9 kΩ
```

were installed on all seven continuity lines.

ESP32 internal pull-ups were disabled.

The buzzer was not part of this experiment.

---

## GPIO Assignment

Experiment 09 retained the Experiment 08 GPIO mapping:

| Sentinel line | GPIO |
|---|---:|
| RA | 1 |
| RB | 2 |
| RC | 10 |
| MT | 16 |
| GC | 11 |
| GB | 12 |
| GA | 13 |

The proposed buzzer assignment remains:

```text
BZ = GPIO21
```

but BZ was not exercised.

All seven continuity GPIOs are located in the ESP32-S3 GPIO0–31 bank.

This allows the physical state of all seven continuity lines to be captured
with one GPIO input-register read.

---

# Controlled Variables

Experiment 09 intentionally retained the following from Experiment 08:

```text
LILYGO T-Display S3
same GPIO assignment
3.9 kΩ external pull-ups
full canonical 21-pair continuity map
active-low measurement
trusted-reference methodology
Forward scan order
Reverse scan order
Interleaved scan order
10,000 scans per characterization condition
zero explicit experimental release settling
```

The primary changed variable was:

```text
Arduino per-measurement GPIO operations
        ↓
direct ESP32-S3 GPIO register operations
```

The short-delay characterization points were also refined to better resolve
the expected transition region.

---

# Direct GPIO Design

## One-Time GPIO Configuration

The seven scanner GPIO pads are configured once during startup.

The intended steady-state configuration is:

```text
input enabled
internal pull-up disabled
internal pull-down disabled
output capability configured
output latch LOW
output driver normally disabled
```

The external 3.9 kΩ pull-ups hold released lines HIGH.

The GPIO mode is not repeatedly reconfigured during scanning.

---

## Output Latches Remain LOW

All seven scanner output latches are set LOW during initialization and remain
LOW throughout the experiment.

A line is therefore driven or released solely through its output-enable state:

```text
output driver disabled
        ↓
line released
        ↓
external 3.9 kΩ pull-up restores HIGH
```

versus:

```text
output driver enabled
        ↓
existing LOW output latch drives line LOW
```

This eliminates repeated `digitalWrite()` operations from the measurement
loop.

---

## Drive Operation

Driving one Sentinel line consists of setting its output-enable bit through the
ESP32-S3 GPIO register interface.

Conceptually:

```text
output latch already LOW
        ↓
enable selected output driver
        ↓
selected line driven LOW
```

---

## Simultaneous Input Snapshot

After the configured explicit settling interval, Experiment 09 reads:

```text
GPIO_IN_REG
```

once.

Because all seven Sentinel continuity lines are in GPIO0–31, that single
register read captures all seven physical input states.

Every sense-line decision for the current driven phase is then made from that
same captured register value.

This differs importantly from Experiment 08, which effectively performed
sequential Arduino `digitalRead()` calls.

Conceptually:

```text
Experiment 08

read sense line
read next sense line
read next sense line
...
```

became:

```text
Experiment 09

one GPIO_IN_REG snapshot
        ↓
interpret all required sense lines
```

This provides both lower overhead and a more literal electrical snapshot.

---

## Release Operation

After interpreting the snapshot, the selected driven line is released by
clearing its output-enable bit.

Conceptually:

```text
disable output driver
        ↓
line becomes electrically released
        ↓
external 3.9 kΩ pull-up restores HIGH
```

No explicit experimental release delay was used.

---

# Canonical Continuity Representation

The direct GPIO implementation does not change Sentinel's logical electrical
model.

Seven logical lines produce 21 unique unordered continuity relationships.

Physical scan order remains independent of canonical bitmap position.

For example:

```text
drive RA -> sense RC
```

and:

```text
drive RC -> sense RA
```

represent the same canonical relationship:

```text
RA-RC
```

The direct-register implementation changes how the electrical state is
acquired, not what the resulting `ContinuityMap` means.

---

# Scan Orders

Experiment 09 retained the same three diagnostic scan orders used previously.

## Forward

```text
RA -> RB -> RC -> MT -> GC -> GB -> GA
```

## Reverse

```text
GA -> GB -> GC -> MT -> RC -> RB -> RA
```

## Interleaved

```text
RA -> GA -> RB -> GB -> RC -> GC -> MT
```

These orders create different electrical histories while producing the same
canonical 21-bit representation.

---

# Trusted Reference

Before characterization, each topology established a conservative trusted
reference using:

```text
Reference scan order:       Forward
Drive settling:             1000 us
Release settling:           1000 us
Reference scans:            1000
```

All 1,000 reference scans were required to agree.

All 32 recorded Experiment 09 topology files established:

```text
Reference status: VALID
```

Therefore all characterization results were compared against a stable trusted
reference.

---

# Characterization Conditions

The explicit drive-to-sample settling intervals were:

```text
0 us
1 us
2 us
3 us
5 us
10 us
20 us
50 us
```

Experimental release settling remained:

```text
0 us
```

Each scan-order / delay condition performed:

```text
10,000 complete scans
```

Each topology therefore contained:

```text
3 scan orders
×
8 settling intervals
=
24 characterization conditions
```

---

# Tested Topologies

Experiment 09 recorded:

```text
32 topology files
```

using floor cords and reels.

The topology is encoded directly in each result filename using Sentinel's
canonical 21-bit representation.

The complete raw serial output is preserved under:

```text
results/floorCords+reels/
```

The Experiment 09 topology set is not identical to the Experiment 08 set and
should therefore be interpreted as its own characterization dataset rather
than as a strict file-for-file paired comparison.

---

# Results

## Aggregate Results

Across the 32 recorded topologies, each settling interval contained:

```text
32 topologies
×
3 scan orders
=
96 characterization conditions
```

with:

```text
10,000 scans per condition
```

The aggregate results were:

| Explicit drive delay | Conditions with errors | Total incorrect scans | Mean complete-scan time |
|---:|---:|---:|---:|
| 0 us | 96 / 96 | 959,526 | 6.09 us |
| 1 us | 90 / 96 | 899,907 | 16.44 us |
| 2 us | 90 / 96 | 895,278 | 21.70 us |
| 3 us | 90 / 96 | 887,482 | 27.05 us |
| 5 us | 82 / 96 | 514,109 | 38.56 us |
| 10 us | 18 / 96 | 76,680 | 68.38 us |
| **20 us** | **0 / 96** | **0** | **128.23 us** |
| **50 us** | **0 / 96** | **0** | **307.89 us** |

The transition from unreliable to universally error-free operation within the
tested matrix therefore occurred between:

```text
10 us
```

and:

```text
20 us
```

of explicit drive-to-sample settling.

---

# Zero-Delay Result

At:

```text
0 us
```

all:

```text
96 / 96
```

topology / scan-order conditions produced at least one incorrect scan.

Across those conditions:

```text
959,526
```

of the:

```text
960,000
```

complete scans were incorrect.

Therefore direct-register scanning with no explicit drive-to-sample delay is
not merely marginal under this test configuration.

It is fundamentally too fast for the tested electrical system to settle
reliably.

The mean complete 21-pair scan time at zero explicit delay was only:

```text
6.09 us
```

---

# Low-Single-Digit Delay Region

The:

```text
1 us
2 us
3 us
```

conditions remained strongly unreliable.

At:

```text
1 us
```

90 of 96 conditions produced errors.

At:

```text
2 us
```

90 of 96 conditions produced errors.

At:

```text
3 us
```

90 of 96 conditions produced errors.

Although some topology/order combinations were already capable of error-free
operation, these delays did not provide sufficient general margin.

---

# 5 us Transition Region

At:

```text
5 us
```

the scanner showed substantial improvement.

However:

```text
82 / 96
```

conditions still produced errors.

Therefore 5 us remained clearly insufficient as a general scanner settling
interval for the tested hardware.

---

# 10 us Result

At:

```text
10 us
```

most topology/order conditions operated correctly.

However:

```text
18 / 96
```

conditions still produced errors, totaling:

```text
76,680 incorrect scans
```

across the complete dataset.

Therefore 10 us is not supported as a generally reliable settling interval by
Experiment 09.

---

# 20 us Result

At:

```text
20 us
```

all:

```text
96 / 96
```

tested topology/order conditions completed:

```text
10,000 scans
```

without observed error.

This represents:

```text
32 topologies
×
3 scan orders
×
10,000 scans
=
960,000 complete scans
```

with zero observed errors.

The mean complete 21-pair scan time was approximately:

```text
128.23 us
```

corresponding to roughly:

```text
7,800 complete continuity maps per second
```

across the tested dataset.

Experiment 09 therefore establishes:

> 20 us is the first tested explicit drive-settling interval at which every
> tested topology and scan order completed without observed error.

This is a tested point, not an exact determination of the minimum reliable
electrical settling time.

---

# 50 us Result

At:

```text
50 us
```

all 96 topology/order conditions also completed without observed error.

The mean complete scan time was approximately:

```text
307.89 us
```

This confirms that the scanner remains reliable with substantial additional
timing margin.

---

# Error Character

The failures observed at insufficient settling intervals were predominantly
false-positive continuity measurements.

That is consistent with the electrical-settling interpretation developed in
earlier experiments:

```text
line released / expected HIGH
        ↓
insufficient time to rise
        ↓
sample occurs while voltage remains LOW enough to be interpreted as LOW
        ↓
false-positive continuity
```

Some false-negative behavior was also observed in the shortest-delay
conditions, particularly around the most extreme timing cases.

Experiment 09 does not directly measure the analog voltage waveform, so this
interpretation remains based on digital behavior rather than oscilloscope
measurement.

---

# Comparison With Experiment 08

Experiment 08 used Arduino GPIO operations.

Its important result was:

```text
0 us explicit delay
    nearly reliable but marginal

1 us explicit delay
    all tested conditions error-free
```

Experiment 09 removed much of the Arduino GPIO overhead.

Its result was dramatically different:

```text
0-3 us
    strongly unreliable

5 us
    transition region but broadly unreliable

10 us
    mostly reliable but still insufficient

20 us
    all tested conditions error-free

50 us
    all tested conditions error-free
```

This difference demonstrates that the Arduino implementation had been
providing substantial implicit electrical settling time.

Therefore:

> Explicit delay values cannot be compared independently of the software
> operations surrounding them.

A configured `0 us` delay does not define an electrical measurement interval
unless the execution path around that delay is also understood.

---

# Scan-Time Comparison

Experiment 08 required approximately:

```text
high-170 to low-180 us
```

for a complete scan even at zero explicit drive delay.

Experiment 09 reduced the zero-delay complete-scan time to approximately:

```text
6.09 us
```

on average.

This is roughly a thirty-fold reduction in complete-scan execution time.

The large reduction confirms that Arduino GPIO operations represented a major
portion of the Experiment 08 execution time.

It also explains why Experiment 08 appeared electrically reliable with much
smaller configured explicit delays.

---

# Why the Direct GPIO Implementation Is Preferable

The primary benefit of direct GPIO access is not simply maximum speed.

The more important benefit is deterministic control of measurement timing.

Experiment 08 effectively contained:

```text
requested explicit delay
+
Arduino GPIO implementation overhead
+
sequential input-read overhead
```

Experiment 09 moves toward:

```text
enable output driver
+
known explicit settling interval
+
one simultaneous input-register snapshot
+
disable output driver
```

This makes the timing requirement an intentional electrical parameter rather
than an accidental consequence of software overhead.

The direct implementation also more closely matches Sentinel's conceptual
measurement abstraction:

```text
beginMeasurement(line)
snapshot()
endMeasurement(line)
```

---

# Interpretation of 20 us

Experiment 09 does **not** establish that:

```text
20 us
```

is the exact minimum reliable settling interval.

The tested delay sequence jumped from:

```text
10 us
```

to:

```text
20 us
```

and the actual transition lies somewhere within that range for the most
difficult tested conditions.

However, finding the exact boundary may not be necessary for the product.

At 20 us, the complete 21-pair scan still averages only about:

```text
128 us
```

or approximately:

```text
7,800 complete scans per second
```

This is already far faster than Sentinel is likely to require for fencing
scoring.

A production timing value should include electrical margin rather than operate
at the exact experimentally determined failure boundary.

Therefore 20 us is currently a strong candidate for a deliberately
conservative explicit drive-settling value.

It should not yet be treated as an immutable production specification.

---

# Performance Implication

The original scanner investigation began partly because the historical fencing
apparatus used approximately:

```text
1 ms
```

of settling after each driven-line transition.

A six-phase complete scan using that historical timing would spend
approximately:

```text
6 ms
```

in explicit drive settling alone.

Experiment 09 demonstrates that the tested ESP32-S3 / 3.9 kΩ interface can
produce reliable complete 21-pair maps with the tested 20 us explicit settling
point while averaging approximately:

```text
128 us
```

per complete scan.

There is therefore no current performance reason to compromise Sentinel's full
21-pair electrical measurement architecture.

---

# Findings

Experiment 09 establishes that:

1. Direct ESP32-S3 GPIO register access successfully implements the complete
   Sentinel 21-pair continuity scanner.

2. All 32 recorded topologies established valid trusted references.

3. A single GPIO input-register read can successfully capture all seven
   continuity-line states for each driven phase.

4. Removing Arduino GPIO overhead dramatically reduces scanner execution time.

5. The zero-explicit-delay complete scan dropped to approximately 6.09 us on
   average.

6. The Arduino implementation had been providing substantial implicit
   electrical settling time.

7. Explicit drive settling of 0 through 5 us was broadly insufficient for the
   tested direct-register implementation.

8. A 10 us explicit delay was much improved but still produced failures in 18
   of 96 tested topology/order conditions.

9. The tested 20 us and 50 us conditions completed without observed errors
   across the complete 32-topology / three-order dataset.

10. The first universally error-free tested point was 20 us.

11. At 20 us, the scanner still averaged approximately 7,800 complete 21-pair
    continuity maps per second.

12. The complete 21-pair measurement architecture remains comfortably viable
    on the ESP32-S3.

---

# Current Engineering Interpretation

Experiment 09 resolves an important ambiguity left by Experiment 08.

The scanner's settling requirement should not be inferred from an explicit
software delay alone.

The relevant electrical interval includes all execution time between:

```text
driving the selected line LOW
```

and:

```text
capturing the sense-line state
```

Arduino GPIO operations made that interval much longer than the configured
delay suggested.

Direct register access makes the interval substantially more explicit.

This is preferable for Sentinel because the hardware implementation can now
provide a deliberate timing guarantee instead of relying on incidental
framework overhead.

---

# Recommended Direction

The direct-register GPIO implementation is a strong candidate for promotion
into Sentinel's ESP32-S3 hardware layer.

A conservative explicit drive-settling value of:

```text
20 us
```

is supported by the current Experiment 09 dataset.

Before declaring 20 us a permanent production specification, future work may
consider:

- additional hardware units,
- longer-duration testing,
- additional representative fencing hardware,
- temperature and supply-voltage variation,
- eventual custom-PCB electrical behavior.

However, determining the exact minimum reliable delay between 10 and 20 us is
not currently necessary for scanner performance.

The existing 20 us tested point already provides complete scan rates far above
the likely needs of the game engine.

---

# Relationship to the Next Development Phase

Experiments 01 through 09 progressively moved Sentinel from an uncertain
historical timing requirement toward a characterized and deterministic
ESP32-S3 scanner implementation.

The progression is now approximately:

```text
historical 1 ms empirical delay
        ↓
basic ESP32 settling characterization
        ↓
full 21-pair scan
        ↓
drive vs. release settling
        ↓
scan-order characterization
        ↓
GPIO reassignment
        ↓
external pull-up characterization
        ↓
ESP32-S3 hardware baseline
        ↓
direct-register GPIO characterization
```

The scanner is now approaching the point where continued timing optimization
offers diminishing product value.

The next major architectural work should therefore consider promoting the
proven ESP32-S3 scanner implementation into the hardware layer and moving
upward toward Sentinel's game engine rather than continuing to optimize the
scanner solely for maximum speed.

---

# Conclusion

Experiment 09 successfully replaced Arduino per-measurement GPIO operations
with direct ESP32-S3 register access while preserving Sentinel's complete
canonical 21-pair continuity model.

The change reduced zero-delay complete-scan execution time to approximately
6 us and exposed the electrical settling interval that Arduino overhead had
previously obscured.

The tested direct-register implementation was broadly unreliable at very short
explicit delays, improved substantially by 10 us, and completed every recorded
condition without observed error at 20 us and 50 us.

The 20 us tested point produced approximately 7,800 complete electrical
snapshots per second while remaining error-free across the 32-topology,
three-scan-order dataset.

These results support a deliberate, explicit settling interval rather than
reliance on incidental software timing.

No change to Sentinel's processor-independent 21-pair continuity architecture
is indicated.

The direct-register ESP32-S3 implementation is now a strong candidate for the
production hardware layer.