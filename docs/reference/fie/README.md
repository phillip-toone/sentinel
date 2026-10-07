# FIE Reference Documents

## Purpose

This directory preserves external Fédération Internationale d'Escrime
(FIE) documents that serve as authoritative or historically relevant
references for Sentinel's electrical interpretation and game-rule
design.

These documents are **reference material**, not Sentinel specifications.

Sentinel requirements derived from them belong in
`docs/specifications/`. Design reasoning based on them belongs in
`docs/rationale/`.

The purpose of retaining the source documents here is to preserve the
exact rule editions and supporting material against which Sentinel was
designed.

------------------------------------------------------------------------

# Current Primary References

## FIE Material Rules --- August 2026

Repository file:

``` text
FIE_Material_Rules_2026-08.pdf
```

Original downloaded filename:

``` text
204156-book material August 2026 ang.pdf
```

Organization: Fédération Internationale d'Escrime (FIE)

Edition: August 2026

Role in Sentinel:

This is the primary material/equipment reference currently used for
Sentinel's electrical interpretation and scoring-apparatus requirements.

Of particular importance are the sections defining foil, épée, and sabre
electrical construction and the scoring-apparatus characteristics in
Annexe B, including contact qualification, blocking, double-hit, and
anti-whip timing.

This document should be treated as the current primary source when a
Sentinel requirement depends on FIE material or scoring-apparatus
behavior.

------------------------------------------------------------------------

## FIE Technical Rules --- August 2026

Repository file:

``` text
FIE_Technical_Rules_2026-08.pdf
```

Original downloaded filename:

``` text
204126-Technical rules August 2026 ang.pdf
```

Organization: Fédération Internationale d'Escrime (FIE)

Edition: August 2026

Role in Sentinel:

This is the corresponding current technical-rules reference.

It should be used when Sentinel's game-engine behavior depends on
fencing rules, bout operation, judging conventions, weapon-specific
scoring rules, or other technical requirements not defined solely by the
Material Rules.

The Technical Rules and Material Rules should be considered together
when a requirement spans both scoring-apparatus behavior and fencing
interpretation.

------------------------------------------------------------------------

# Historical Supporting Reference

## FIE New Rules for the Sabre --- 2016

Repository file:

``` text
FIE_Sabre_Rule_Changes_2016.pdf
```

Original downloaded filename:

``` text
123895-new rules for sabre_cover_ang.pdf
```

Organization: Fédération Internationale d'Escrime (FIE), SEMI Commission

Date: 2016

Role in Sentinel:

This document records the change in sabre scoring-apparatus blocking
time from:

``` text
120 ms +/- 10 ms
```

to:

``` text
170 ms +/- 10 ms
```

with application beginning in the 2016--2017 season.

It is retained primarily as historical provenance. It helps explain why
older scoring-machine documentation or implementations may refer to a
120 ms sabre timing value while current FIE Material Rules specify 170
ms +/- 10 ms.

Current Sentinel behavior should be derived from the current FIE rules
rather than from this historical document.

------------------------------------------------------------------------

# Source Authority

Reference documents in this directory do not all have equal authority.

For current Sentinel design decisions, prefer sources in approximately
this order:

``` text
current FIE Material Rules
        |
        v
current FIE Technical Rules
        |
        v
current FIE official supporting / SEMI documents
        |
        v
historical FIE documents
```

Historical documents are useful for understanding how requirements
evolved, but they shall not override current rules.

When documents appear to conflict, verify the document date, edition,
amendment history, whether one supersedes another, and whether the
requirements apply to the same weapon and apparatus behavior.

Do not silently combine requirements from different editions.

------------------------------------------------------------------------

# External Sources vs. Sentinel Requirements

The presence of an FIE document in this directory does not automatically
make every statement in that document a Sentinel software requirement.

The intended flow is:

``` text
FIE source document
        |
        v
source verification
        |
        v
engineering interpretation
        |
        v
Sentinel specification
        |
        v
implementation
        |
        v
testing
```

Important distinctions should be preserved between what the FIE source
explicitly requires, what Sentinel engineers infer from that
requirement, what Sentinel ultimately specifies, and how a particular
implementation satisfies it.

------------------------------------------------------------------------

# Timing Requirements

The FIE Material Rules contain several timing concepts that must not be
collapsed into one generic timing value.

Examples include:

``` text
contact qualification / sensitivity
blocking / lockout
double-hit behavior
anti-whip behavior
visual or audible output duration
```

These describe different apparatus behaviors.

Sentinel documentation should identify which FIE timing requirement is
being implemented rather than referring generically to a weapon's
"timing."

------------------------------------------------------------------------

# Electrical Terminology

Sentinel models seven logical electrical lines:

``` text
RA
RB
RC
CP
GC
GB
GA
```

The FIE documents describe the physical fencing equipment and its
electrical connections.

Sentinel's weapon-specific electrical interpretation maps those physical
requirements onto the seven logical lines and their twenty-one canonical
continuity relationships.

That mapping belongs in Sentinel documentation, not in modifications to
these reference documents.

The reference PDFs should remain unchanged from their source versions.

------------------------------------------------------------------------

# Document Preservation

Reference PDFs in this directory should be treated as immutable
snapshots.

Do not edit the PDF files to add annotations, highlights, corrections,
or Sentinel-specific terminology.

If an FIE document is superseded:

1.  retain the older document when it remains useful for historical
    provenance;
2.  add the new document as a separate file;
3.  update this README to identify the new current edition;
4.  update affected Sentinel specifications and rationale deliberately;
5.  preserve Git history showing when the governing reference changed.

Do not overwrite an older edition with a newer PDF under the same
filename.

------------------------------------------------------------------------

# Recommended Naming

Use descriptive repository filenames rather than opaque download
identifiers.

Current recommended names are:

``` text
FIE_Material_Rules_2026-08.pdf
FIE_Technical_Rules_2026-08.pdf
FIE_Sabre_Rule_Changes_2016.pdf
```

Record the original downloaded filename in this README so the repository
copy can still be traced back to its source.

------------------------------------------------------------------------

# Provenance

The documents in this directory were obtained from FIE-published
material for use as engineering references during Sentinel development.

When adding a new reference document, record at least:

``` text
organization
document title
edition or date
original filename
source URL, when known
purpose in Sentinel
```

Where practical, also record the date on which the source was retrieved
or verified.

------------------------------------------------------------------------

# Maintenance Principle

Sentinel should preserve the source material needed to understand its
design, while keeping a clear boundary between external rules and
Sentinel's own architecture.

The guiding principle is:

> **Preserve the authoritative source, document the interpretation, and
> specify Sentinel behavior separately.**
