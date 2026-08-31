\# Experiment 07 — ESP32 External Pull-Up Characterization



\## Purpose



This experiment investigated how external pull-up resistance affects the

settling behavior and reliability of Sentinel's full 21-pair continuity scan

when connected to realistic fencing cabling.



Earlier experiments established that:



\- short wiring can be scanned with little or no explicit settling delay,

\- floor cords and reels can produce structured false-positive measurements,

\- drive-to-sample settling is more important than release settling for the

&#x20; observed failures,

\- scan order can strongly affect reliability under marginal conditions, and

\- substantial GPIO reassignment does not eliminate the settling phenomenon.



Experiment 07 tests whether pull-up strength is a major electrical parameter

controlling the observed behavior.



\---



\## Hardware



The experiment used a TTGO T-Display based on the classic ESP32.



GPIO assignments returned to the original experimental mapping:



| Sentinel line | GPIO |

|---|---:|

| RA | 21 |

| RB | 22 |

| RC | 17 |

| MT | 32 |

| GC | 25 |

| GB | 26 |

| GA | 27 |



The physical test configuration used floor cords and reels.



\---



\## Electrical Change



Earlier scanner experiments configured released and sensed lines as:



```cpp

INPUT\_PULLUP

```



Experiment 07 instead configured them as:



```cpp

INPUT

```



External resistors provided the pull-up to the supply.



This intentionally removed the ESP32 internal pull-up from the measurement

interface so that pull-up resistance could be varied externally.



The driven line continued to be configured as an output and driven LOW.



\---



\## External Pull-Up Values



Five external pull-up resistance values were tested:



```text

1.8 kΩ

3.9 kΩ

8.2 kΩ

15 kΩ

33 kΩ

```



Each result filename records both the topology and resistor value.



For example:



```text

000000000000000000000\_01800.txt

```



means:



```text

Topology:         000000000000000000000

External pull-up: 1800 Ω

```



\---



\## Scan Orders



The experiment retained the three scan orders used in Experiment 05.



\### Forward



```text

RA -> RB -> RC -> MT -> GC -> GB -> GA

```



\### Reverse



```text

GA -> GB -> GC -> MT -> RC -> RB -> RA

```



\### Interleaved



```text

RA -> GA -> RB -> GB -> RC -> GC -> MT

```



\---



\## Settling Times



For each external pull-up value, the tested drive-to-sample settling intervals

were:



```text

0 us

10 us

20 us

50 us

100 us

150 us

200 us

```



Explicit release settling remained:



```text

0 us

```



Each order/delay condition performed 10,000 complete continuity scans.



\---



\## Trusted Reference



Before characterization, the program established a trusted reference using:



```text

Forward scan order

Drive settling:   1000 us

Release settling: 1000 us

Reference scans:  1000

```



All 1,000 reference scans were required to agree before characterization

continued.



The recorded Experiment 07 configurations produced valid references.



\---



\## Tested Topologies



Three floor-cord + reel topologies were tested:



```text

000000000000000000000

000000000010000010000

100000000000000000001

```



Each topology was tested with all five external pull-up values.



This produced 15 raw result files.



\---



\## Results



External pull-up resistance had a large and systematic effect on the required

settling interval.



The first tested drive-settling interval at which all three scan orders

completed 10,000 scans without observed error was:



| External pull-up | `000000000000000000000` | `000000000010000010000` | `100000000000000000001` |

|---:|---:|---:|---:|

| 1.8 kΩ | 0 us | 0 us | 0 us |

| 3.9 kΩ | 0 us | 0 us | 0 us |

| 8.2 kΩ | 10 us | 10 us | 20 us |

| 15 kΩ | 10 us | 20 us | 50 us |

| 33 kΩ | 50 us | 50 us | 100 us |



These values are tested points, not measurements of the exact transition

threshold.



\---



\## Strong Pull-Up Results



\### 1.8 kΩ



With 1.8 kΩ external pull-ups, all three tested topologies completed all three

scan orders without observed errors at zero explicit drive-settling delay.



\### 3.9 kΩ



With 3.9 kΩ external pull-ups, all three tested topologies again completed all

three scan orders without observed errors at zero explicit drive-settling

delay.



Therefore, under these tested conditions, neither 1.8 kΩ nor 3.9 kΩ required

an additional explicit `delayMicroseconds()` between driving a line LOW and

sampling the sense lines.



This does not mean that the physical electrical settling time was zero.



Software execution itself introduces time between GPIO operations.



\---



\## Intermediate Pull-Up Results



