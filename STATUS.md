# Status

## Current Phase

Hardware Integration

## Current Milestone

ESP32 Continuity Scanner Integration and Electrical Settling-Time Characterization

## Current Version

v0.1.0

## Project State

Sentinel has progressed beyond initial planning and domain modeling.

The electrical domain and continuity scanner architecture have been specified, the processor-independent scanner has been implemented, and its core behavior has been validated on a desktop C++ compiler using simulated electrical hardware.

The next phase is integration with real ESP32 hardware.

---

## Completed Specifications

The following specifications define the current scanner architecture:

- `docs/specifications/ELECTRICAL_MODEL.md`
- `docs/specifications/CONTINUITY_SCANNER.md`

The design reasoning leading to the current architecture is preserved in:

- `docs/rationale/DESIGN_RATIONALE.md`

These documents should be reviewed before making architectural changes to the scanner.

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

Hardware-specific behavior is intended to be supplied through a small hardware-facing interface.

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

The current implementation stores one complete electrical snapshot in a `ContinuityMap`.

---

## Desktop Validation

The processor-independent scanner has been compiled and tested successfully using:

```text
GCC 16.1.0
C++20
-Wall
-Wextra
-Wpedantic
```

A portable WinLibs GCC toolchain was used during development on Windows.

The following behavior has been verified:

- All 21 canonical line-pair bit assignments.
- Symmetry of continuity queries.
- Single isolated continuity connections.
- Transitive electrical connectivity.
- Multiple independent connected components.
- Correct isolation between independent components.

The current test infrastructure includes:

- `tests/scanner_smoke_test.cpp`
- `tests/MockNodeIO.h`
- `tests/continuity_scanner_test.cpp`

Generated executables are excluded by `.gitignore`.

---

## Simulated Electrical Model

`MockNodeIO` provides a software simulation of the seven electrical lines.

The mock models physical electrical connectivity rather than simply supplying expected scanner results.

For example:

```text
RA ----- GC ----- MT
```

is treated as one electrically connected component.

Therefore:

```text
RA-GC = continuity
RA-MT = continuity
GC-MT = continuity
```

This allows the processor-independent scanner to be tested without physical hardware.

---

## Existing Proven ESP32 Measurement Method

The existing fencing scoring apparatus uses the following measurement method:

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

The important electrical behavior is:

```text
Idle line:
    INPUT_PULLUP

Line under test:
    OUTPUT LOW

Connected sense line:
    reads LOW

Unconnected sense line:
    remains HIGH through its pull-up
```

Therefore the existing hardware detects continuity using **active-low measurement**.

The processor-independent scanner should not depend upon this polarity. Hardware-specific code should normalize the physical measurement into the logical continuity representation expected by the scanner.

---

## Settling-Time Discovery

The historical `delay(1)` in `setPIN()` represents a 1 millisecond settling delay before continuity is evaluated.

This delay was selected empirically during development of the original scoring apparatus.

The system appeared functional with shorter or absent settling behavior until realistic fencing cabling was introduced.

Problems were observed particularly when using:

- long floor cables
- cables routed away from the scoring apparatus
- fencing reels
- additional connecting cables associated with reels

The exact electrical cause and minimum required settling time were not characterized at the time.

The 1 millisecond delay was retained because it produced reliable operation.

---

## Important Performance Implication

A complete seven-line continuity scan requires six driven-line phases:

```text
Drive RA -> observe RB, RC, MT, GC, GB, GA
Drive RB -> observe RC, MT, GC, GB, GA
Drive RC -> observe MT, GC, GB, GA
Drive MT -> observe GC, GB, GA
Drive GC -> observe GB, GA
Drive GB -> observe GA
```

With a 1 millisecond settling delay for each phase, settling time alone requires approximately:

```text
6 ms per complete electrical snapshot
```

The original concern that a full 21-pair scan might be too slow therefore remains valid.

However, the required settling time has never been measured systematically.

The full 21-pair scanner shall **not** be abandoned or optimized away until real hardware measurements determine whether its scan rate is sufficient.

---

## Current Engineering Question

The next major engineering question is:

> What is the minimum reliable settling time required after selecting a driven line when Sentinel is connected to realistic fencing cabling and reels?

This should be determined experimentally rather than inferred from ESP32 GPIO switching specifications.

The GPIO itself switches much faster than 1 millisecond.

The unknown behavior involves the complete electrical system, including:

- cable length
- cable capacitance
- reel wiring
- connector resistance
- contact resistance
- internal pull-up characteristics
- external electrical loading

---

## Planned Hardware Experiment

The ESP32 implementation should initially reproduce the known-good measurement behavior while making settling time configurable.

Candidate settling times may include:

```text
1000 us
500 us
250 us
100 us
50 us
20 us
10 us
5 us
2 us
1 us
0 us
```

For each value, repeated scans should be performed and measurement errors counted.

Testing should progress from simple to realistic electrical configurations, including:

1. Bare or short wiring.
2. Body cords / short connecting cables.
3. Long floor cables.
4. Reels and associated connecting cables.
5. Real weapons and representative fencing equipment.

The objective is to establish an empirical safe settling time with an appropriate reliability margin.

---

## Current Architectural Direction

The processor-independent scanner should remain unchanged while hardware behavior is characterized.

The hardware-facing layer should conceptually provide operations equivalent to:

```text
beginMeasurement(line)
snapshot()
endMeasurement()
```

The exact API is not yet finalized.

For the existing ESP32 hardware, `beginMeasurement()` would likely:

1. Ensure non-driven lines are inputs with pull-ups.
2. Configure the selected line as an output.
3. Drive the selected line LOW.
4. Allow the required electrical settling time.

`snapshot()` would sample the seven lines and normalize active-low GPIO measurements into logical continuity bits.

`endMeasurement()` would return the driven line to its idle input/pull-up state.

This interface may evolve during hardware integration.

---

## Immediate Next Goal

Establish an ESP32 development environment on the primary Linux Mint development machine.

PlatformIO is currently the likely embedded toolchain, but Sentinel's processor-independent core should remain independent of PlatformIO.

The first hardware milestone is:

```text
Linux Mint
    |
    v
ESP32 toolchain
    |
    v
Compile firmware
    |
    v
Flash ESP32
    |
    v
Receive serial output
```

A simple ESP32 test program should be compiled, flashed, and observed over the serial monitor before integrating the Sentinel scanner.

---

## Next Steps

1. Pull the latest Sentinel repository onto the Linux Mint development machine.
2. Establish a working ESP32 compile/flash/serial workflow.
3. Identify the exact ESP32 board/module used by the existing scoring apparatus.
4. Implement the initial ESP32 hardware-facing continuity interface.
5. Reproduce the existing known-good 1 ms active-low measurement behavior.
6. Verify the full 21-pair scan on physical hardware.
7. Characterize minimum reliable settling time using realistic fencing cables and reels.
8. Only then evaluate whether scanner optimization or architectural changes are necessary.

---

## Guidance for Future Development

Do not redesign the processor-independent scanner merely because the historical implementation used a 1 millisecond delay.

Do not assume ESP32 GPIO transition time determines the required settling time.

Do not optimize GPIO access, function-call overhead, or scanner bit manipulation before measuring the real electrical system.

The immediate priority is:

> **Measure first. Optimize from evidence.**

---

## Last Known Clean Repository State

The scanner implementation and current desktop tests were committed and pushed through:

```text
3254c63  Test independent continuity components
```

At that point:

```text
branch: main
local:  synchronized with origin/main
working tree: clean
```

---

## Last Updated

2026-08-07
