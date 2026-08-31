# Sentinel Design Rationale

## Purpose

Specifications describe **what** Sentinel is.

The implementation describes **how** Sentinel works.

This document explains **why** Sentinel was designed the way it is.

Engineering decisions are rarely obvious years later. This document exists to
preserve the reasoning behind those decisions so future contributors,
including future versions of ourselves, understand the intent behind the
architecture.

This document is intentionally narrative rather than normative.

---

# Design Philosophy

Sentinel is developed using an architecture-first engineering process.

The preferred order of development is:

```text
Vision

↓

Architecture

↓

Specification

↓

Implementation

↓

Testing

↓

Optimization
```

This intentionally delays implementation until the problem has been clearly
defined.

The goal is not to avoid refactoring.

The goal is to ensure that refactoring improves implementation rather than
silently changing the underlying design.

Hardware characterization has reinforced the value of this approach.

When realistic fencing cabling initially made the complete continuity scan
appear potentially too slow or unreliable, Sentinel did not immediately
simplify the measurement architecture.

Instead, the electrical behavior was measured.

That distinction allowed an implementation problem to be investigated without
prematurely turning it into an architectural limitation.

---

# Preserve Knowledge

One of the primary goals of Sentinel is to preserve engineering knowledge.

Code alone cannot explain why architectural decisions were made.

Whenever practical, important design reasoning should be captured within the
repository rather than existing only in conversations or developer memory.

Sentinel should eventually become self-documenting.

A new engineer should be able to understand the project's evolution by reading
the repository alone.

This includes preserving experimental evidence.

The experiment directories are therefore not merely temporary development
artifacts. They preserve:

```text
question

↓

method

↓

raw evidence

↓

current interpretation
```

Later evidence may revise an interpretation without making the original
experiment useless.

That distinction is important. Engineering understanding should be allowed to
evolve while the evidence that produced earlier conclusions remains available.

---

# Separate Measurement From Interpretation

One of the most significant architectural decisions made during Sentinel's
development was separating electrical measurement from electrical
interpretation.

Originally, it was tempting to build a scanner that directly detected
touches.

Instead, Sentinel treats these as separate problems.

The Continuity Scanner performs only one task:

> Measure electrical continuity.

It does not understand:

- foil
- épée
- sabre
- scoring
- timing
- touches

Those concepts belong to higher architectural layers.

This separation makes the scanner reusable, testable, and independent of game
rules.

Hardware characterization strengthened this decision.

Electrical phenomena such as settling time, pull-up resistance, GPIO polarity,
and cable capacitance affect **how** continuity is measured.

They do not change **what continuity means** to Sentinel.

---

# Separate Logical Measurement From Physical Measurement

Sentinel's logical electrical model should not depend on the details of a
particular processor or input circuit.

The logical scanner asks questions such as:

```text
Is RA continuous with RC?
```

The physical hardware may answer that question by:

```text
drive RA LOW
sense RC
```

or, in another measurement ordering:

```text
drive RC LOW
sense RA
```

Both represent the same unordered logical relationship:

```text
RA-RC
```

This distinction became important during ESP32 characterization.

Different physical scan orders produced different transient behavior under
marginal electrical conditions even though they represented the same logical
continuity map.

Therefore:

> Measurement order is an implementation detail. Canonical continuity identity
> is an architectural concept.

The hardware layer is responsible for translating physical measurements into
the canonical logical representation.

---

# Measure Everything

An early design question asked whether the scanner should measure only the
electrical relationships required by the current rule engines.

Although this would reduce the number of measurements, it would tightly couple
the scanner to specific games.

Instead, Sentinel measures every unique electrical relationship between the
seven logical lines.

Seven lines produce 21 unique unordered pairs.

Advantages include:

- scanner independence
- simpler diagnostics
- easier replay
- easier testing
- future game support
- future rule changes
- complete electrical snapshots
- separation of measurement from interpretation

The computational cost of processing 21 measurements is small on modern
processors.

The more important concern proved to be the physical electrical settling
behavior between measurement phases.

