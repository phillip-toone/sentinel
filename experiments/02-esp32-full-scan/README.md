# Experiment 02 — ESP32 Full Scan

## Purpose

Experiment 01 demonstrated that repeated measurement of a single ESP32 drive/sense pair did not require an explicit settling delay under the tested conditions.

However, that experiment did not reproduce Sentinel's proposed complete continuity scanner.

Experiment 02 was created to answer a broader question:

> Can an ESP32 repeatedly perform Sentinel's complete six-phase, 21-pair continuity scan with no explicit settling delay?

The experiment also measured the execution time of a complete scan using ordinary Arduino GPIO functions.

---

## Scanner Configuration

The experiment used seven logical Sentinel lines:

```text
RA
RB
RC
MT
GC
GB
GA
```

mapped to the following GPIOs on a TTGO T-Display V1.1:

| Line | GPIO |
| ---- | ---: |
| RA   |   21 |
| RB   |   22 |
| RC   |   17 |
| MT   |   32 |
| GC   |   25 |
| GB   |   26 |
| GA   |   27 |

These assignments were selected for experimental convenience and are not intended to define permanent Sentinel hardware assignments.

---

## Measurement Method

All seven lines normally idle as:

```text
INPUT_PULLUP
```

During each measurement phase, one line is changed to:

```text
OUTPUT LOW
```

The remaining lines are sampled digitally.

A sensed LOW indicates continuity with the currently driven line.

After the measurements for that phase are complete, the driven line is returned to:

```text
INPUT_PULLUP
```

No explicit settling delay was used.

---

## Six-Phase Scan

A complete scan consists of six driven-line phases:

```text
Drive RA -> sense RB, RC, MT, GC, GB, GA
Drive RB -> sense RC, MT, GC, GB, GA
Drive RC -> sense MT, GC, GB, GA
Drive MT -> sense GC, GB, GA
Drive GC -> sense GB, GA
Drive GB -> sense GA
```

GA does not need to be driven because every unique pair involving GA has already been measured during an earlier phase.

The six phases produce all 21 unique unordered line-pair measurements.

---

## Canonical 21-Bit Representation

The resulting snapshot uses Sentinel's canonical ordering:

|  Bit | Pair  |
| ---: | ----- |
|    0 | RA-RB |
|    1 | RA-RC |
|    2 | RA-MT |
|    3 | RA-GC |
|    4 | RA-GB |
|    5 | RA-GA |
|    6 | RB-RC |
|    7 | RB-MT |
|    8 | RB-GC |
|    9 | RB-GB |
|   10 | RB-GA |
|   11 | RC-MT |
|   12 | RC-GC |
|   13 | RC-GB |
|   14 | RC-GA |
|   15 | MT-GC |
|   16 | MT-GB |
|   17 | MT-GA |
|   18 | GC-GB |
|   19 | GC-GA |
|   20 | GB-GA |

This ordering is defined formally in:

```text
docs/specifications/ELECTRICAL_MODEL.md
```

---

## Test Method

For each physical topology, the experiment:

1. Performed one scan to determine an initial map.
2. Performed another scan at the beginning of the stability test and treated that result as the expected map.
3. Repeated the complete scan 1,000,000 times.
4. Counted scans whose result differed from the expected map.
5. Recorded the first differing map.
6. Measured the average execution time of a complete scan.

The experiment used no explicit settling delay between driving a line LOW and sampling the remaining lines.

---

## Physical Configurations

Testing progressed from short jumper wiring to realistic fencing cabling.

Raw results are preserved under:

```text
results/
```

The result filenames encode the physical topology that was intentionally configured using the canonical 21-bit representation.

For example:

```text
000000000000000000000.txt
```

represents a topology in which no continuity relationships were intentionally present.

The test groups include:

```text
00-short_jumper_baseline/
01-floorCords+reels+bodyCables/
02-floorCords/
```

---

## Short-Jumper Results

With short jumper wiring, the full scanner was highly stable.

The tested short-jumper topologies completed:

```text
1,000,000 scans
```

without observed instability.

This included both sparse topologies and the topology in which all 21 continuity relationships were present.

The complete scan required approximately:

```text
325 us
```

using ordinary Arduino operations such as:

```cpp
pinMode(...)
digitalWrite(...)
digitalRead(...)
```

with no explicit settling delay.

This corresponds to approximately:

```text
3,000 complete electrical snapshots per second
```

