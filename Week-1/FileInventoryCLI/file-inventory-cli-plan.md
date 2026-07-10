# File Inventory CLI — Implementation Plan

## 1. Project Structure & Build Setup

- Organize into logical modules, even before writing code:
  - `main` — CLI entry point, argument parsing, orchestration.
  - `scanner` — directory traversal logic.
  - `filter` — include/exclude path matching logic.
  - `aggregator` — grouping and summing logic.
  - `report` — text/JSON output formatting.
  - `tests` — unit tests for filters and aggregation.
- Decide on a build system (CMake is the natural choice for C++17 + `std::filesystem`, since some toolchains need explicit linking of `stdc++fs` on older GCC).
- Set up a `tests/` target separate from the main executable so unit tests can run independently.

## 2. Define Core Data Structures

- **FileRecord**: represents one scanned file — path, extension, size in bytes, maybe last-modified time.
- **FilterConfig**: holds include patterns, exclude patterns, and any depth limits.
- **SummaryKey**: a variant/enum describing the grouping mode — by extension, by size bucket, or by folder.
- **SummaryEntry**: aggregated result per key — count of files, total bytes, maybe min/max size.
- **ReportFormat**: enum for `PlainText` vs `Json`.

Keep these as simple value types (structs) so they're easy to copy, compare in tests, and reason about without hidden state.

## 3. CLI Argument Parsing

- Decide the argument surface:
  - Root directory to scan (positional, required).
  - `--include <pattern>` (repeatable).
  - `--exclude <pattern>` (repeatable).
  - `--group-by <extension|size|folder>`.
  - `--format <text|json>`.
  - `--output <file>` (optional; default to stdout).
  - `--max-depth <n>` (optional).
- Parse into a `CliOptions` struct up front, validate early (e.g., root path exists and is a directory), and fail fast with a clear error message and non-zero exit code if invalid.
- Keep parsing logic separate from business logic so tests can construct `CliOptions` directly without invoking the CLI.

## 4. Path Filtering Logic

- Design filter matching as a pure function: given a path (relative to scan root) and a `FilterConfig`, return true/false for "should this be included."
- Matching approach:
  - Support glob-like patterns (`*.txt`, `build/*`) or simple substring/extension matches — decide scope up front (recommend starting with extension + simple wildcard, not full regex).
  - Exclude rules take precedence over include rules (or define and document whichever precedence you choose, consistently).
  - If no include patterns are specified, default to "include everything" then apply excludes.
- Keep this logic free of any filesystem I/O — it should operate purely on path strings/`std::filesystem::path` objects, which makes it trivially unit-testable.

## 5. Directory Traversal (Scanner)