That concern was legitimate, but subsequent hardware experiments showed that
it should be addressed in the electrical interface rather than by reducing the
logical measurement model.

---

# Preserve the Full Measurement Model

The decision to measure all 21 unique line relationships was challenged by the
behavior of the original scoring apparatus.

The historical ESP32 implementation used an active-low continuity measurement
technique:

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

Under normal conditions, lines were configured as inputs with pull-ups.

During measurement, one selected line was configured as an output and driven
LOW.

Electrical continuity caused connected lines to be pulled LOW as well.

The 1 millisecond delay in `setPIN()` was not derived from ESP32 GPIO switching
specifications.

It was retained because it produced reliable operation during development.

Earlier implementations appeared functional with less conservative timing
until realistic fencing cabling was introduced.

Long floor cords, fencing reels, and their associated connecting cables
exposed reliability problems that were not apparent with short wiring.

A complete seven-line scan requires six driven-line phases.

If every phase required the historical 1 millisecond delay, settling alone
would require approximately:

```text
6 ms
```

per complete electrical snapshot.

This made the performance concern real.

However, Sentinel deliberately retained the complete measurement model while
the physical behavior was characterized.

That decision was subsequently validated experimentally.

Experiments 01 through 07 demonstrated that the historical 1 millisecond delay
was not an inherent requirement of the complete scan.

Most significantly, external pull-up characterization demonstrated tested
configurations in which the complete scan operated across realistic floor-cord
and reel topologies without requiring any additional explicit
drive-to-sample delay.

Therefore:

> A limitation in one electrical implementation should not be promoted into a
> limitation of the logical architecture without evidence that the architecture
> itself is the problem.

The complete 21-pair measurement model remains the preferred Sentinel
architecture.

---

# Settling Is a System Property

One of the most important lessons from hardware characterization is that
electrical settling time does not belong to the GPIO alone.

The relevant system includes:

- processor GPIO characteristics
- pull-up resistance
- cable capacitance
- cable length
- reel wiring
- connector resistance
- contact resistance
- external electrical loading
- logic thresholds
- measurement sequence

The nanosecond-scale switching capability of a microcontroller GPIO therefore
does not establish the required measurement interval for the complete fencing
system.

Conversely, a historically successful millisecond-scale delay does not prove
that the electrical system actually requires a millisecond.

The correct settling interval must be established from the complete physical
system.

This leads to a broader rule:

> Do not infer system-level electrical timing from component-level switching
> specifications alone.

---

# Drive Settling and Release Settling Are Different

Early scanner implementations treated "settling time" as a single general
concept.

Hardware characterization showed that two distinct intervals should be
considered:

```text
drive line LOW
    |
    v
wait
    |
    v
sample
```

and:

```text
release previous line
    |
    v
wait
    |
    v
drive next line
```

Experiment 04 varied these independently.

For the observed failure mechanism, sufficient **drive-to-sample settling**
corrected the false-positive measurements.

Adding release-only settling through the tested range did not provide the same
benefit.

This does not prove that release behavior can never matter under any hardware
configuration.

It does establish that timing parameters should correspond to specific
physical transitions rather than being represented by an unexplained generic
delay.

---

# Scan Order Is Not the Logical Model

Experiment 05 demonstrated that Forward, Reverse, and Interleaved measurement
orders could behave very differently when the electrical interface was near
its settling limit.

In one difficult topology at a 100 microsecond drive delay, changing only the
measurement ordering changed the observed result from essentially complete
failure to complete success.

This initially made scan order appear to be a possible fundamental issue.

Later external pull-up characterization provided important context.

With sufficiently strong external pull-ups, the tested differences among scan
orders disappeared even with no additional explicit drive delay.

The resulting design lesson is:

> Scan-order sensitivity can be a symptom of marginal electrical settling
> rather than a reason to encode one particular order into the logical
> architecture.

Sentinel should therefore keep canonical continuity representation independent
of physical scan order.

A hardware implementation may choose an order for electrical or performance
reasons, but that choice should not change the meaning of a `ContinuityMap`.

---

# GPIO Assignment Is an Implementation Detail