before further GPIO optimization.

---

## Long-Cable Results

The behavior changed substantially when realistic fencing cabling was introduced.

Configurations involving floor cords, reels, and body cables produced unstable maps when the scanner ran with no explicit settling delay.

Some configurations produced errors in nearly every scan, while others produced less frequent instability.

The observed erroneous maps were often structured rather than random.

For example, apparent continuity could appear among groups of related lines rather than as isolated arbitrary bit corruption.

This demonstrated that the behavior of the complete scanner depends on the external electrical system and cannot be inferred solely from ESP32 GPIO switching speed.

---

## Important Methodological Limitation

Experiment 02 contained a significant flaw in the way it established the expected map.

The stability test used:

```cpp
const uint32_t expected = scanContinuity();
```

This means that one scan performed with the same zero-delay measurement method under investigation was promoted to the expected result.

That is not a reliable ground truth when the scanner itself is unstable.

In some raw result files, the topology encoded in the filename disagrees with the program's reported initial or expected map.

For example, a filename representing:

```text
000000000000000000000
```

could contain an observed map with several continuity bits set.

This disagreement is itself useful evidence of scanner instability.

However, it also means that the numerical `Errors:` values recorded by Experiment 02 do **not** necessarily represent the number of scans that disagreed with the actual physical topology.

They represent:

> The number of scans that disagreed with one particular scan that the program selected as its reference.

Therefore the Experiment 02 error counts must not be treated as quantitative error rates relative to known physical truth.

---

## What Experiment 02 Reliably Established

Despite the reference-method flaw, Experiment 02 established several important observations.

### The Complete Scan Is Computationally Practical

A complete 21-pair scan using ordinary Arduino GPIO operations required approximately hundreds of microseconds rather than milliseconds when no explicit settling delay was added.

This demonstrated that the computational work of performing the complete scan is not inherently prohibitive.

### Short Wiring Is Not Representative

The scanner was stable with short jumper wiring but became unstable when realistic fencing cabling was introduced.

Therefore successful bench testing with short wires is insufficient to characterize the electrical behavior of the scoring apparatus.

### External Wiring Matters

Floor cords, reels, body cables, connectors, and related external wiring materially affect measurement behavior.

The limiting factor is therefore not simply the switching speed of the ESP32 GPIO peripheral.

### Zero Explicit Delay Is Not Universally Reliable

The complete scanner cannot safely assume that the natural execution time of Arduino GPIO operations provides sufficient settling time under all fencing configurations.

---

## Working Hypotheses

The structured errors observed with longer cabling are consistent with electrical history or settling effects between measurement phases.

Possible contributors include:

* cable capacitance
* cable length
* reel wiring
* connector resistance
* contact resistance
* internal pull-up characteristics
* interaction between consecutive driven-line phases

These are hypotheses.

Experiment 02 did not establish the physical mechanism responsible for the errors.

---

## Why Experiment 03 Was Required

Experiment 02 demonstrated the need for a more rigorous reference method.

A useful characterization experiment must compare short-delay scans against a reference that is established independently of the short-delay behavior being tested.

Experiment 03 therefore changed the methodology to:

1. Use the historically reliable 1 ms settling interval.
2. Perform many slow reference scans.
3. Require those reference scans to agree.
4. Establish the resulting map as the trusted reference.
5. Test progressively shorter settling intervals against that reference.
6. Distinguish false-positive from false-negative continuity measurements.
7. Record errors by individual canonical line pair.

That became:

```text
03-esp32-full-scan-characterization
```

---

## Findings

Experiment 02 established that:

1. The complete six-phase / 21-pair scan is fast enough to warrant continued investigation.
2. Zero explicit settling delay can work extremely well with short wiring.
3. Realistic fencing cables can make the same zero-delay scanner highly unstable.
4. The instability is structured enough to justify investigating scan history and electrical settling rather than treating it as random digital noise.
5. The original self-referenced stability-test methodology cannot provide trustworthy quantitative error rates.
6. A known-good reference methodology is required before settling-time requirements can be characterized.

---

## Status

**Complete**

Experiment 02 successfully demonstrated the feasibility of the full scanner and exposed the cable-dependent instability that required more rigorous characterization.

Its raw observations remain valuable.

Its reported `Errors:` counts should **not** be interpreted as ground-truth error rates because of the self-referenced expected-map methodology.

Experiment 03 supersedes Experiment 02 for quantitative settling-time characterization.
