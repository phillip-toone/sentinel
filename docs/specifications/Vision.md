---
title: Vision
version: 0.1
status: Draft
last_updated: 2026-07-30
authors:
  - Phillip Toone
---

# Vision

## Why Sentinel Exists

Modern fencing scoring systems range from expensive commercial equipment to hobby projects that often combine hardware, firmware, and user interfaces into a single application.

Sentinel exists to explore a different approach.

Rather than building another scoring machine, Sentinel aims to build a reusable, deterministic scoring engine that can serve as the foundation for many different scoring systems.

The engine should understand fencing—not displays, operating systems, or communication protocols.

Everything else should be replaceable.

---

## Our Goal

Our goal is to create an open, modular platform that is:

- Reliable enough to trust during competition.
- Flexible enough to adapt to future technologies.
- Understandable by new contributors.
- Pleasant to maintain for many years.
- Valuable as both a practical tool and an educational resource.

Sentinel is intended to demonstrate that embedded software can be both technically rigorous and enjoyable to work on.

---

## Guiding Principles

### Architecture Before Implementation

We believe thoughtful design reduces complexity.

We will strive to understand the problem before writing code.

---

### The Engine Is the Product

The scoring engine is the heart of Sentinel.

Displays, communication methods, hardware platforms, and user interfaces are clients of the engine—not the other way around.

---

### One Source of Truth

The scoring engine owns the state of the bout.

No display, mobile application, or external device should maintain an independent version of the score or bout state.

---

### Testability by Design

Correctness is more important than cleverness.

The scoring engine should be testable independently of any hardware.

Where practical, every rule should be verified through automated tests.

---

### Hardware Independence

The rules of fencing should not depend on GPIO pins, displays, Bluetooth, or a particular microcontroller.

The same scoring engine should be capable of running on embedded hardware, desktop simulators, and future platforms.

---

### Simplicity

We value simple designs over clever designs.

A system that is easy to understand is easier to verify, maintain, and extend.

---

### Documentation Matters

Documentation is part of the product.

Important design decisions should be recorded.

Future contributors—including ourselves—should be able to understand not only *what* was built, but *why*.

---

## Long-Term Vision

Our hope is that Sentinel becomes more than a personal project.

Whether adopted by clubs, students, hobbyists, or simply future versions of ourselves, Sentinel should serve as a reference implementation for thoughtful embedded software design.

If someone studying Sentinel learns better software engineering practices—even if they never build a fencing machine—then the project will have achieved one of its goals.

---

## Success

We will consider Sentinel successful if:

- The scoring engine is portable.
- The architecture is easy to understand.
- The behavior is deterministic.
- The code is enjoyable to maintain.
- The documentation explains the reasoning behind the design.
- The project helps others learn.

Working firmware is only one measure of success.

A well-designed system that teaches good engineering is equally valuable.
