# Week 3 — C++20 Concepts: Constrained Algorithms Library

## Implementation Plan

---

## 1. Project Summary

This week's project is a **header-only C++20 utility library** that provides a small set of algorithms — searching, filtering, and formatting — over containers, where every algorithm's requirements are expressed explicitly through **concepts** instead of implicit, unconstrained template parameters.

The point of the project isn't the algorithms themselves (they're deliberately simple); it's the exercise of **designing API contracts at compile time**. Each function should read like a specification: what kind of type is required, what operations that type must support, and what happens (in the form of a clear compiler error) when a caller violates the contract.

The deliverable is a single include-only library (no `.cpp` files, no linking step) that can be dropped into any C++20 project with `#include`.

### Why this matters
- Unconstrained templates in C++17 produce cryptic, multi-page error messages when misused — the failure surfaces deep inside the implementation rather than at the call site.
- Concepts move the failure to the point of instantiation, with a message that names the actual violated requirement.
- This is a shift from "templates as duck-typing" to "templates as documented contracts," which is the core mental model C++20 is pushing toward.

---

## 2. Goals Mapping

| Weekly goal | How the project satisfies it |
|---|---|
| Learn concepts and constrained templates | Library is built entirely around 2–3 hand-written concepts |
| Replace unconstrained templates with clearer requirements | Every algorithm uses `requires` clauses instead of bare `typename T` |
| Think in terms of compile-time API contracts | Each concept doubles as documentation of what a type must provide to be usable with the library |

---

## 3. Concepts to Design

Three concepts, each mapping to one category of algorithm:

### 3.1 `Sortable`
Describes a type that can be ordered and swapped — the minimum requirement for anything touched by a comparison-based algorithm (search, sort-dependent filtering).

Conceptual requirements (no code, just contract):
- Supports `<` (or a three-way comparison) between two instances
- Is swappable / movable

### 3.2 `Printable` (or `Formattable`)
Describes a type that can be converted to a textual representation — required by the formatting algorithms.

Conceptual requirements:
- Supports stream insertion (`operator<<`) into an output stream, **or**
- Is explicitly convertible to `std::string`

Worth deciding early which of the two you standardize on, since it changes how the formatting module is written.

### 3.3 `RangeLike`
Describes a container-like type that the algorithms can iterate over generically, rather than hardcoding `std::vector` everywhere.

Conceptual requirements:
- Exposes `begin()` / `end()`
- The iterator type supports increment and dereference
- (Optionally) exposes `size()` for algorithms that want to reserve capacity

### Composition
`RangeLike` is the "container" concept; `Sortable` and `Printable` describe the **element type** inside that container. Algorithms will typically combine both, e.g. "a range-like container whose value type is sortable."

---

## 4. Library Structure

Because this is header-only, the "modules" are conceptual groupings of headers rather than build targets.

```
constrained-algorithms/
├── include/
│   └── calg/                     (library namespace: calg = "constrained algorithms")
│       ├── concepts.hpp          — all custom concepts live here
│       ├── search.hpp            — constrained search algorithms
│       ├── filter.hpp            — constrained filter algorithms
│       ├── format.hpp            — constrained formatting/printing algorithms
│       └── calg.hpp              — umbrella header that includes the above
├── tests/
│   └── ...                       — Catch2 test files, one per module
├── examples/
│   └── diagnostics_demo.cpp      — deliberately-broken calls used to capture before/after compiler errors
├── CMakeLists.txt
└── README.md
```

### 4.1 `concepts.hpp`
Single source of truth for `Sortable`, `Printable`, and `RangeLike`. Nothing else in the library should redefine or duplicate a requirement — every other header depends on this one.

