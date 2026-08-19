# Experiment 03 — ESP32 Full-Scan Characterization

## Purpose

Experiment 02 demonstrated that Sentinel's complete six-phase / 21-pair scan was computationally practical but could become unstable when realistic fencing cabling was connected.

Experiment 02 also exposed a flaw in its test methodology: the scanner used one zero-delay scan as the expected result and then compared subsequent zero-delay scans against it.

Experiment 03 was created to correct that problem and answer a more rigorous question:

> How much explicit settling time is required for Sentinel's complete 21-pair scan to reproduce a trusted electrical topology reliably when realistic fencing cabling is attached?

---

## Relationship to Previous Experiments

### Experiment 01

```text
01-esp32-settling-time
```

tested one drive/sense pair repeatedly.

It demonstrated that no explicit delay was required for the tested two-pin configurations, but it did not reproduce the rapidly changing drive sequence of the complete scanner.

### Experiment 02

```text
02-esp32-full-scan
```

implemented the complete six-phase scanner.

It demonstrated that:

* the full scan was computationally practical;
* short jumper wiring was stable without an explicit delay;
* realistic fencing cabling introduced substantial instability;
* a self-referenced zero-delay scan was not a trustworthy ground truth.

Experiment 03 therefore retained the full scanner but replaced the reference methodology.

---

## Hardware

Testing used a TTGO T-Display V1.1 based on the classic ESP32.

The experimental Sentinel line mapping was:

| Line | GPIO |
| ---- | ---: |
| RA   |   21 |
| RB   |   22 |
| RC   |   17 |
| MT   |   32 |
| GC   |   25 |
| GB   |   26 |
| GA   |   27 |

These GPIO assignments were selected for the experiment and do not define permanent Sentinel hardware assignments.

---

## Electrical Measurement Method

The experiment retained the active-low measurement method used by the existing scoring apparatus.

All lines normally idle as:

```text
INPUT_PULLUP
```

During each measurement phase, one selected line becomes:

```text
OUTPUT LOW
```

The remaining lines are sampled digitally.

A sensed LOW represents continuity with the driven line.

After the phase is complete, the driven line returns to:

```text
INPUT_PULLUP
```

---

## Complete Scan Sequence

A complete electrical snapshot requires six driven-line phases:

```text
Drive RA -> sense RB, RC, MT, GC, GB, GA
Drive RB -> sense RC, MT, GC, GB, GA
Drive RC -> sense MT, GC, GB, GA
Drive MT -> sense GC, GB, GA
Drive GC -> sense GB, GA
Drive GB -> sense GA
```

This measures all 21 unique unordered line pairs.

The resulting map uses the canonical ordering defined by:

```text
docs/specifications/ELECTRICAL_MODEL.md
```

---

## Reference Method

Experiment 03 does not ask the short-delay scanner to determine its own expected result.

Instead, each run first establishes a conservative reference topology.

The reference procedure uses:

```text
Settling time per driven phase: 1000 us
Reference scans:                1000
```

The first slow scan becomes a candidate reference.

Another 999 scans are then performed using the same 1 ms settling interval.

The reference is accepted only if:

```text
1000 / 1000 scans agree exactly
```

If any reference scan disagrees, characterization is aborted.

This approach is based on the historical scoring implementation, where a 1 millisecond delay was empirically known to provide reliable operation.

The slow reference is therefore used as a conservative measurement procedure rather than assuming that a particular manually entered bitmap is correct.

---

## Reference Validation

All physical configurations tested during Experiment 03 successfully established a stable reference.

For every recorded run:

```text
Reference scans agreeing: 1000 / 1000
Reference status: VALID
```

This provided a stable reference against which the shorter settling intervals could be compared.

---

## Settling-Time Sweep

After establishing the reference, the experiment automatically tested:

```text
0 us
1 us
2 us
5 us
10 us
20 us
50 us
100 us
250 us
500 us
```

Each settling interval was tested for:

```text
10,000 complete scans
```

Each complete scan contained all six driven-line phases and all 21 canonical continuity measurements.

---

## Error Classification

Every short-delay scan was compared against the trusted reference map.

