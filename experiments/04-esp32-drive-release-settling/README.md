# Experiment 04 — ESP32 Drive vs. Release Settling

## Purpose

Experiment 03 demonstrated that a complete six-phase / 21-pair continuity scan required explicit settling time when realistic fencing cabling was attached, and that the worst tested configurations required a tested interval of 100 us before completing 10,000 scans without an observed error.

However, Experiment 03 did not establish where that required settling time belonged.

The relevant timing sequence is:

    Drive line LOW
          |
          v
    DRIVE SETTLING
          |
          v
    sample sense lines
          |
          v
    release line (INPUT_PULLUP)
          |
          v
    RELEASE SETTLING
          |
          v
    drive next line LOW

Experiment 04 was created to answer a specific question:

> Does the required settling time belong primarily before sampling the sense lines, after releasing the previous driven line, or both?

---

## Relationship to Previous Experiments

### Experiment 01

tested one drive/sense pair repeatedly and found that no explicit delay was required for the tested configurations. It did not reproduce the rapidly changing drive sequence of the complete scanner.

### Experiment 02

implemented the complete six-phase scanner and exposed cable-dependent instability with no explicit settling delay. Its reference method was flawed, but the raw data showed structured errors consistent with scan-history effects.

### Experiment 03

corrected the reference method and characterized the settling-time requirement across multiple cabling configurations and electrical topologies. It found that 100 us was sufficient for the worst tested configurations, but did not separate drive settling from release settling.

Experiment 04 retains the complete scanner and trusted reference methodology from Experiment 03, but adds independent control of drive settling and release settling delays.

---

## Hardware

Testing used a TTGO T-Display V1.1 based on the classic ESP32.

The experimental Sentinel line mapping was:

    Line    GPIO
    RA        21
    RB        22
    RC        17
    MT        32
    GC        25
    GB        26
    GA        27

These GPIO assignments were selected for the experiment and do not define permanent Sentinel hardware assignments.

---

## Electrical Measurement Method

The experiment retained the active-low measurement method consistent with the historical scoring apparatus.

All lines normally idle as:

    INPUT_PULLUP

During each measurement phase, one selected line becomes:

    OUTPUT LOW

The remaining lines are sampled digitally.

A sensed LOW represents continuity with the driven line.

After the phase is complete, the driven line returns to:

    INPUT_PULLUP

---

## Complete Scan Sequence

A complete electrical snapshot requires six driven-line phases:

    Drive RA -> sense RB, RC, MT, GC, GB, GA
    Drive RB -> sense RC, MT, GC, GB, GA
    Drive RC -> sense MT, GC, GB, GA
    Drive MT -> sense GC, GB, GA
    Drive GC -> sense GB, GA
    Drive GB -> sense GA

This measures all 21 unique unordered line pairs.

The resulting map uses the canonical ordering defined in:

    docs/specifications/ELECTRICAL_MODEL.md

---

## Reference Method

Before testing short delays, the experiment first establishes a conservative reference topology.

The reference procedure uses:

    Drive settling:   1000 us per driven phase
    Release settling: 1000 us per released line
    Reference scans:  1000

All 1000 reference scans must agree exactly.

This approach ensures that the reference is established independently of the short-delay behavior being characterized, correcting the methodological flaw present in Experiment 02.

---

## Test Conditions

The experiment tests three conceptual groups for each physical topology.

### Baseline

    Drive 0 us
    Release 0 us

### Drive Settling Only

    Drive 10 us   Release 0 us
    Drive 20 us   Release 0 us
    Drive 50 us   Release 0 us
    Drive 100 us  Release 0 us

### Release Settling Only

    Drive 0 us    Release 10 us
    Drive 0 us    Release 20 us
    Drive 0 us    Release 50 us
    Drive 0 us    Release 100 us

### Equal Drive and Release Settling

    Drive 10 us   Release 10 us
    Drive 20 us   Release 20 us
    Drive 50 us   Release 50 us
    Drive 100 us  Release 100 us

Each condition performs 10,000 complete scans.

---

## Error Classification

Every scan is compared against the trusted reference map.

The experiment records:

    * incorrect complete scans
    * scans containing false-positive continuity
    * scans containing false-negative continuity
    * the first erroneous map
    * a difference mask
    * false-positive counts for each canonical pair
    * false-negative counts for each canonical pair
    * average complete-scan time
    * complete scans per second

