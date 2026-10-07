# Sentinel Glossary

## Status

**Draft — terminology reference.** This glossary defines how Sentinel uses terms; it does not independently establish electrical, timing, or scoring requirements. Where an FIE rule is normative, consult the cited edition and article directly.

## Scope and naming policy

Sentinel uses three related vocabularies:

- **FIE / fencing domain:** terms describing fencing equipment, scoring-apparatus behavior, and competition rules. Prefer the FIE's wording when referring to those concepts.
- **Sentinel electrical and software domain:** names for our own measurement and interpretation abstractions. Preserve these names when they make an architectural distinction explicit.
- **Hardware implementation:** names for particular pins, actuators, peripherals, or circuits. Do not rename hardware merely because its function has an FIE name.

In particular, a **physical contact**, a **measured continuity relationship**, a **registered hit**, a **signalled hit**, and an **awarded touch** are not interchangeable.

## FIE / fencing equipment terminology

| Term | Meaning in Sentinel documentation | Usage guidance |
| --- | --- | --- |
| **Conductive piste** | The conductive fencing surface that neutralises hits on the floor under the applicable rules. | Prefer over *metallic strip*. Sentinel's logical line is `CP`. See FIE Material Rules m.57. |
| **Earth circuit** | The weapon/equipment return or protective electrical circuit identified as *earth* in FIE equipment descriptions. | Prefer *earth/earthed* when referring to FIE-defined circuits; do **not** assume this means a processor's GND pin or a direct connection to building protective earth. See m.18, m.29, m.31. |
| **Pointe d'arrêt** | The movable point of a foil or épée used in the weapon's electrical mechanism. | Use in formal weapon descriptions; *point* or *tip* may be used in explanatory prose when unambiguous. See m.11 and m.19. |
| **Conductive jacket** | Conductive garment connected to the target circuit in foil or sabre. | Use when referring specifically to the garment; *lamé* remains acceptable as common fencing terminology. See m.28 and m.34. |
| **Valid conductive surface** | Electrically conductive target area specified for a weapon; for sabre it includes the relevant jacket, glove, and mask surfaces. | Do not substitute *conductive jacket* when the intended surface includes glove or mask. See Annex B, sabre principles. |
| **Bodywire** | Cable assembly connecting a fencer's equipment to the scoring apparatus through the spool/connecting system. | Prefer the source term *bodywire* in formal equipment descriptions. See m.29, m.31, m.35. |
| **Scoring apparatus** | Equipment that registers and signals hits under the applicable FIE requirements. | Prefer over *scoring box* or *scoring machine* in specifications. *Central judging apparatus* may be used when referring to the FIE's specifically named equipment. |
| **Blocking interval** | Interval during which the scoring apparatus suppresses subsequent registrations according to weapon-specific rules. | Prefer *blocking* for the FIE apparatus mechanism; *lockout* may be retained when discussing existing code or general engineering mechanisms. The triggering event and duration are weapon-specific. |
| **Visual signal** | Light-based indication produced by the scoring apparatus. | Distinguish from the underlying registered hit and from the particular LED/LCD hardware. |
| **Audible signal** | Sound indication produced by the scoring apparatus. | Distinguish from the hardware buzzer (`BZ`) that may generate it. |
| **Valid / non-valid hit** | FIE apparatus categories, particularly relevant to foil, depending on the electrical and weapon-specific conditions. | Do not assume *valid* is synonymous with an awarded touch. Use *off-target* as explanatory terminology only when it matches the intended non-valid condition. |
| **Anti-whip behavior** | Weapon-specific sabre apparatus behavior concerning contact involving the opponent's blade and conductive target. | Use the current FIE Annex B requirements rather than treating it as ordinary debounce. |

## Sentinel electrical and observation terminology

