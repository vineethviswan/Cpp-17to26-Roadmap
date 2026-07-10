# C++17 to C++26 Weekly Learning Plan

This document lays out a week-by-week plan for a C++ developer who is already comfortable with C++17 and wants to catch up through C++20, C++23, and C++26 in a practical way. C++20 is the largest jump after C++17 because it adds concepts, ranges, coroutines, modules, and broader compile-time capabilities, while C++23 improves the standard library with features such as `std::expected`, `std::print`, and `std::mdspan`, and C++26 adds major new directions such as reflection, contracts, and the `std::execution` async model.[cite:26][cite:61][cite:33][cite:31]

The plan is organized around short projects because newer C++ features become useful only when they are applied to code structure, APIs, and tooling. Each week includes clear tasks, one main output, and an example project so the learning path stays grounded in real code instead of feature memorization.[cite:26][cite:61]

## How to Use This Plan

- Spend 5 to 7 hours per week on focused study, coding, and cleanup.
- Keep one repository for all exercises, with one folder per week.
- At the end of each week, write a short note: what changed, what felt useful, and what still feels unclear.
- Use cppreference as the primary reference for language and library details, and verify C++26 availability using compiler support pages before attempting newer features in your main toolchain.[cite:17][cite:31]

## Weekly Schedule

### Week 1 — Refresh modern C++17 style

**Goals**
- Review structured bindings, `if constexpr`, `std::optional`, `std::variant`, `std::string_view`, and filesystem support.[cite:16]
- Identify old habits to stop using, especially raw status codes, over-copying strings, and custom tagged unions where library types are clearer.[cite:16]
- Set up a build matrix for GCC, Clang, or MSVC with explicit standard flags.

**Tasks**
- Create a repo with `week01` through `week16` folders.
- Write small snippets using `std::optional`, `std::variant`, and structured bindings.
- Convert one older helper function to use `std::string_view` and one file operation to use `std::filesystem`.[cite:16]

**Example project**
- A file inventory CLI that walks a directory tree and reports file counts, extensions, and total size using `std::filesystem`.[cite:16]

### Week 2 — Safer APIs in C++17

**Goals**
- Practice returning value-oriented result types instead of mutable output parameters.
- Use `std::variant` for parse results and `if constexpr` to simplify generic code branches.[cite:16]
- Improve interfaces to be easier to test and reuse.

**Tasks**
- Refactor Week 1 utilities to reduce manual null checks and error-prone string ownership.
- Build a parser that returns `std::variant<Success, Error>` or nested `std::optional` where appropriate.[cite:16]
- Add unit tests for parsing, path filtering, and aggregation.

**Example project**
- A config parser for INI-like files with typed values and clean error reporting using `std::variant`.[cite:16]

### Week 3 — Enter C++20 with concepts

**Goals**
- Learn concepts and constrained templates, one of the largest language improvements in C++20.[cite:26]
- Replace unconstrained templates with clearer requirements and better diagnostics.[cite:26]
- Start thinking in terms of API contracts expressed at compile time.

**Tasks**
- Write two to three custom concepts for sortable, printable, or range-like types.
- Refactor one generic algorithm from earlier weeks to use `requires` clauses.[cite:26]
- Compare compiler diagnostics before and after applying concepts.

**Example project**
- A header-only utility library with constrained algorithms for searching, filtering, and formatting containers.[cite:26]

### Week 4 — Ranges and views

**Goals**
- Learn the C++20 ranges library and its view/adaptor model.[cite:43]
- Replace manual loops with composable pipelines where it improves readability.[cite:43]
- Distinguish between owning containers and lazy views.

**Tasks**
- Practice `filter`, `transform`, `take`, and `drop` on vectors and strings.[cite:43]
- Rewrite a parser or reporting pipeline from Week 1 or 2 using ranges.[cite:43]
- Benchmark one loop-based implementation against a ranges-based implementation for understanding, not for premature optimization.