The experiment recorded:

* incorrect complete scans;
* scans containing false-positive continuity;
* scans containing false-negative continuity;
* the first erroneous map;
* a difference mask;
* false-positive counts for each canonical pair;
* false-negative counts for each canonical pair;
* average complete-scan time;
* complete scans per second.

This allowed Experiment 03 to characterize not only whether a scan failed, but how it failed.

---

## Physical Configurations

Testing included configurations using:

* floor cords;
* floor cords with reels;
* body-cable configurations;
* sparse continuity topologies;
* more highly connected topologies;
* the fully connected 21-bit topology.

The raw serial output from each run is preserved under:

```text
results/
```

The filenames encode the intentionally configured canonical topology.

---

## Observed Settling Behavior

The minimum **tested** settling interval that produced zero errors in 10,000 scans varied with both the physical cable configuration and electrical topology.

Observed results included:

| Physical configuration | Reference map | First tested delay with 0 errors in 10,000 scans |
| ---------------------- | ------------: | -----------------------------------------------: |
| Floor cords            |    `0x000000` |                                            20 us |
| Floor cords            |    `0x000110` |                                            10 us |
| Floor cords            |    `0x100001` |                                            50 us |
| Floor cords            |    `0x1FFFFF` |                                             0 us |
| Floor cords + reels    |    `0x000000` |                                            50 us |
| Floor cords + reels    |    `0x000410` |                                            50 us |
| Floor cords + reels    |    `0x040040` |                                           100 us |
| Floor cords + reels    |    `0x040400` |                                            50 us |
| Floor cords + reels    |    `0x100001` |                                           100 us |
| Floor cords + reels    |    `0x1FFFFF` |                                             0 us |

The worst tested configurations therefore required a tested interval of:

```text
100 us
```

before completing 10,000 scans without an observed error.

This does **not** establish 100 us as Sentinel's production settling-time requirement.

It establishes only that:

> Under the tested configurations, some intervals below 100 us produced errors while 100 us produced no errors in the corresponding 10,000-scan tests.

The actual transition threshold and appropriate engineering margin remain to be determined.

---

## Effect of Cabling

The experiment confirmed that external fencing wiring materially affects the required settling behavior.

Adding reels generally increased the settling interval required to eliminate observed errors.

This reinforces the conclusion from Experiment 02 that bench testing with short jumper wires is not representative of the complete fencing apparatus.

The relevant electrical system includes more than the ESP32 GPIO itself.

Potential contributors include:

* cable length;
* cable capacitance;
* reel wiring;
* connector resistance;
* contact resistance;
* internal pull-up characteristics;
* interaction between consecutive measurement phases.

Experiment 03 did not isolate which of these mechanisms dominates.

---

## Effect of Electrical Topology

The required settling interval also depended strongly on the electrical topology being measured.

Some sparse configurations required tens or hundreds of microseconds of explicit settling before errors disappeared.

In contrast, the fully connected topology:

```text
111111111111111111111
```

completed 10,000 scans without observed errors even at:

```text
0 us
```

for both the floor-cord and floor-cord-plus-reel configurations tested.

This is an important observation.

When every canonical pair is expected to indicate continuity, residual electrical state cannot create an additional false-positive continuity bit because every bit is already expected to be set.

Sparse topologies provide substantially more opportunity for transient electrical state to appear as false continuity.

---

## Error Character

At insufficient settling intervals, observed errors were predominantly associated with apparent continuity that should not have been present.

This behavior is consistent with transient electrical state, residual charge, capacitive coupling, or other history-dependent effects between measurement phases.

However:

> Experiment 03 did not prove the physical mechanism responsible for the errors.

Cable capacitance and scan-history effects remain working hypotheses rather than established causes.

---

## Full-Scan Performance

Experiment 03 also demonstrated that the complete 21-pair scan remains computationally practical even when explicit settling time is introduced.

At approximately:

```text
100 us settling per driven phase
```

a complete six-phase scan required approximately:

```text
786 us
```

in the observed tests.

This corresponds to approximately:

```text
1,270 complete electrical snapshots per second
```