| Term | Definition | Boundary |
| --- | --- | --- |
| **Logical line** | One of Sentinel's seven named electrical observation nodes: `RA`, `RB`, `RC`, `CP`, `GC`, `GB`, `GA`. | Not a GPIO number or a physical connector pin. |
| **Continuity relationship** | Measured conductive path between two logical lines, represented as a Boolean. | Reports measurement, not cause or fencing significance. |
| **ContinuityMap** | Canonical 21-bit representation of the unique unordered continuity relationships among the seven logical lines. | No inherent time or fencing interpretation. |
| **Electrical snapshot / snapshot** | One complete scanner result represented by a `ContinuityMap`. | Its constituent measurements may be acquired over a finite interval; it is not necessarily instantaneous. |
| **Continuity Scanner** | Processor-independent component responsible for producing a complete electrical snapshot. | Does not qualify touches, apply fencing timing rules, or maintain observation history. |
| **ContinuityObservation** | One complete `ContinuityMap` associated with a monotonic timestamp representing when the completed scan became available. | Adds temporal association, not interpretation. |
| **Observation Stream** | Chronologically ordered sequence of `ContinuityObservation` values. | Current conceptual contract produces one observation per completed scanner cycle, including repeated maps. |
| **Electrical Interpreter** | Anticipated layer that derives weapon-specific electrical meaning from observations. | Exact interface and responsibilities remain to be specified; it must not be conflated with raw measurement or game rules. |
| **Game Rule Engine** | Anticipated layer applying fencing rules to interpreted events and state. | Detailed interface and behavior remain to be specified. |
| **Event** | A meaningful occurrence inferred by a higher layer from observations or other inputs. | Not a synonym for one raw observation; concrete event types remain TBD. |
| **State / GameState** | Current modeled condition of the game or apparatus at a defined layer. | Specific fields and transition rules remain TBD. |

### Canonical line identifiers

| Identifier | Meaning | Index |
| --- | --- | ---: |
| `RA` | Red A line | 0 |
| `RB` | Red B line | 1 |
| `RC` | Red C line | 2 |
| `CP` | Conductive piste | 3 |
| `GC` | Green C line | 4 |
| `GB` | Green B line | 5 |
| `GA` | Green A line | 6 |

**Historical note:** Experiments 01–09 used `MT` (*Metallic Strip*) for the logical line now called `CP`. The rename changes neither its index nor any canonical continuity bit position. Preserve original experiment source code and captured results as historical evidence.

## Hardware terminology

| Term | Meaning | Distinction |
| --- | --- | --- |
| **GPIO** | Processor general-purpose input/output signal or pin. | Physical assignment is platform-specific; it is not a logical fencing line. |
| **`BZ`** | Existing Sentinel hardware identifier for a buzzer output. | Keep `BZ` at the hardware layer; use *audible signal* for apparatus behavior. |
| **Display / LED** | Physical presentation devices. | They may implement a *visual signal* but are not themselves the domain-level signal. |

## Fencing and game vocabulary requiring further specification

These terms appeared in the original glossary. They are retained here so that future game-engine work can define them from the current FIE Technical Rules and the intended Sentinel scope rather than assigning premature or conflicting meanings.

| Term | Current status |
| --- | --- |
| **Bout** | Fencing competition term; detailed Sentinel lifecycle semantics TBD. |
| **Match** | Usage and distinction from *bout* TBD; do not assume they are interchangeable in formal specifications. |
| **Period** | Bout timing subdivision; detailed timing and transition semantics TBD. |
| **Passivity** | Fencing-rule concept; exact current rule terminology and Sentinel behavior TBD. |
| **Lockout** | Existing/general term; prefer *blocking interval* for the FIE apparatus behavior. Any distinct software meaning must be explicitly defined. |
| **Touch** | Fencing outcome term; distinguish from raw electrical contact and from apparatus registration. Awarding semantics TBD. |
| **Hit** | Ambiguous without qualification: physical contact, registered hit, or signalled hit. Qualify the term when timing or scoring depends on the distinction. |
| **Valid** | Context-dependent: valid conductive target, valid registered hit, or awarded touch are different concepts. |
| **Off-target** | Common descriptive term, especially for foil; use *non-valid hit* where matching FIE apparatus terminology. |

## Source and authority

Current reference editions preserved in `docs/reference/fie/`:

- `FIE_Material_Rules_2026-08.pdf` — equipment construction and apparatus requirements, especially m.11, m.18–19, m.29, m.31, m.34–35, m.57, and Annex B.
- `FIE_Technical_Rules_2026-08.pdf` — competition and game-rule terminology; detailed game terms above await a targeted review.
- `FIE_Sabre_Rule_Changes_2016.pdf` — historical context, not a replacement for current requirements.

The normative definitions of `ContinuityMap`, the scanner, and `ContinuityObservation` remain in their dedicated specifications. If this glossary conflicts with an authoritative specification, resolve the conflict explicitly rather than silently changing behavior.

## Maintenance

Update this glossary when a domain term is adopted or changed. Document aliases and historical names, avoid blanket search-and-replace of ambiguous words, and preserve raw experiment outputs and historical code. Changes to public identifiers or data representations require their own compatibility review and tests.
