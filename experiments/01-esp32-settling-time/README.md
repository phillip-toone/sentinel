# Experiment 01 — ESP32 Settling Time

## Purpose

This experiment was created to investigate the settling delay historically used by Sentinel's predecessor scoring firmware when measuring electrical continuity with an ESP32.

The existing scoring apparatus used the following measurement method:

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

The 1 millisecond delay in `setPIN()` was selected empirically because it produced reliable operation. The minimum required settling time had never been characterized systematically.

This experiment asked:

> How long after driving one ESP32 GPIO LOW must the system wait before a connected sense GPIO reliably reads LOW, and how long after releasing the driven GPIO must it wait before the sense GPIO reliably returns HIGH?

---

## Measurement Method

Two ESP32 GPIOs were used:

```text
DRIVE_PIN = GPIO22
SENSE_PIN = GPIO21
```

The sense pin remained configured as:

```text
INPUT_PULLUP
```

During the LOW-settling test, the drive pin was changed to:

```text
OUTPUT LOW
```

and the sense pin was sampled after a configurable delay.

During the HIGH-release test, the connected network was first allowed to settle LOW. The drive pin was then returned to:

```text
INPUT_PULLUP
```

and the sense pin was sampled after a configurable delay.

A sensed LOW during the drive phase indicated continuity.

---

## Settling Delays Tested

The experiment tested explicit delays of:

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
1000 us
```

Each delay was tested for:

```text
10,000 trials
```

for both:

* transition to LOW
* release back to HIGH

---

## Physical Configurations

Two configurations were tested.

### Short Jumper Baseline

The drive and sense GPIOs were connected using a short jumper.

Raw results are preserved in:

```text
results/short_jumper_baseline.txt
```

### Full Cable Configuration

The drive and sense GPIOs were connected through the longer fencing cabling available during testing.

Raw results are preserved in:

```text
results/full_all_cables.txt
```

---

## Results

Both physical configurations produced:

```text
0 LOW errors
0 HIGH errors
```

for all 10,000 trials at every explicit settling delay tested.

This included:

```text
0 us explicit delay
```

---

## Interpretation

The experiment demonstrated that the historical 1 millisecond delay was not required for reliable operation of this specific two-pin measurement sequence under the tested configurations.

However, a configured delay of `0 us` does **not** mean that the physical sense line was sampled at the same instant that the drive line changed state.

The Arduino operations themselves consume execution time:

```cpp
pinMode(...)
digitalWrite(...)
digitalRead(...)
```

Therefore this experiment established only that:

> The natural execution time between the Arduino GPIO operations was sufficient for the tested two-pin configurations.

It did not establish the actual electrical settling time of the fencing apparatus.

---

## Important Limitation

This experiment exercised one drive/sense relationship repeatedly.

It did **not** reproduce Sentinel's proposed complete scanner, in which the actively driven line changes rapidly:

```text
RA
RB
RC
MT
GC
GB
```

A full scan can therefore expose electrical history or transition effects that are not present when repeatedly testing the same two GPIOs.

This limitation became important in Experiment 02.

---

## Findings

Experiment 01 established that:

1. The historical 1 ms delay was highly conservative for the two-pin configurations tested.
2. No explicit settling delay was required for reliable repeated measurement of one drive/sense pair in 10,000 trials.
3. Arduino GPIO execution overhead provides some nonzero implicit settling time even when the configured delay is zero.
4. A two-pin test is insufficient to determine whether a complete rapidly switched seven-line scan is reliable.

---

## Resulting Question

The experiment led directly to the next question:

> Can the ESP32 reliably perform Sentinel's complete six-phase, 21-pair continuity scan with no explicit settling delay?

That question became the purpose of Experiment 02:

```text
02-esp32-full-scan
```

---

## Status

**Complete**

The experiment successfully answered its intended two-pin characterization question and motivated full-scanner testing.

Its results should not be interpreted as establishing a production settling-time requirement for Sentinel.