**Example project**
- A log analyzer that loads lines, filters warnings/errors, extracts fields, and summarizes counts through range pipelines.[cite:43]

### Week 5 — `std::span`, comparisons, and utility features

**Goals**
- Learn `std::span` as a non-owning view over contiguous data, which is one of the most practical C++20 tools for API cleanup.[cite:26]
- Practice the three-way comparison operator `<=>` and selected utility additions from C++20.[cite:26]
- Improve function signatures to be more generic without losing safety.

**Tasks**
- Replace raw pointer-plus-length interfaces with `std::span` where appropriate.[cite:26]
- Add comparison support to one custom type using defaulted or explicit `<=>`.[cite:26]
- Review where `constexpr` and `consteval` can remove runtime work from helper code.[cite:26]

**Example project**
- A binary packet inspector that parses fixed-layout records through `std::span<std::byte>` and compares packets by structured fields.[cite:26]

### Week 6 — Coroutines fundamentals

**Goals**
- Understand what coroutines are in C++20 and how suspend/resume behavior differs from threads.[cite:42]
- Learn the basic promise/awaiter model at a practical level, enough to read and use coroutine-based code.[cite:42]
- Build a simple generator-style coroutine before attempting async workflows.

**Tasks**
- Implement a minimal generator that yields integers or lines of text.[cite:42]
- Trace execution manually to understand suspension points.
- Read a small coroutine example and annotate where state is preserved.

**Example project**
- A lazy sequence generator for line streaming, Fibonacci generation, or token production.[cite:42]

### Week 7 — Coroutines in an async workflow

**Goals**
- Move from toy coroutines to a practical async flow.[cite:42]
- Learn where coroutines help readability and where they are not worth the complexity.[cite:42]
- Structure cancellation, timeouts, and result passing clearly.

**Tasks**
- Wrap coroutine results in a small task abstraction or use an existing learning-friendly helper library.
- Model success and failure paths explicitly.
- Add logging for start, suspend, resume, and completion events.

**Example project**
- A coroutine-based downloader or asset fetch simulator that queues jobs and resumes them as data becomes available.[cite:42]

### Week 8 — Threads, stop tokens, and structured cancellation

**Goals**
- Learn `std::jthread` and cooperative cancellation patterns in C++20.[cite:26]
- Compare thread-based concurrency with coroutine-based flow so the trade-offs are concrete.[cite:26][cite:42]
- Practice clean shutdown and cancellation-aware loops.

**Tasks**
- Build a worker loop using `std::jthread` and stop tokens.[cite:26]
- Add cancellation to the Week 7 project or to a separate background processing demo.[cite:26]
- Document when a thread is simpler than a coroutine for your style of projects.

**Example project**
- A task runner that processes queued work on background threads with cooperative stop requests.[cite:26]

### Week 9 — Modules and build organization

**Goals**
- Learn the purpose of modules in C++20 and how they aim to improve isolation and build structure.[cite:26]
- Understand that module support still varies by compiler and build tooling, so this is partly a tooling exercise.[cite:31]
- Reorganize code around explicit exported interfaces.

**Tasks**
- Convert a small utility library to one or more modules.[cite:26]
- Compare compile behavior and include hygiene with the header-based version.
- Record compiler or IDE limitations in your current environment.[cite:31]

**Example project**
- A modular math, string, or utility package exported through C++20 module interfaces.[cite:26]

### Week 10 — C++23 error handling with `std::expected`

**Goals**
- Learn `std::expected` as a first-class result type for success-or-error APIs in C++23.[cite:61]
- Replace exception-heavy or boolean-error APIs where explicit composition improves clarity.[cite:61]
- Practice keeping error types meaningful and lightweight.

**Tasks**
- Refactor one parser, loader, or downloader API to return `std::expected<T, E>`.[cite:61]
- Create helper functions for chaining operations and mapping errors.
- Compare readability between exception-based and expected-based code.

