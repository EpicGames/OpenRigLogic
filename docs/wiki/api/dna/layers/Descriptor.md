# API Reference — `dna/layers/Descriptor`

---

<!-- ink:api name="Archetype" module="dna/layers/Descriptor" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `Archetype`

Character archetype stored in the DNA descriptor.

### Why this exists
DNA assets carry per-archetype rig data such as blend shape weights and joint ranges. Using a strongly-typed enum prevents raw integer or string mismatches at call sites and ensures only valid archetype identifiers reach the configuration system. The `alien` and `other` values extend coverage beyond human demographics to stylised and non-human characters.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `asian` | enumerator | Asian archetype. |
| `black` | enumerator | Black archetype. |
| `caucasian` | enumerator | Caucasian archetype. |
| `hispanic` | enumerator | Hispanic archetype. |
| `alien` | enumerator | Alien archetype. |
| `other` | enumerator | Any other archetype. |

### Construction

```cpp
dna::Archetype archetype = dna::Archetype::caucasian;
```

### Relationships

- `Gender` — declared alongside it in the same header
- `Gender` — companion demographic attribute defined in the same header
- `Configuration` — consuming header that imports this enum
<!-- ink:api-end name="Archetype" -->

<!-- ink:api name="Gender" module="dna/layers/Descriptor" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `Gender`

Character gender stored in the DNA descriptor.

### Why this exists
Separating gender from archetype avoids a combinatorial explosion of a single enum while keeping both demographic axes as first-class, type-safe identifiers. The `other` value handles non-binary and unspecified cases without requiring a nullable or sentinel type.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `male` | enumerator | Male. |
| `female` | enumerator | Female. |
| `other` | enumerator | Any other value. |

### Construction

```cpp
dna::Gender gender = dna::Gender::female;
```

### Relationships

- `Archetype` — declared alongside it in the same header
- `Archetype` — companion enum defined in the same header; used together for full demographic identification
- `Configuration` — header imported alongside this definition
<!-- ink:api-end name="Gender" -->