- Use `std::filesystem::recursive_directory_iterator` as the traversal mechanism.
- For each directory entry:
  - Skip entries that throw or are inaccessible (permission errors) — catch and log a warning rather than aborting the whole scan.
  - Determine if it's a regular file (skip symlinks-to-nowhere, directories themselves, special files).
  - Extract path, extension (normalize case if desired), and size via `directory_entry::file_size()` wrapped in error handling (some files may vanish mid-scan or be inaccessible).
  - Apply the max-depth limit if configured (track depth manually since `recursive_directory_iterator` doesn't restrict depth natively — compare path segment counts relative to root).
  - Run the path filter logic from Step 4; skip non-matching entries.
  - On success, construct a `FileRecord` and pass it to the aggregation step.
- Return either a `std::vector<FileRecord>` or, for large trees, consider streaming records directly into the aggregator to avoid holding everything in memory — decide based on expected scale.
- Wrap filesystem calls that can throw (`filesystem_error`) in try/catch at the granularity of "per entry," so one bad file doesn't kill the whole scan.

## 6. Aggregation Logic

- Design a generic aggregator that takes a `FileRecord` and a grouping strategy, and updates a `std::map<SummaryKey, SummaryEntry>` (map keeps deterministic ordering for output; unordered_map if performance matters more and you sort at report time instead).
- Grouping strategies:
  - **By extension**: key = lowercase extension (or "no extension" bucket).
  - **By size bucket**: define bucket boundaries (e.g., 0–4KB, 4KB–1MB, 1MB–100MB, 100MB+) and map a size to a bucket label.
  - **By folder**: key = immediate parent directory (or top-level directory relative to scan root — decide and document).
- Keep the aggregation function pure/stateless where possible: input = existing map + new record + grouping mode, output = updated map. This makes it easy to unit test without touching the filesystem.
- Also accumulate a grand total (overall file count, overall byte count) separately for the report header.

## 7. Report Generation

- **Plain text**:
  - Print a header (root path scanned, grouping mode, timestamp).
  - Print each group: label, file count, total size (human-readable, e.g., KB/MB/GB conversion).
  - Print totals at the end.
  - Sort groups (alphabetically, or descending by size/count — pick one and document it).
- **JSON**:
  - Decide on a schema up front, e.g.:
    - `root`, `groupBy`, `generatedAt`
    - `groups`: array of `{ key, fileCount, totalBytes }`
    - `totals`: `{ fileCount, totalBytes }`
  - Either hand-roll minimal JSON serialization (safe since the data shape is simple and fixed) or pull in a lightweight header-only JSON library — decide based on dependency tolerance.
- Both formatters take the same aggregated summary map as input, so adding a new format later doesn't touch scanning/filtering/aggregation logic at all.
- Write to stdout by default; if `--output` is given, open an `ofstream` and write there, with error handling if the path isn't writable.

## 8. Orchestration (main)

- Sequence:
  1. Parse CLI args → `CliOptions`.
  2. Validate root path.
  3. Run scanner with filter config → collect/stream `FileRecord`s.
  4. Feed records into aggregator with chosen grouping mode → summary map.
  5. Pass summary map + totals to the chosen report formatter.
  6. Write output (stdout or file).
  7. Return appropriate exit code (0 success, non-zero on fatal errors like invalid root path).
- Keep `main` thin — it should just wire the pieces together, since all real logic lives in testable functions/classes.

## 9. Error Handling Strategy

- Distinguish between:
  - **Fatal errors** (bad CLI args, root path doesn't exist) → print error, exit non-zero immediately.
  - **Recoverable per-entry errors** (permission denied on one file, broken symlink) → log a warning to stderr, skip the entry, continue scanning.
- Use `std::error_code` overloads of `std::filesystem` functions where available to avoid exceptions in the common "file vanished" case, reserving try/catch for truly exceptional conditions.

## 10. Unit Testing Plan

- Choose a lightweight framework (e.g., Catch2 or doctest) that integrates easily with CMake.
- **Filter tests**:
  - Include-only patterns match expected files.
  - Exclude patterns override includes.
  - No patterns → everything included.
  - Edge cases: files with no extension, hidden files (dotfiles), case sensitivity.
- **Aggregation tests**:
  - Grouping by extension produces correct counts/sums for a small synthetic set of `FileRecord`s.
  - Grouping by size bucket places boundary values in the correct bucket (test off-by-one at bucket edges).
  - Grouping by folder correctly extracts parent directory across nested paths.
  - Empty input produces an empty summary with zero totals.
- **Report tests** (optional but useful):
  - Given a fixed summary map, plain text output contains expected lines.
  - JSON output is valid JSON and matches expected structure (can parse it back and check fields).
- Since filter and aggregation logic don't touch the real filesystem, these tests run fast and deterministically — no need to spin up temp directories for most of them. Reserve a small number of integration-style tests (using `std::filesystem::create_directories` in a temp dir) to validate the scanner itself end-to-end.

## 11. Incremental Build Order

A sensible order to actually implement and test as you go:

1. Data structures (`FileRecord`, `FilterConfig`, `SummaryEntry`, etc.)
2. Filter logic + unit tests
3. Aggregator logic + unit tests
4. Scanner (wire up traversal, depend on filter)
5. Report formatters (text first, then JSON)
6. CLI parsing
7. `main` orchestration
8. End-to-end manual testing on a real directory tree, then a couple of integration tests

This order front-loads the parts that are pure logic and easiest to test, and defers I/O-heavy and glue code until the underlying logic is verified.