**Example project**
- A resource loader that validates paths, parses data, and returns structured failures using `std::expected`.[cite:61]

### Week 11 — C++23 output and diagnostics

**Goals**
- Learn newer C++23 library additions for printing and diagnostics, especially `std::print` and `std::stacktrace` where available.[cite:61][cite:56]
- Improve developer-facing tooling, logs, and debugging output.[cite:61][cite:56]
- Build reusable formatting helpers instead of ad hoc streams everywhere.

**Tasks**
- Replace selected `std::cout` formatting code with `std::print` or equivalent availability-tested paths.[cite:61][cite:56]
- Capture and display stack traces in an error-reporting demo where supported.[cite:56]
- Add command-line switches for verbose logging, quiet mode, and debug output.

**Example project**
- A diagnostics CLI that prints structured system or app state and emits stack traces on controlled failures.[cite:56][cite:61]

### Week 12 — `std::mdspan` and data-oriented design

**Goals**
- Learn `std::mdspan` as a non-owning multidimensional view, effectively a multidimensional counterpart to `std::span`.[cite:47]
- Practice separating storage from indexing/view logic.[cite:47]
- Apply this to grids, matrices, images, or simulation buffers.

**Tasks**
- Build a small 2D or 3D data wrapper around a flat buffer plus `std::mdspan`.[cite:47]
- Implement slicing, indexing, and a few transformations.
- Compare clarity against a hand-written row/column indexing approach.

**Example project**
- A grayscale image processor or tile-map manipulator using `std::mdspan` over a flat vector buffer.[cite:47]

### Week 13 — Consolidation project with C++20 and C++23

**Goals**
- Combine the most practical newer features into one medium-size program.[cite:26][cite:61]
- Prefer features that improve design, not features included just to “tick a box.”
- Practice writing documentation and tests around language-driven architecture choices.

**Tasks**
- Choose one prior project and rebuild it using concepts, ranges, `std::span`, and `std::expected`.[cite:26][cite:43][cite:61]
- Add logging through `std::print` if supported.[cite:61]
- Write a README explaining what each feature improved.

**Example project**
- A small asset indexing tool or game-data pipeline utility using modern parsing, filtering, and error propagation.[cite:26][cite:43][cite:61]

### Week 14 — C++26 reflection

**Goals**
- Explore static reflection, one of the headline C++26 additions.[cite:33]
- Learn the idea of compile-time introspection and where it can remove boilerplate in serializers, inspectors, bindings, and code generators.[cite:33][cite:35]
- Treat this week as exploration because compiler support is still evolving.[cite:31]

**Tasks**
- Read at least two reflection examples and identify what boilerplate they eliminate.[cite:33][cite:35]
- Attempt a tiny reflected type inspector using whichever compiler support is available.[cite:31]
- Keep a fallback non-reflection implementation for comparison.

**Example project**
- A reflection-driven debug printer or serializer that enumerates fields of selected user-defined types.[cite:33][cite:35]

### Week 15 — C++26 contracts

**Goals**
- Learn the design goals of contracts in C++26, including preconditions, postconditions, and contract assertions.[cite:33]
- Distinguish contracts from normal validation and from internal asserts.[cite:33]
- Explore where contracts improve API boundaries in libraries and application code.

**Tasks**
- Annotate a small set of APIs with precondition/postcondition intent, even if toolchain support is incomplete.[cite:33][cite:31]
- Build a validation-heavy example where contract style is clearly beneficial.
- Write notes on which checks belong at boundaries and which belong in ordinary logic.

**Example project**
- A geometry or financial-calculation library with explicit assumptions and postconditions around key functions.[cite:33]

### Week 16 — C++26 execution and final capstone

**Goals**
- Explore the `std::execution` direction in C++26, including sender/receiver-style asynchronous composition.[cite:33][cite:38]
- Understand the core vocabulary: sender, receiver, scheduler, operation state, and composition.[cite:33]
- Finish with a capstone that reflects what is practical today and what is emerging for later adoption.

