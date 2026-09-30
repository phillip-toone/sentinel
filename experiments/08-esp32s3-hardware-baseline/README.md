# Experiment 08 — ESP32-S3 Hardware Baseline

## Purpose

This experiment established a continuity-scanner baseline on the intended
Sentinel production development platform:

```text
LILYGO T-Display S3
ESP32-S3
```

Experiments 01 through 07 were performed primarily on a classic ESP32-based
TTGO T-Display.

Experiment 08 asks whether the electrical interface and complete 21-pair
continuity scanner developed during those experiments transfer successfully to
the ESP32-S3 while retaining the existing Arduino GPIO implementation.

This experiment intentionally does **not** introduce direct-register GPIO
access.

The objective is to change the processor/board and GPIO assignment while
preserving the proven scanner methodology as closely as practical.

---

## Hardware

Board:

```text
LILYGO T-Display S3
ESP32-S3
```

The experiment used realistic fencing:

```text
floor cords + reels
```

External pull-ups:

```text
3.9 kΩ
```

were installed on all seven continuity lines.

ESP32 internal pull-ups were not used.

The buzzer was not part of this experiment.

---

## GPIO Assignment

Experiment 08 used:

| Sentinel line | GPIO |
|---|---:|
| RA | 1 |
| RB | 2 |
| RC | 10 |
| MT | 16 |
| GC | 11 |
| GB | 12 |
| GA | 13 |

The proposed buzzer assignment is:

```text
BZ = GPIO21
```

but BZ was not exercised during Experiment 08.

All seven continuity GPIOs are in the ESP32-S3 GPIO0–31 bank.

This is useful for future direct-register implementations because the physical
state of all seven continuity lines can potentially be captured with one
32-bit GPIO input-register read.

---

## Preserved Board Resources

The selected GPIO mapping intentionally avoids several resources that may be
valuable to Sentinel later.

In particular, the mapping preserves:

```text
GPIO17 / GPIO18
    I²C

GPIO43 / GPIO44
    UART / general expansion
```

The mapping also avoids ESP32-S3 strapping pins for the seven continuity
signals.

GPIO11, GPIO12, and GPIO13 are used by Sentinel rather than being retained for
the optional T-Display S3 SD-card interface.

Therefore optional SD-card support is one known tradeoff of this development
board pin assignment.

The non-touch T-Display S3 was the baseline for this experiment.

---

## Electrical Measurement Method

The scanner retained the active-low measurement method developed in the
previous experiments.

Released and sensed lines are configured as:

```cpp
INPUT
```

rather than:

```cpp
INPUT_PULLUP
```

External 3.9 kΩ resistors provide the pull-ups.

For each measurement phase:

```text
selected line
    ↓
OUTPUT LOW
    ↓
drive-to-sample settling
    ↓
read other lines
    ↓
selected line released to INPUT
```

A connected sense line is expected to read LOW.

An unconnected line is expected to return HIGH through its external pull-up.

Explicit release settling remained:

```text
0 us
```

---

## Canonical Continuity Map

Sentinel represents the seven logical electrical lines as 21 unique unordered
continuity relationships.

Physical scan order does not determine canonical bitmap position.

For example:

```text
drive RA -> sense RC
```

and:

```text
drive RC -> sense RA
```

both represent the canonical relationship:

```text
RA-RC
```

Experiment 08 retained this separation between physical measurement order and
logical continuity representation.

---

## Scan Orders

Three scan orders were retained as diagnostic stress conditions.

### Forward

```text
RA -> RB -> RC -> MT -> GC -> GB -> GA
```

### Reverse

```text
GA -> GB -> GC -> MT -> RC -> RB -> RA
```

### Interleaved

```text
RA -> GA -> RB -> GB -> RC -> GC -> MT
```

These orders are not being evaluated as candidate game rules or different
logical scanners.

They provide different electrical histories while producing the same canonical
21-bit continuity map.

---

## Manual Start

After startup and serial initialization, the program waits for the operator to
press a key before beginning the experiment.

This allows the serial monitor and result capture to be established before the
trusted-reference and characterization output begins.

The manual-start wait occurs before electrical characterization and does not
alter the scanner measurement timing.

