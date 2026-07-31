---
title: System Model
version: 1.0
status: Draft
last_updated: 2026-07-31
authors:
  - Phillip Toone
  - ChatGPT
---

# System Model

## Purpose

This document describes the conceptual flow of information through Sentinel.

Rather than describing software classes or hardware interfaces, it explains how a physical event becomes a scoring decision.

Understanding this model should make the architecture of Sentinel largely self-evident.

---

# Overview

Sentinel transforms observations of the physical world into fencing decisions.

This transformation occurs in several distinct stages.

```
Physical World
        │
        ▼
Observation Layer
        │
        ▼
Interpretation Layer
        │
        ▼
Scoring Layer
        │
        ▼
Presentation Layer
```

Each layer has a single responsibility.

---

# Physical World

The physical world contains the things that Sentinel cannot control.

Examples include:

- Weapon tips
- Blade contact
- Body contact
- Ground paths
- Referee button presses
- Time

The physical world exists independently of Sentinel.

---

# Observation Layer

The Observation Layer records measurable facts about the physical world.

Examples include:

- Circuit opened
- Circuit closed
- Voltage changed
- Timer advanced
- Button pressed

Observations contain no fencing knowledge.

An observation is simply something that happened.

Examples:

```
LeftTipCircuitClosed

RightGuardOpened

ClockAdvanced(1 ms)
```

---

# Interpretation Layer

The Interpretation Layer understands weapons.

It converts electrical observations into weapon-specific events.

Examples include:

- Valid épée contact
- Foil tip depressed
- Sabre blade contact
- Off-target contact
- Lockout interval expired

This layer understands weapon rules but does not understand score.

Its responsibility is answering questions such as:

> "Did the observations represent a valid weapon event?"

---

# Scoring Layer

The Scoring Layer understands the rules of fencing.

It consumes interpreted weapon events.

It owns:

- Score
- Timer
- Cards
- Priority
- Bout state
- Referee commands

The Scoring Layer does not know:

- GPIO
- Voltages
- Continuity
- Pin numbers

It only understands fencing concepts.

Examples:

```
AwardLeftTouch

AwardRightTouch

StartClock

StopClock

IssueYellowCard
```

---

# Presentation Layer

The Presentation Layer communicates the current state of the bout.

Examples include:

- Built-in display
- Android application
- Web interface
- Bluetooth
- LEDs
- Buzzer

Presentation components never own the bout state.

They present the state owned by the Scoring Layer.

---

# One Source of Truth

The Scoring Layer is the authoritative owner of all bout state.

Presentation components may cache state for display purposes but must never become authoritative.

---

# Design Principles

Each layer should know only what is necessary to perform its responsibility.

For example:

- The Observation Layer should not know fencing.
- The Interpretation Layer should not know score.
- The Scoring Layer should not know hardware.
- The Presentation Layer should not make scoring decisions.

Maintaining these boundaries allows each layer to evolve independently.

---

# Example

The following example illustrates the conceptual flow.

```
Weapon tip contacts opponent

↓

Circuit closes

↓

Observation:
LeftTipCircuitClosed

↓

Observation duration measured

↓

Interpretation:
ValidÉpéeContact

↓

Scoring:
AwardLeftTouch

↓

Presentation:
Left light on
Increase left score
Play buzzer
Notify connected clients
```

This sequence is conceptual rather than implementation-specific.

Different hardware platforms may produce different observations while sharing the same interpretation and scoring layers.

---

# Future Work

Subsequent specifications will describe each layer in greater detail.

These include:

- Domain Model
- Requirements
- Architecture
- Engine Specification

This document establishes only the conceptual model.