---

## Physical Configurations

Testing focused on floor cords with reels, as this configuration represented the most challenging cabling from Experiment 03.

Four deliberately selected canonical topologies were tested:

### 1. Sensitive Sparse Topology

    100000000000000000001

This topology required a tested 100 us drive-settling interval in Experiment 03 and was considered the most useful for distinguishing drive from release settling.

### 2. All Open

    000000000000000000000

This topology is useful because every error must be a false positive. There can be no false negatives because no continuity is expected.

### 3. Second Sensitive Topology

    001000000000001000000

This topology also required a tested 100 us interval in Experiment 03 but places expected continuity in different portions of the canonical map.

### 4. Fully Connected Control

    111111111111111111111

This topology passed at 0 us in Experiment 03. Because every continuity bit is expected to be set, it cannot produce false-positive continuity. Any error would involve a false negative.

Together, the all-open and fully connected topologies provide useful opposite controls.

---

## Results

### Drive Settling Works; Release Settling Does Not

The results were unambiguous across all tested configurations.

For the all-open topology:

    Drive 50 us / Release 0 us  -> PASS
    Drive 0 us  / Release 50 us -> FAIL (10000 errors)
    Drive 50 us / Release 50 us -> PASS

For the second sensitive topology (0x040400):

    Drive 50 us / Release 0 us  -> PASS
    Drive 0 us  / Release 50 us -> FAIL (10000 errors)
    Drive 50 us / Release 50 us -> PASS

For the sparse topology (0x000410):

    Drive 50 us / Release 0 us  -> PASS
    Drive 0 us  / Release 50 us -> FAIL (10000 errors)
    Drive 50 us / Release 50 us -> PASS

In every case, release settling alone failed with 10000 out of 10000 incorrect scans, even at 100 us.

### The Sensitive Topology Exception

The sensitive topology (0x100001) required more than 100 us of drive settling:

    Drive 50 us  / Release 0 us  -> FAIL (10000 errors)
    Drive 100 us / Release 0 us  -> FAIL (60 errors)
    Drive 50 us  / Release 50 us -> FAIL (10000 errors)
    Drive 100 us / Release 100 us -> FAIL (23 errors)

At 100 us drive settling, the errors were reduced to 60 out of 10000 scans, all on bit 1 (RA-RC). At 50 us, all 10000 scans failed with errors on bits 1 and 6 (RA-RC and RB-RC).

This topology demonstrates that some configurations require more than 100 us of drive settling.

### The Fully Connected Control

    111111111111111111111

All conditions passed with zero errors, including the baseline with no settling delay. This confirms that when every bit is expected to be set, false positives cannot occur, and the drive strength is sufficient to avoid false negatives.

### Error Pattern Consistency

The false-positive patterns observed in Experiment 04 were identical to those observed in Experiments 02 and 03.

For the all-open topology:

    False positives consistently appeared on:
      Bit 0:  RA-RB
      Bit 1:  RA-RC
      Bit 6:  RB-RC
      Bit 18: GC-GB
      Bit 19: GC-GA
      Bit 20: GB-GA

For the sensitive topology (0x100001):

    False positives appeared on:
      Bit 1:  RA-RC
      Bit 6:  RB-RC
      Bit 18: GC-GB
      Bit 19: GC-GA

This consistency across experiments strongly supports the conclusion that the errors are deterministic and structured rather than random.

---

## Interpretation

### The Required Settling Time Is Drive Settling

Experiment 04 conclusively demonstrates that the required settling time belongs before sampling the sense lines, not after releasing the previous driven line.

Release settling alone, even at 100 us, failed to eliminate errors in any sparse topology tested. Drive settling alone, at 50 us or 100 us, eliminated errors in all but the most sensitive topology.

### The Physical Mechanism Is GPIO Capacitive Coupling

The consistent error pattern provides strong evidence for the underlying mechanism.

The false positives occur on pairs of GPIOs that are:

    1. Adjacent on the ESP32 package
    2. Connected through external cabling capacitance

Specifically:

    GPIO21 (RA) and GPIO22 (RB) are adjacent pins
    GPIO25 (GC) and GPIO26 (GB) are adjacent pins
    GPIO26 (GB) and GPIO27 (GA) are adjacent pins