**Tasks**
- Read a high-level overview of sender/receiver and sketch how it differs from futures or manual callback chains.[cite:33]
- Build a prototype task pipeline abstraction or a simplified job graph.
- Write a final report on which features are ready for production in your environment and which are better kept experimental.[cite:31]

**Example project**
- A small asset-processing or job-execution pipeline that models work as composable operations and cancellation-aware stages.[cite:33][cite:38]

## Suggested Weekly Deliverables

Each week should produce three concrete outputs:
- One runnable program or isolated prototype.
- One markdown note summarizing what was learned and where the feature fits best.
- One cleanup commit that improves naming, tests, or structure instead of adding new features.

This pattern keeps the learning practical and prevents the plan from becoming a list of disconnected experiments. It also makes the final repository a useful portfolio piece rather than a folder of throwaway snippets.

## Appendix: Example Projects Explained

### 1. File inventory CLI

This project scans a directory tree and produces a summary grouped by file extension, size bucket, or folder. It is useful early because it naturally exercises `std::filesystem`, structured bindings, optional metadata handling, and clean value-returning helpers without requiring advanced build setup.[cite:16]

Suggested features:
- Recursive scan with include/exclude filters.
- Aggregation by file type, count, and total bytes.
- Optional JSON or plain text report.
- Unit tests for path filters and summary logic.

### 2. INI-like config parser

This parser reads a small configuration format and returns typed values with explicit success or failure paths. It is a good bridge from basic C++17 value types to later C++23 `std::expected`-style error handling because it starts with `std::variant` and can later be refactored into a more expressive result API.[cite:16][cite:61]

Suggested features:
- Support sections, key-value pairs, comments, and whitespace trimming.
- Parse integers, booleans, strings, and arrays.
- Return structured parse errors with line numbers.
- Add a conversion layer that maps raw parsed values into application settings.

### 3. Header-only constrained algorithms library

This project collects small generic algorithms and rewrites them with concepts. It is useful because concepts immediately show their value through clearer function intent and better compiler errors when the wrong type is passed.[cite:26]

Suggested features:
- Search, filter, join, or pretty-print helpers.
- Custom concepts for sortable ranges, printable elements, or mappable containers.
- Side-by-side comparison of unconstrained versus constrained templates.
- Tests that intentionally fail to verify diagnostics.

### 4. Log analyzer with ranges

This analyzer reads log lines and transforms them through a pipeline of filters and projections. It is one of the best exercises for ranges because the problem is naturally sequential and expressive when written as chained views rather than nested loops.[cite:43]

Suggested features:
- Filter by severity, timestamp, or module.
- Extract tokens and aggregate counts.
- Output top errors or suspicious patterns.
- Compare eager materialization with lazy views.

### 5. Binary packet inspector

This project uses `std::span` over byte buffers to parse fixed or semi-structured records. It is especially relevant if you work close to systems, protocols, file formats, or game data because it teaches non-owning views, safe slicing, and carefully defined boundaries between raw memory and typed interpretation.[cite:26]

Suggested features:
- Read a binary file into memory and inspect headers.
- Create packet views over subranges.
- Add comparisons using `<=>` for packet ordering or equality checks.[cite:26]
- Export packet summaries as text.

### 6. Coroutine-based downloader or asset fetch simulator

This project models asynchronous work without deeply committing to a networking stack. It is practical because coroutines can be learned first through simulated waits, staged progress, and resumable tasks before introducing real sockets or HTTP libraries.[cite:42]

Suggested features:
- Queue multiple requests.
- Simulate network delay and partial progress.
- Support cancellation, retries, and completion callbacks.
- Add logging around suspend and resume points.

### 7. Background task runner with `std::jthread`