Experiment 06 reassigned five of the seven logical Sentinel lines to different
ESP32 GPIOs.

The detailed behavior changed, but the settling phenomenon remained.

This weakened a simple explanation based solely on the original complete GPIO
assignment.

The experiment did not prove that individual GPIO characteristics or board
layout are irrelevant.

In particular, RA and RC remained on their original GPIOs during that
experiment.

The architectural lesson is narrower:

> Logical Sentinel line identity must remain independent of processor GPIO
> numbering.

GPIO assignment belongs in the hardware-specific implementation.

Future boards and processor families should be free to choose different
physical pins without changing the processor-independent scanner.

---

# Pull-Up Strength Is an Electrical Design Parameter

Experiment 07 replaced the ESP32 internal pull-up behavior with external
pull-up resistors and varied their resistance.

The tested values were:

```text
1.8 kΩ
3.9 kΩ
8.2 kΩ
15 kΩ
33 kΩ
```

The results showed a strong and systematic relationship between pull-up
resistance and the explicit settling interval required for reliable scanning.

Stronger pull-ups substantially reduced the observed settling requirement.

With 1.8 kΩ and 3.9 kΩ external pull-ups, all three tested electrical
topologies and all three tested scan orders completed the zero-explicit-delay
conditions without observed errors.

As resistance increased, progressively longer tested settling intervals were
required.

This strongly supports an electrical-settling interpretation of the earlier
false positives.

A plausible explanation is RC-like behavior involving pull-up resistance and
the capacitance of the complete wiring system.

However, Sentinel has not yet directly measured:

- cable capacitance
- transient line voltage
- logic-threshold crossing time
- waveform shape
- an equivalent RC time constant

The design rationale should therefore preserve the distinction between
**evidence** and **physical explanation**.

The evidence establishes that pull-up resistance has substantial control over
the observed behavior.

The detailed physical model remains subject to further measurement.

---

# Stronger Is Not Automatically Better

The external pull-up experiments demonstrated that lower resistance can improve
settling speed.

That does not mean the smallest practical resistance should automatically be
selected.

When continuity exists and one line is driven LOW, the output must sink current
through the pull-up network.

A production pull-up value must therefore balance:

- settling speed
- GPIO sink current
- simultaneous connected pull-ups
- cable and connector resistance
- logic-level margin
- noise susceptibility
- power consumption
- electrical protection
- processor limitations
- desired scan rate

This is an example of why experimental optimization should identify a useful
design region rather than simply maximize one measured variable.

No production pull-up resistance is selected solely by Experiment 07.

---

# Zero Explicit Delay Does Not Mean Zero Settling Time

Several external-pull-up configurations completed the tested scans with a
configured drive delay of:

```text
0 us
```

This means only that the program inserted no additional explicit
`delayMicroseconds()` before sampling.

The physical measurement still includes finite time consumed by:

- `pinMode()`
- `digitalWrite()`
- loop execution
- `digitalRead()`
- framework overhead
- processor execution

This distinction matters because future optimization may reduce that implicit
time.

A hardware implementation that replaces framework calls with direct register
operations could require an explicit delay even when the current Arduino-based
implementation does not.

Therefore:

> Software overhead must not be confused with an electrical timing guarantee.

---

# The Scanner Is a Measurement Instrument

The scanner behaves like a voltmeter or oscilloscope.

It reports what it measures.

It does not attempt to correct inconsistent measurements.

It performs no inference.

If an impossible electrical condition is observed, that condition is reported
exactly as measured.

Higher architectural layers may choose to interpret such conditions as faults.

The scanner shall not.

Hardware reliability should therefore be achieved by producing trustworthy
measurements, not by teaching the processor-independent scanner to hide
electrical errors.

---

# Fix Physical Problems at the Physical Layer

The hardware experiments provide an important architectural lesson.

When long cables produced false-positive continuity measurements, several
software-level responses would have been possible:

- measure fewer relationships
- ignore known troublesome pairs
- infer a "more likely" topology
- filter impossible maps
- encode game-specific expectations into the scanner

Those approaches would make the logical model compensate for an unreliable
physical measurement.

