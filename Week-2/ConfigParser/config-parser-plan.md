# Week 2: Config Parser & Value-Oriented Refactor

## Goals

- Practice returning value-oriented result types instead of mutable output parameters.
- Use `std::variant` for parse results and `if constexpr` to simplify generic code branches.
- Improve interfaces to be easier to test and reuse.

## Overview

This week introduces a small configuration file parser as a bridge from basic C++17 value types toward later C++23 `std::expected`-style error handling. It starts with `std::variant<Success, Error>` as the result type and is designed so it can later be refactored into a more expressive result API without a full rewrite.

The parser reads a lightweight config format (sections, key-value pairs, comments) and returns typed values with explicit success or failure paths — no exceptions used for control flow, no output parameters, no manual null checks leaking into calling code.

## Tasks

- Refactor Week 1 utilities to remove manual null checks and error-prone string ownership patterns.
- Build a parser that returns `std::variant<Success, Error>` (or nested `std::optional` where appropriate) instead of writing into caller-supplied output arguments.
- Add unit tests covering parsing, path filtering, and aggregation logic.

## Features

- Support for sections, key-value pairs, comments, and whitespace trimming.
- Parsing of integers, booleans, strings, and arrays.
- Structured parse errors that include line numbers, rather than opaque failure states.
- A conversion layer that maps raw parsed values into typed application settings.

## Module Structure

The work is organized into a few cooperating pieces:

- **Value types** — the core result and error types used throughout (parsed values, parse errors, parsed document model).
- **Tokenizer** — turns raw config text into a stream of tokens, itself returning results by value.
- **Parser** — consumes tokens and builds a structured document, or returns a structured error.
- **Settings conversion layer** — maps the parsed document into strongly-typed application settings, with its own distinct error type for semantic (as opposed to syntax) problems.
- **Week 1 utility refactors** — existing path-filtering and aggregation utilities updated to follow the same value-returning, output-parameter-free style.

## Testing Plan

Unit tests are organized by component:

- Tokenizer correctness, including error line/column reporting.
- Parser correctness across well-formed and malformed input, including edge cases like empty input and duplicate keys.
- Path filtering behavior across various filter conditions and empty input.
- Aggregation correctness across typical and edge-case input sets.
- Settings conversion, including missing-key and type-mismatch error handling.

## Build Order

1. Core value and error types.
2. Tokenizer, tested in isolation.
3. Parser, built on top of the tokenizer.
4. Document lookup/convenience helpers.
5. Settings conversion layer.
6. Week 1 utility refactors (done in parallel, as they're independent of the parser).
7. Integration into the CLI, replacing any ad hoc config handling with the new parser and converter.