### 4.2 `search.hpp`
Constrained equivalents of `std::find` / `std::binary_search`-style operations. Constrained on `RangeLike` + `Sortable` (or equality-comparable, if you want a looser search that doesn't require ordering).

### 4.3 `filter.hpp`
Constrained equivalent of `std::copy_if` — takes a `RangeLike` and a predicate, returns a new container of matching elements. This is a good candidate for the **refactor task** (see Section 5), since filtering is a natural function to have already written in an earlier, unconstrained week.

### 4.4 `format.hpp`
Takes a `RangeLike` whose value type satisfies `Printable` and produces a formatted string (e.g. comma-separated, or one-per-line). This module is where the `Printable` concept gets exercised most directly.

### 4.5 `calg.hpp`
Umbrella header — consumers include just this one file to get the whole library. Keeps the public interface simple regardless of internal file layout.

---

## 5. Task-by-Task Plan

### Task A — Write 2–3 custom concepts
1. Draft the informal contract for each concept in plain language first (as in Section 3) before writing any `requires` expression.
2. Implement in `concepts.hpp`.
3. Sanity-check each concept in isolation with a couple of throwaway types (one that should satisfy it, one that shouldn't) before wiring it into any algorithm.

### Task B — Refactor a generic algorithm from an earlier week to use `requires`
1. Pick one algorithm from the earlier (C++17-era) work — filtering is the most natural fit given the container-processing theme.
2. Port it into `filter.hpp` as-is first, unconstrained, to confirm it still behaves correctly under C++20.
3. Add a `requires` clause using the concepts from Task A.
4. Note in the README (or a short "before/after" section) what changed structurally — not just the added clause, but whether the constraint revealed an implicit assumption the original code was making.

### Task C — Compare compiler diagnostics before and after
1. Keep an unconstrained copy of the refactored algorithm (or use a feature flag / separate branch) so both versions can be compiled against the same bad input.
2. In `examples/diagnostics_demo.cpp`, deliberately call each algorithm with a type that violates its contract (e.g. pass a non-comparable type into the search algorithm).
3. Capture the actual compiler output for both versions side by side.
4. Summarize the comparison (e.g. line count of the error, whether the failure point is the call site or buried in the implementation, whether the message names the unmet requirement).

---

## 6. Testing Approach

Consistent with the existing toolchain: **CMake + Catch2**.

- One test file per module (`test_search.cpp`, `test_filter.cpp`, `test_format.cpp`).
- Tests should cover:
  - Correct behavior on types that satisfy the concepts.
  - That the library still compiles cleanly for multiple different container types (e.g. `std::vector`, `std::array`) to confirm `RangeLike` isn't accidentally narrowed to one container.
- Concept *violations* aren't something Catch2 can assert on directly (they're compile errors) — those are handled separately via the diagnostics comparison in Task C, not as part of the normal test suite. A `static_assert(!SomeConcept<BadType>)` block is a lightweight way to still get automated confirmation that a concept correctly rejects a bad type.

---

## 7. Build Setup Notes

- Header-only means the CMake target is an `INTERFACE` library — no compiled `.cpp` sources for `calg` itself.
- Set the C++ standard explicitly to C++20 (`cxx_std_20`) on the target, since concepts require it.
- Keep the test executable and the (optional) diagnostics-demo executable as separate CMake targets from the interface library, mirroring the multi-target layout from the earlier CLI project.

---

## 8. Suggested Order of Work

1. Write informal contracts for the three concepts (plain language, no code).
2. Implement `concepts.hpp` and validate each concept against throwaway pass/fail types.
3. Build `search.hpp` and `format.hpp` from scratch using the concepts.
4. Port and refactor the earlier filter algorithm into `filter.hpp`, adding `requires`.
5. Write tests for all three modules.
6. Build the diagnostics demo and capture before/after compiler output.
7. Write the README summarizing the concepts, the refactor, and the diagnostics comparison.

---

## 9. Stretch Ideas (optional, not required for the week's goals)

- Add a fourth concept for types that support both `Sortable` and `Printable`, to see how concept composition/combination reads in practice.
- Explore `requires` expressions with compound requirements (e.g. constraining not just that an operation exists, but that it returns a specific type) once the basic contracts are working.