Sentinel instead prefers:

```text
physical measurement problem
        |
        v
characterize physical cause
        |
        v
improve electrical interface
        |
        v
preserve clean logical measurement
```

This keeps uncertainty and compensation in the layer where they belong.

The scanner should not become more complicated merely because the electrical
interface is insufficiently robust.

---

# Experiments Are Part of the Engineering Record

Hardware experiments are stored under:

```text
experiments/
```

The numbered experiment directories preserve chronological development.

The sequence through the current hardware characterization is:

```text
01  ESP32 settling time

↓

02  Full 21-pair scan

↓

03  Full-scan settling characterization

↓

04  Drive vs. release settling

↓

05  Scan-order characterization

↓

06  GPIO reassignment

↓

07  External pull-up characterization
```

Each experiment should preserve enough information to answer:

- What question was being asked?
- What hardware and software were used?
- What variable changed?
- What raw evidence was observed?
- What conclusion was reasonable at the time?
- What remained uncertain?

An experiment README is allowed to contain a conclusion that later evidence
refines.

The Git history and raw results preserve that evolution.

---

# Use Domain Terminology

Whenever practical, Sentinel adopts terminology already used within the sport
of fencing.

Examples include:

- Line
- A-Line
- B-Line
- C-Line

rather than inventing new software terminology.

This makes the documentation easier for armorers, coaches, officials, and
experienced fencers to understand.

Processor-specific terminology such as GPIO appears only within hardware
abstraction layers and hardware-specific documentation.

---

# Documentation Is Part of the Design

Documentation is treated as part of the engineering process rather than an
afterthought.

Specifications define contracts.

Rationale explains decisions.

Source code implements specifications.

Experiments preserve evidence.

Status documents preserve the current engineering handoff.

These documents serve different purposes and should not be collapsed into one
another.

The repository should remain understandable even if individual developers are
no longer available.

---

# Git Commits Represent Ideas

Sentinel favors small commits that each represent one complete engineering
idea.

Examples include:

- Define Sentinel electrical continuity model
- Specify Sentinel continuity scanner
- Define scanner line and continuity map interfaces
- Document ESP32 scan order experiment results
- Document ESP32 GPIO reassignment experiment
- Document ESP32 external pull-up experiment

This approach makes the Git history an engineering narrative rather than
simply a record of file changes.

Experimental source, raw results, and interpretation should normally be
committed together when they form one completed experimental record.

---

# Evolve Through Refactoring

The project intentionally embraces refactoring.

Initial implementations are expected to improve over time.

Architectural decisions should be relatively stable.

Implementations are expected to evolve.

The preferred approach is:

```text
Make it correct.

↓

Make it understandable.

↓

Make it fast.
```

Hardware characterization adds another useful formulation:

```text
Make it measurable.

↓

Measure it.

↓

Understand it.

↓

Optimize it.
```

Optimization should follow evidence.

It should not substitute for evidence.

---

# Design For Portability

Sentinel is intended to outlive any single processor family.

The architecture therefore separates:

- electrical concepts
- hardware interfaces
- processor implementations
- game logic

The current ESP32 experiments are valuable because they characterize one
physical implementation.

Their GPIO numbers, pull-up implementation, timing behavior, and Arduino
framework overhead are not Sentinel architectural requirements.

The long-term goal is for the same processor-independent core library to
support multiple processor families without modification.

A future processor may have different:

- GPIO characteristics
- input thresholds
- internal pull-ups
- drive strength
- timing behavior
- hardware peripherals

Those differences should be accommodated by the hardware-specific layer.

---

# Long-Term Goal

The ultimate objective is not simply to build a fencing scoring machine.

The goal is to build an engineering platform that is:

- understandable
- maintainable
- portable
- deterministic
- measurable
- evidence-driven
- well documented
- enjoyable to work on

Every significant design decision should support those goals.

The hardware experiments reinforced a principle that should remain central to
Sentinel:

> **Preserve the clean model. Measure the real system. Fix problems at the
> layer where they actually occur.**

And when performance is uncertain:

> **Measure first. Optimize from evidence.**