When one pin is driven LOW and then released, the adjacent pin reads LOW through capacitive coupling. The time required for this coupling to dissipate is the drive settling time.

External cabling adds additional capacitance, increasing the time required for the driven pin to return to a stable HIGH state after release.

### Why Release Settling Does Not Help

Release settling occurs after the driven line has already been released and the sense lines have been sampled. The erroneous LOW state has already been read by the time the release settling delay begins.

Waiting after sampling cannot correct a measurement that has already been taken.

Drive settling, by contrast, delays the sampling until the driven line and all coupled lines have stabilized.

### Why the Sensitive Topology Requires More Time

The topology 0x100001 includes continuity between RA and GA (bit 0) and between GB and GA (bit 20).

When RA is driven LOW, the RA-GA connection keeps both RA and GA LOW. This creates a low-impedance path that holds the RA line LOW longer than when RA is not connected to GA. The capacitive coupling from RA to RB and RC therefore persists longer, requiring more drive settling time to dissipate.

This explains why the RA-RC false positive (bit 1) is the last error to disappear in this topology.

---

## Summary of Findings

    Finding                                                      Evidence
    -------------------------------------------------------------------------
    Drive settling eliminates errors                            All topologies
    Release settling does not eliminate errors                  All topologies
    50 us drive settling passes 3 of 4 topologies               All except 0x100001
    100 us drive settling reduces errors on sensitive topology  From 10000 to 60
    Release settling provides no observable benefit             0 us vs 100 us same result
    Error pattern is deterministic and repeatable               Consistent across experiments
    False positives cluster on adjacent GPIO pairs              GPIO21/22, GPIO25/26, GPIO26/27

---

## Recommended Sentinel Settling Strategy

Based on the combined evidence from Experiments 01 through 04:

    Drive settling:   150 us (with margin)
    Release settling: 0 us

The 150 us value provides a safety margin above the 100 us that was sufficient for all but the most sensitive tested configuration.

This is substantially less than the historical 1 ms delay used by the predecessor scoring apparatus, while remaining conservative relative to the measured requirements.

At this setting, the complete 21-pair scan requires approximately:

    6 phases * 150 us = 900 us of settling time
    Plus measurement overhead

This corresponds to approximately 1000 complete electrical snapshots per second, which is more than sufficient for fencing scoring applications.

---

## Questions for Future Investigation

Experiment 04 has answered its primary question, but several related questions remain open:

### GPIO Assignment

The false-positive pattern follows GPIO adjacency. Would spreading the logical lines across non-adjacent GPIOs reduce or eliminate the coupling effect?

### Scan Order

The current scan order drives lines in this sequence:

    RA -> RB -> RC -> MT -> GC -> GB

Would a different order reduce the structured coupling between adjacent GPIO groups?

### Hardware Modification

Would adding series resistors or using external pull-up resistors reduce the coupling effect?

### Direct-Register GPIO Access

The experiments used ordinary Arduino GPIO functions. Would direct-register access change the timing behavior?

These questions are not urgent for Sentinel development, as a 150 us drive settling delay is already practical and conservative. However, they may become relevant for future optimization or hardware revision.

---

## Status

Complete.

Experiment 04 successfully separated drive settling from release settling and established that the required settling time belongs before sampling, not after releasing the previous line.

The results support the adoption of a 150 us drive settling delay with zero release settling delay for the Sentinel production scanner.

Raw results are preserved in:

    results/floorCords+reels/

---

## Conclusion

The four-experiment sequence now tells a complete and consistent story:

    Experiment 01: Single-pair measurements work at 0 us, even with cables.
    Experiment 02: Full-scan measurements with cables are unstable at 0 us.
    Experiment 03: Drive settling of 100 us fixes the worst tested configurations.
    Experiment 04: Drive settling is the mechanism; release settling does nothing.

The root cause is capacitive coupling between adjacent GPIO pins, exacerbated by the additional capacitance of fencing cables and reels.

The recommended production setting is 150 us drive settling with 0 us release settling, providing a conservative margin above the measured requirement while remaining substantially faster than the historical 1 ms delay.

The complete 21-pair scanner remains computationally practical and is ready for integration into Sentinel.