## Trusted Reference

Before characterization, the program established a conservative trusted
reference using:

```text
Scan order:          Forward
Drive settling:      1000 us
Release settling:    1000 us
Reference scans:     1000
```

All 1,000 reference scans were required to agree before characterization
continued.

All 28 recorded Experiment 08 topology files produced:

```text
Reference scans agreeing: 1000 / 1000
Reference status: VALID
```

---

## Characterization Conditions

Each topology was tested using all three scan orders.

The explicit drive-to-sample settling intervals were:

```text
0 us
1 us
2 us
5 us
10 us
20 us
50 us
100 us
```

Explicit release settling remained:

```text
0 us
```

Each scan-order / delay condition performed:

```text
10,000 complete scans
```

Therefore each topology contained:

```text
3 scan orders
×
8 settling intervals
=
24 characterization conditions
```

---

## Tested Topologies

Experiment 08 recorded:

```text
28 topology files
```

using floor cords and reels.

The topology is encoded directly in each result filename using Sentinel's
canonical 21-bit representation.

The test set included sparse, intermediate, difficult, and fully connected
electrical configurations.

Among the included configurations were the previously important topology:

```text
100000000000000000001
```

and the fully connected topology:

```text
111111111111111111111
```

The complete raw serial output for every topology is preserved under:

```text
results/
```

---

# Results

## 1 us and Greater

The strongest result from Experiment 08 is:

> Every recorded condition at 1 us explicit drive settling or greater
> completed all 10,000 scans without observed error.

At the 1 us point alone, this represents:

```text
28 topologies
×
3 scan orders
×
10,000 scans
=
840,000 complete scans
```

with no observed errors.

The tested:

```text
2 us
5 us
10 us
20 us
50 us
100 us
```

conditions also completed without observed errors.

Therefore no recorded Experiment 08 failure remained once the explicit
drive-to-sample settling interval reached the tested 1 us point.

---

## Zero Explicit Delay

Zero explicit drive settling was very close to reliable but was not completely
error-free.

Eighteen of the 28 topologies passed all three scan orders at zero explicit
delay.

Ten topologies produced at least one zero-delay failure.

The observed failing zero-delay conditions were:

| Topology | Scan order | Incorrect scans |
|---|---|---:|
| `000000000000010000101` | Reverse | 2 / 10,000 |
| `000000000000100001001` | Reverse | 27 / 10,000 |
| `000010000000000000001` | Interleaved | 1 / 10,000 |
| `100000000000000000000` | Interleaved | 1 / 10,000 |
| `100000000000000000001` | Interleaved | 5 / 10,000 |
| `100000000000100001001` | Reverse | 8 / 10,000 |
| `100000000011000000000` | Interleaved | 5 / 10,000 |
| `100000110000000000000` | Forward | 10 / 10,000 |
| `100000110000000000000` | Interleaved | 1 / 10,000 |
| `100000110000000000001` | Forward | 14 / 10,000 |
| `100000110000000000001` | Reverse | 2 / 10,000 |
| `100000110000000000001` | Interleaved | 3 / 10,000 |
| `100110000000000000000` | Forward | 12 / 10,000 |

The largest observed zero-delay failure count was:

```text
27 / 10,000
```

All of these failing configurations passed without observed error at the next
tested settling interval:

```text
1 us
```

---

## Previously Difficult Topology

The topology:

```text
100000000000000000001
```

had been particularly useful in earlier classic-ESP32 experiments because it
exposed marginal settling behavior.

On the T-Display S3 with 3.9 kΩ external pull-ups:

```text
Forward, 0 us:
    PASS

Reverse, 0 us:
    PASS

Interleaved, 0 us:
    5 / 10,000 incorrect
```

At:

```text
1 us
```

and every larger tested delay, all three orders completed without observed
error.

This shows that the previously difficult topology remains useful as a stress
condition, but the ESP32-S3 / 3.9 kΩ interface performs substantially inside
the useful timing range.

---

## Fully Connected Topology

The fully connected topology:

```text
111111111111111111111
```

completed every tested scan order and settling interval without observed
error, including:

```text
0 us
```