using ordinary Arduino GPIO operations.

No direct-register GPIO optimization was required to achieve this rate.

This is significant because the original historical implementation used approximately:

```text
6 ms
```

of explicit settling time alone for six measurement phases when using a 1 ms delay per phase.

Experiment 03 therefore provides evidence that the full 21-pair scanner may remain practical even with a substantial electrical settling margin.

---

## Historical 1 ms Delay

The original scoring apparatus used:

```cpp
delay(1);
```

after selecting a driven line.

Experiment 03 suggests that this value was conservative relative to the tested configurations.

The worst tested configurations reached zero observed errors at a tested delay of 100 us, one tenth of the historical delay.

However, the historical 1 ms value should not be considered incorrect.

It was selected empirically to produce reliable operation before systematic characterization was available.

Experiment 03 provides a basis for replacing that conservative value with a measured requirement and an intentional engineering margin.

---

## What Experiment 03 Established

Experiment 03 established that:

1. The historical 1 ms settling interval provides a stable reference for every physical configuration tested.
2. The full 21-pair scan remains viable and does not need to be abandoned based on the historical 1 ms delay.
3. Zero explicit settling delay is not reliable for all realistic fencing configurations.
4. Required settling behavior depends on external cabling.
5. Required settling behavior also depends on electrical topology.
6. Reels can increase the settling requirement.
7. The worst tested configurations required a tested delay of 100 us before completing 10,000 scans without an observed error.
8. Fully connected topologies can behave differently from sparse topologies.
9. The observed error pattern is consistent with history-dependent electrical settling, but the physical mechanism has not yet been established.
10. Ordinary Arduino GPIO operations are already fast enough to make the complete scanner worthy of continued development.

---

## What Experiment 03 Did Not Establish

Experiment 03 did **not** establish:

* that 100 us is the minimum reliable settling time;
* that 100 us is an appropriate production value;
* the required safety margin;
* the dominant physical cause of the settling behavior;
* whether drive settling or release settling is more important;
* whether scan order affects the errors;
* whether errors follow logical lines or physical GPIO assignments;
* whether direct-register GPIO access would materially improve the system;
* whether a different electrical input design would change the settling requirement.

These remain open engineering questions.

---

## Questions Raised for Further Testing

The Experiment 03 results suggest several useful next questions.

### Drive Settling vs. Release Settling

The current scanner waits only after driving the next line LOW.

It does not intentionally provide a neutral interval after releasing the previous driven line.

The relevant timing may therefore involve two different quantities:

```text
Drive line LOW
      |
      v
drive settling
      |
      v
sample
      |
      v
release line
      |
      v
release settling
      |
      v
drive next line LOW
```

Experiment 03 did not separate these effects.

### Scan Order

The scanner currently drives lines in this fixed order:

```text
RA -> RB -> RC -> MT -> GC -> GB
```

Structured errors observed around particular line groups raise the possibility that measurement history or scan position influences the result.

Reversing or permuting the scan order while leaving the physical topology unchanged could help determine whether errors follow scan history.

### GPIO Assignment

If an error remains associated with the same logical line after changing its physical GPIO, the external topology becomes a stronger suspect.

If the error follows the GPIO instead, processor or board-level behavior becomes more likely.

Experiment 03 did not vary GPIO assignment.

---

## Direction After Experiment 03

The results do not justify abandoning Sentinel's complete 21-pair electrical snapshot.

Instead, they justify further characterization of the timing behavior.

The next experiment should change one variable at a time and investigate whether the observed settling requirement is primarily associated with:

* drive settling;
* release settling;
* scan order;
* physical GPIO assignment;
* or the external electrical topology itself.

Production optimization should remain secondary until these effects are better understood.

The guiding principle remains:

> **Measure first. Optimize from evidence.**

---

## Status

**Complete**

Experiment 03 corrected the reference flaw discovered in Experiment 02 and produced the first quantitatively useful characterization of Sentinel's full scanner under realistic fencing cabling.

Its results support continued development of the complete 21-pair scanner while motivating targeted investigation of scan-history and settling behavior before selecting a production timing value.
