\# Experiment 06 — ESP32 GPIO Reassignment Characterization



\## Purpose



This experiment investigated whether the structured continuity errors observed

in earlier full-scan experiments were strongly associated with the original

ESP32 GPIO assignments.



Experiment 05 demonstrated that scan order can materially affect reliability

under marginal settling conditions. However, the logical Sentinel lines,

physical GPIO assignments, external fencing wiring, and measurement direction

were still partially confounded.



Experiment 06 changed the GPIO assignment while retaining the same scanner

methodology and scan-order characterization.



The experiment is diagnostic. It does not select a production GPIO assignment

or production settling-time constant.



\---



\## Hardware



The experiment used a TTGO T-Display based on the classic ESP32.



The physical test configuration used floor cords and reels.



\---



\## GPIO Reassignment



The original Experiment 05 assignment was:



| Sentinel line | Experiment 05 GPIO |

|---|---:|

| RA | 21 |

| RB | 22 |

| RC | 17 |

| MT | 32 |

| GC | 25 |

| GB | 26 |

| GA | 27 |



Experiment 06 used:



| Sentinel line | Experiment 06 GPIO |

|---|---:|

| RA | 21 |

| RB | 32 |

| RC | 17 |

| MT | 25 |

| GC | 26 |

| GB | 33 |

| GA | 13 |



Five of the seven logical lines were therefore reassigned.



RA and RC remained on GPIO21 and GPIO17.



This is an important limitation of the experiment because earlier testing had

identified RA-RC as a particularly sensitive relationship. Experiment 06

therefore cannot independently distinguish logical RA-RC behavior from

behavior associated specifically with GPIO21 and GPIO17.



\---



\## Measurement Method



The experiment retained the full 21-pair continuity scan.



Every unordered pair was measured exactly once and translated into the same

canonical 21-bit Sentinel continuity map regardless of physical measurement

order.



The three scan orders remained:



\### Forward



```text

RA -> RB -> RC -> MT -> GC -> GB -> GA