This topology also represents a high-current normal scanner condition because
the driven LOW line can sink current through multiple external pull-ups.

No measurement reliability problem was observed in this condition.

---

# Findings

Experiment 08 established that:

1. The complete Sentinel 21-pair continuity scanner operates successfully on
   the LILYGO T-Display S3 / ESP32-S3.

2. The selected GPIO assignment works with the existing Arduino-based scanner.

3. The 3.9 kΩ external pull-up interface transfers successfully from the
   classic ESP32 experiments to the ESP32-S3 platform.

4. All 28 recorded topologies established valid 1000/1000 trusted references.

5. All recorded conditions at 1 us explicit drive settling or greater
   completed 10,000 scans without observed error.

6. Zero explicit drive settling is close to reliable but remains measurably
   marginal for some topology / scan-order combinations.

7. The observed transition between marginal and error-free operation occurred
   between the tested 0 us and 1 us explicit-delay points.

8. Forward, Reverse, and Interleaved orders all operate reliably once
   sufficient settling is provided.

9. The full 21-pair measurement architecture remains viable on the intended
   ESP32-S3 development platform.

---

# Important Interpretation of Zero Delay

A configured drive-settling interval of:

```text
0 us
```

does not mean the electrical system receives zero settling time.

Experiment 08 still uses Arduino GPIO operations such as:

```cpp
pinMode()
digitalWrite()
digitalRead()
```

as well as ordinary loop and function-call execution.

These operations consume finite processor time.

The complete zero-explicit-delay scans still required approximately hundreds
of microseconds, with typical complete scan times around the high-170 to
low-180 us range.

Therefore the successful and near-successful zero-delay results include
implicit settling produced by software execution overhead.

This distinction is particularly important for the next scanner
characterization.

---

# Why Direct Register Access Is Not Part of Experiment 08

The ESP32-S3 provides lower-level GPIO register access that can perform GPIO
operations much faster and more deterministically than the Arduino functions
used here.

All seven Experiment 08 continuity lines are located in the GPIO0–31 bank:

```text
GPIO1   RA
GPIO2   RB
GPIO10  RC
GPIO11  GC
GPIO12  GB
GPIO13  GA
GPIO16  MT
```

This makes it possible for a future implementation to capture the physical
state of all seven continuity lines with one GPIO input-register read.

However, introducing direct-register access during Experiment 08 would have
changed both:

```text
processor / board
```

and:

```text
GPIO implementation
```

simultaneously.

Experiment 08 therefore intentionally retained the Arduino GPIO
implementation so the ESP32-S3 migration could be characterized independently.

---

# Next Investigation

The next scanner experiment should investigate direct ESP32-S3 GPIO register
access while holding the established hardware conditions constant.

A future Experiment 09 should retain:

```text
LILYGO T-Display S3
same GPIO assignment
3.9 kΩ external pull-ups
same canonical 21-pair scanner
same trusted-reference methodology
representative floor-cord + reel topologies
```

while changing:

```text
Arduino GPIO access
        ↓
direct ESP32-S3 register access
```

The important question will be:

> How much explicit drive-to-sample settling is required when Arduino GPIO
> call overhead is removed?

A direct-register implementation may require an explicit settling interval
even when the Arduino implementation appears reliable with little or no
additional delay.

That would not represent a regression.

It would replace accidental software timing with a deliberate and measurable
electrical timing parameter.

---

# Conclusion

Experiment 08 successfully established the LILYGO T-Display S3 as a viable
Sentinel continuity-scanner platform.

The complete 21-pair scanner, selected GPIO mapping, and 3.9 kΩ external
pull-up interface operated reliably across a broad 28-topology floor-cord +
reel test set once the explicit drive-to-sample delay reached the tested
1 us point.

Zero explicit delay was nearly reliable but produced a small number of
repeatable failures in several topology / scan-order combinations.

This narrow 0-to-1 us transition provides a useful baseline for the next stage
of scanner development.

The next optimization should therefore focus on replacing Arduino GPIO
operations with deterministic direct-register access and then measuring the
explicit settling interval required by that faster implementation.

No change to Sentinel's processor-independent 21-pair continuity architecture
is indicated by Experiment 08.