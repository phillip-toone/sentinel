---
title: Project Style Guide
version: 0.1
status: Draft
last_updated: 2026-07-30
authors:
  - Phillip Toone
---

# Project Style Guide

This document defines the conventions used throughout the Sentinel project.

The goal of these conventions is consistency, readability, and maintainability rather than rigid adherence to rules.

Whenever a convention no longer serves the project, it should be discussed and revised.

---

# Philosophy

Sentinel is intended to be a long-lived engineering project.

Every file should be understandable by someone reading it years later—including ourselves.

Documentation is considered part of the product.

---

# General Principles

- Prefer clarity over cleverness.
- Prefer explicitness over implicit behavior.
- Keep documents focused on a single purpose.
- Record important decisions.
- Explain *why*, not only *what*.
- Favor simple solutions over complex ones.

---

# Documentation

All major project documents should include YAML front matter.

Example:

```yaml
---
title: Vision
version: 0.1
status: Draft
last_updated: 2026-07-30
authors:
  - Phillip Toone
---
```

Document versions represent significant revisions rather than every edit.

Git provides the detailed history.

---

# Markdown

Use Markdown for all documentation.

Guidelines:

- Use ATX headings (`#`).
- Leave one blank line between sections.
- Wrap long paragraphs naturally for readability.
- Use fenced code blocks with language identifiers.
- Prefer tables only when they improve readability.

---

# Directory Structure

Each top-level directory should contain a README describing its purpose.

Directories should represent responsibilities rather than technologies whenever practical.

---

# Naming

Choose names that describe intent rather than implementation.

Good:

```
ScoringEngine
WeaponState
BoutClock
```

Less desirable:

```
ESP32Engine
MainManager
Utils
Stuff
```

Avoid abbreviations unless they are universally understood.

---

# Architecture Decision Records

Major design decisions should be documented as ADRs.

ADRs should explain:

- The problem.
- The decision.
- Alternatives considered.
- Consequences.

---

# Commit Messages

Each commit should represent one logical change.

Good examples:

```
Establish initial repository structure

Document project vision

Define scoring engine responsibilities
```

Avoid:

```
Update stuff

Fix things

More changes
```

---

# Issues

GitHub Issues are used for planning, design discussions, and future work.

Whenever practical:

- One issue should represent one topic.
- Decisions should reference issues.
- Completed architectural decisions should reference an ADR.

---

# Formatting

Unless a language requires otherwise:

- UTF-8 encoding
- Unix line endings (`LF`)
- Final newline at end of file
- No trailing whitespace

---

# AI Assistance

AI is considered a design and development partner.

All important architectural decisions should be reviewed by a human contributor before implementation.

The project should never depend upon a particular AI model.

Documentation should contain sufficient context that future contributors—or future AI systems—can understand the reasoning behind previous decisions.

---

# Evolving This Guide

This guide is expected to evolve.

Consistency is important, but improving the project is more important than preserving outdated conventions.

When a convention no longer serves the project well, it should be discussed, documented, and revised.
