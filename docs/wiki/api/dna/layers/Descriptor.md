# API Reference — `dna/layers/Descriptor`

---

<!-- ink:api name="Archetype" module="dna/layers/Descriptor" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Archetype`

Classifies a character's demographic archetype for DNA rig selection.

### Why this exists

DNA assets carry per-archetype rig data such as blend shape weights and joint ranges. Using a strongly-typed enum prevents raw integer or string mismatches at call sites and ensures only valid archetype identifiers reach the configuration system. The `alien` and `other` values extend coverage beyond human demographics to stylised and non-human characters.

### Fields

| Name | Type | Description |
|---|---|---|
| `asian` | enumerator | Human demographic archetype — Asian. |
| `black` | enumerator | Human demographic archetype — Black. |
| `caucasian` | enumerator | Human demographic archetype — Caucasian. |
| `hispanic` | enumerator | Human demographic archetype — Hispanic. |
| `alien` | enumerator | Non-human or stylised character archetype. |
| `other` | enumerator | Catch-all for archetypes not covered by the named values. |

### Relationships

- `Gender` — companion demographic attribute defined in the same header
- `Configuration` — consuming header that imports this enum

<!-- ink:api-end name="Archetype" -->

<!-- ink:api name="Gender" module="dna/layers/Descriptor" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Gender`

Specifies the gender of a character for DNA rig selection.

### Why this exists

Separating gender from archetype avoids a combinatorial explosion of a single enum while keeping both demographic axes as first-class, type-safe identifiers. The `other` value handles non-binary and unspecified cases without requiring a nullable or sentinel type.

### Fields

| Name | Type | Description |
|---|---|---|
| `male` | enumerator | Male gender designation. |
| `female` | enumerator | Female gender designation. |
| `other` | enumerator | Non-binary or unspecified gender. |

### Relationships

- `Archetype` — companion enum defined in the same header; used together for full demographic identification
- `Configuration` — header imported alongside this definition

<!-- ink:api-end name="Gender" -->