As pull-up resistance increased, explicit settling delay became necessary.



At 8.2 kΩ, the most difficult tested topology required the tested 20 us point

for all three scan orders to complete without observed errors.



At 15 kΩ, the most difficult topology required the tested 50 us point.



At 33 kΩ, the most difficult topology required the tested 100 us point.



The transition therefore changed progressively with pull-up resistance rather

than appearing as a simple binary effect.



\---



\## Error Character



Failures continued to be dominated by false-positive continuity

measurements.



No corresponding population of false-negative failures emerged from the

resistor sweep.



Increasing the drive-to-sample settling interval eliminated the observed

false positives.



\---



\## Scan-Order Interaction



Experiment 05 demonstrated that scan order can have a very large effect when

the scanner is operating near its settling limit.



Experiment 07 provides additional context for that result.



With sufficiently strong external pull-ups, the Forward, Reverse, and

Interleaved orders all completed the tested conditions without requiring

additional explicit drive settling.



As pull-up resistance increased and the interface became more

settling-sensitive, differences among scan orders reappeared.



This indicates that scan-order sensitivity is strongly coupled to the

electrical settling behavior of the interface.



It does not establish scan order itself as the underlying physical cause.



\---



\## Findings



Experiment 07 established that:



1\. External pull-up resistance strongly affects the settling requirement of

&#x20;  the full 21-pair scanner.



2\. Stronger external pull-ups substantially reduce the required explicit

&#x20;  drive-to-sample delay.



3\. Under the tested conditions, 1.8 kΩ and 3.9 kΩ external pull-ups completed

&#x20;  all three topologies and all three scan orders without observed errors at

&#x20;  zero explicit drive delay.



4\. Increasing resistance from 8.2 kΩ through 15 kΩ to 33 kΩ progressively

&#x20;  increased the tested settling interval required for error-free operation.



5\. Scan-order sensitivity becomes much less significant when the electrical

&#x20;  interface settles sufficiently quickly.



6\. The observed failures remain dominated by false-positive continuity

&#x20;  measurements.



7\. The complete 21-pair continuity scan remains viable with realistic fencing

&#x20;  cabling when the electrical interface is appropriately designed.



\---



\## Interpretation



The systematic relationship between pull-up resistance and required settling

time strongly supports an electrical settling explanation for the previously

observed false positives.



A plausible interpretation is that cable and circuit capacitance interact

with pull-up resistance to determine how quickly sensed lines return toward a

valid HIGH level.



Lower pull-up resistance would be expected to reduce the corresponding

charging time and therefore reduce the required settling interval.



However, this experiment did not directly measure:



\- cable capacitance,

\- GPIO voltage versus time,

\- logic-threshold crossing time,

\- transient waveform shape, or

\- an equivalent lumped RC time constant.



Therefore Experiment 07 supports an RC-like settling interpretation but does

not by itself establish a complete quantitative electrical model.



\---



\## Engineering Tradeoff



A stronger pull-up improves settling speed but also increases current when a

line is intentionally driven LOW through a continuity path.



The resistor value should therefore not be selected solely by choosing the

lowest resistance that produced error-free scans.



Future hardware design should consider:



\- GPIO sink current,

\- current through multiple simultaneously connected pull-ups,

\- logic-level noise margin,

\- cable capacitance,

\- wiring resistance,

\- power consumption,

\- electrical protection, and

\- desired scan rate.



No production pull-up resistance is selected by this experiment alone.



\---



\## Important Meaning of Zero Delay



A configured drive-settling value of:



```text

0 us

```



means only that the program executes no additional explicit

`delayMicroseconds()` before sampling.



It does not mean that sampling occurs at the same physical instant that the

GPIO is driven LOW.



GPIO configuration, framework calls, loop execution, and digital reads all

consume finite time.



This distinction will become important if the scanner is later optimized

using lower-level GPIO operations.



\---



\## Conclusion



Experiment 07 provides strong evidence that the long-cable reliability

problem is fundamentally influenced by the electrical characteristics of the

input interface rather than being an inherent limitation of performing the

complete 21-pair scan.



External pull-up resistance has substantial control over the required

drive-to-sample settling interval.



With sufficiently strong external pull-ups, the complete scanner operated

across the tested realistic cabling topologies without requiring an

additional explicit settling delay.



The results support continued development of the full 21-pair Sentinel

continuity scanner.



The next engineering question is not whether the full scan must be abandoned,

but how to design the electrical interface to provide reliable logic levels,

appropriate current, adequate margin, and sufficient scan speed.

