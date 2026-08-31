\# Experiment 05 — ESP32 Scan-Order Characterization



\## Purpose



This experiment investigated whether the reliability problems observed during

full 21-pair continuity scanning depend on the order in which the seven

Sentinel electrical lines are measured.



Earlier experiments established that realistic fencing cabling can require

substantial drive-to-sample settling time. The resulting errors were

structured rather than random.



Experiment 05 asks whether changing measurement order changes those errors.



The experiment is diagnostic. It does not select a production scan order or

production settling-time constant.



\---



\## Hardware



The experiment used a TTGO T-Display based on the classic ESP32.



GPIO assignments were:



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



\## Canonical Continuity Map



Sentinel represents the seven logical electrical lines as 21 unique unordered

continuity relationships.



Experiment 05 deliberately separates:



\- physical measurement order

\- canonical bitmap position



Changing scan order therefore does not change the meaning of the resulting

21-bit continuity map.



For example, measuring RC while RA is driven and measuring RA while RC is

driven both represent the canonical RA-RC relationship.



\---



\## Scan Orders



Three measurement orders were tested.



\### Forward



```text

RA -> RB -> RC -> MT -> GC -> GB -> GA