This project focuses on thread lifetime and cooperative cancellation instead of coroutine mechanics. It helps clarify where traditional concurrency is still the simpler choice, especially for long-running worker threads or producer-consumer designs.[cite:26]

Suggested features:
- Worker queue with one or more processing threads.
- Stop-token-aware polling loop.[cite:26]
- Status reporting for queued, running, and completed tasks.
- Graceful shutdown and resource cleanup.

### 8. Modular utility package

This project reorganizes an earlier library into module interface units and implementation units. Its value is as much about tooling and codebase structure as about syntax because modules are meant to reduce include overhead and improve boundaries, even though ecosystem support still varies.[cite:26][cite:31]

Suggested features:
- One exported utility module and one internal support module.
- Rebuild an earlier parser or analyzer on top of the module version.
- Compare build ergonomics against the header-only version.
- Note compiler-specific limitations in your environment.[cite:31]

### 9. Resource loader with `std::expected`

This project loads files, validates content, and produces either a parsed value or a structured error. It is a strong real-world use case for `std::expected` because the code usually has multiple failure points that should remain explicit and composable.[cite:61]

Suggested features:
- File existence and permission checks.
- Parse-and-validate workflow with meaningful error types.
- Error propagation helpers.
- Tests for failure scenarios.

### 10. Diagnostics CLI with `std::print` and stack traces

This tool focuses on developer experience. Modern printing and stack traces are not the flashiest additions, but they are useful because they improve day-to-day diagnostics, logging, and failure analysis in tools and services.[cite:56][cite:61]

Suggested features:
- Subcommands for environment, config, and runtime checks.
- Colorized or structured terminal output if the platform supports it.
- Optional stacktrace dump on controlled failure paths.[cite:56]
- Quiet and verbose modes.

### 11. Image or tile-map processor with `std::mdspan`

This project uses a flat vector as storage and `std::mdspan` to expose multidimensional indexing. It is ideal for grid-heavy applications such as games, image processing, scientific tools, and map editors because it separates layout concerns from algorithms that walk rows, columns, or tiles.[cite:47]

Suggested features:
- 2D indexing and bounds-aware wrappers.
- Blur, threshold, flood-fill, or tile transformation operations.
- Alternate layouts or views for subregions.
- Benchmarks comparing manual indexing and view-based access.

### 12. Reflection-driven debug printer or serializer

This project targets the future-facing part of the roadmap. The core idea is to generate structured output from a type definition without manually writing repetitive field-walking code, which is one of the clearest reflection use cases described for C++26.[cite:33][cite:35]

Suggested features:
- Enumerate fields of selected types where compiler support allows it.[cite:31]
- Emit debug text, JSON-like output, or editor-inspector views.
- Compare reflected output with hand-written serializers.
- Keep the code isolated in an experimental folder because toolchain support is still evolving.[cite:31]

### 13. Contract-annotated validation library

This project demonstrates where contracts help: expressing assumptions and guarantees at API boundaries. It is most useful when the functions have clear domain rules, such as valid ranges, normalization requirements, or postconditions that should always hold after computation.[cite:33]

Suggested features:
- Geometry, finance, parsing, or unit-conversion functions with clear constraints.
- Runtime validation fallback when native contract support is unavailable.[cite:31]
- Tests that distinguish caller mistakes from internal logic bugs.
- Notes on what belongs in contracts versus normal defensive code.

### 14. Sender/receiver-style job pipeline

This capstone explores the C++26 async direction by modeling work as composable operations rather than isolated callbacks or thread launches. Even if the full standard implementation is not yet available in your compiler, the vocabulary and design model are worth learning because they point toward how generic async composition is being standardized.[cite:33][cite:38][cite:31]

Suggested features:
- Task stages such as load, transform, validate, and save.
- Explicit success, error, and cancellation paths.[cite:33]
- Scheduler abstraction or simplified executor handle.[cite:33]
- Final write-up on what to adopt now versus later in production.[cite:31]
