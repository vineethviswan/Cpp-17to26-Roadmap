# Week 5 — Binary Packet Inspector: Implementation Plan

**Theme:** `std::span`, `<=>`, `constexpr`/`consteval` (C++20)
**Output:** A CLI that loads a binary file, parses fixed-layout packet records through non-owning views, and prints/compares/exports summaries.

---

## 1. Goals Mapped to the Design

| Week 5 goal | Where it shows up |
|---|---|
| Replace pointer+length with `std::span` | Every parsing function and `PacketView` take `std::span<const std::byte>` |
| `<=>` on a custom type | `PacketHeader` (defaulted) and `PacketView` (explicit) |
| `constexpr` / `consteval` | Header layout constants, field offsets, checksum, magic-number validation |
| Non-owning views + safe slicing | `PacketView` never owns bytes; `ByteReader` bounds-checks every read |

---

## 2. Packet Format (define this first)

A deliberately simple fixed-header format so the focus stays on spans, not protocols.

**Header (fixed size, e.g. 16 bytes)**
- `magic` (u32) — constant identifying a valid packet
- `version` (u8)
- `type` (u8) — enum-backed (Data, Control, Heartbeat, ...)
- `flags` (u16)
- `sequence` (u32)
- `payload_length` (u16)
- `checksum` (u16) — over header (minus checksum) + payload

**Body:** `payload_length` bytes, followed immediately by the next packet.

**Decisions to write down before coding**
- Endianness of the file (pick little- or big-endian; implement byte-swap in one place).
- Whether a packet may have trailing padding.
- Max payload size (used for sanity checks).

---

## 3. Project Layout (Visual Studio 2026, no CMake)

Use a single **solution** with multiple **projects**, set up through the VS IDE. Keep the same logical split as before (library / CLI / tests / sample-data tool), but each piece is a `.vcxproj` instead of a CMake target.

```
week05/
├── PacketInspector.sln
├── Directory.Build.props         # shared compiler settings (see below)
├── pktinspect_core/              # Static Library (.lib)
│   ├── pktinspect_core.vcxproj
│   ├── include/pktinspect/
│   │   ├── byte_utils.hpp        # constexpr helpers, endian handling
│   │   ├── packet_types.hpp      # enums, constants, PacketHeader
│   │   ├── byte_reader.hpp       # cursor over span
│   │   ├── packet_view.hpp       # non-owning view of one packet
│   │   ├── packet_parser.hpp     # splits buffer into PacketViews
│   │   ├── packet_summary.hpp    # value-type summaries + formatting
│   │   └── file_loader.hpp       # reads file into owning buffer
│   └── src/
│       ├── file_loader.cpp
│       ├── packet_parser.cpp
│       └── packet_summary.cpp
├── pktinspect_cli/               # Console App (.exe)
│   ├── pktinspect_cli.vcxproj
│   └── src/main.cpp
├── pktinspect_tests/             # Console App (.exe) running Catch2
│   ├── pktinspect_tests.vcxproj
│   ├── test_byte_reader.cpp
│   ├── test_packet_header.cpp
│   ├── test_packet_view.cpp
│   ├── test_parser.cpp
│   └── test_comparisons.cpp
└── make_sample_data/             # Console App (.exe), generates test .bin files
    ├── make_sample_data.vcxproj
    └── make_sample_data.cpp
```

**Solution setup steps**
1. Create a blank solution, then add the four projects above.
2. In `pktinspect_core`, set **Configuration Type = Static Library (.lib)**.
3. In `pktinspect_cli`, `pktinspect_tests` and `make_sample_data`, add a **Project Reference** to `pktinspect_core` (right-click Dependencies > Add Reference). This handles link order and build order for you.
4. For every project that includes the headers, add `$(SolutionDir)pktinspect_core\include` to **C/C++ > General > Additional Include Directories** (or put it in the shared props file below).
5. Set `pktinspect_cli` as the startup project.

**Shared compiler settings (`Directory.Build.props` or a Property Sheet)**

Put these in one place so all four projects agree; otherwise it is easy to build one project as C++17 by accident.
- **C++ Language Standard:** `/std:c++20` (or `/std:c++latest` if you want to try newer features later)
- **Warning Level:** `/W4`, and optionally **Treat Warnings As Errors** (`/WX`)
- **Conformance mode:** `/permissive-`
- **`/Zc:__cplusplus`:** so `__cplusplus` reports the real value (useful for feature checks)
- **Character Set / SDL checks:** leave SDL checks on (`/sdl`) for the learning project
- Apply to **both Debug and Release**, and to **x64** (ignore Win32 unless you need it)

**Unit test framework options (pick one)**
- **Catch2** via `vcpkg` manifest mode (`vcpkg.json` in the solution folder, enabled in project properties) or via NuGet. Matches the framework you used for the earlier CLI project.
- **Microsoft Native Unit Test project** (built into VS) if you prefer Test Explorer integration without any package manager.
- Either way, the `static_assert`-based tests live directly in the test `.cpp` files and are "run" simply by compiling.

**Where files go**
- Keep the project folders on a **local, non-cloud-synced path** (sync clients can lock `.obj`, `.pdb` and `.tlog` files mid-build and cause confusing errors).
- Add a `.gitignore` for `.vs/`, `x64/`, `Debug/`, `Release/`, `*.user`.

**Running with sample data**
- Set **Debugging > Command Arguments** on `pktinspect_cli` to something like `$(SolutionDir)samples\valid.bin --verify-checksums`.
- Set `make_sample_data`'s **Working Directory** (or an output argument) so generated `.bin` files land in `$(SolutionDir)samples\`.

---

## 4. Components

### 4.1 `byte_utils` (free functions, mostly `constexpr`)

Free-function module, no state.

**Functions**
- `read_u8 / read_u16 / read_u32(span<const byte>, offset)` → value (constexpr; assumes caller already bounds-checked)
- `swap_endian(...)` helpers if the file endianness differs from host
- `compute_checksum(span<const byte>)` → u16 (constexpr so it can be tested at compile time)
- `to_hex(span<const byte>, max_bytes)` → string (runtime only; for display)

**Notes**
- Keep the bounds-check *out* of these; that is `ByteReader`'s job. Document the precondition.
- Good place for a `static_assert` self-test of the checksum on a tiny constant array.

---

### 4.2 `packet_types`

**Constants (constexpr)**
- `kMagic`, `kHeaderSize`, `kMaxPayload`
- Field offsets: `kOffMagic`, `kOffVersion`, ... (derived from each other, not hand-typed)

**`enum class PacketType : uint8_t`**
- Values: Data, Control, Heartbeat, Unknown
- Free function `to_string(PacketType)` (constexpr, returns `string_view`)

**`consteval` candidates**
- A `consteval` function that validates the layout table at compile time (offsets contiguous, sum equals `kHeaderSize`). Compile error if you edit one field and forget another.

**`struct PacketHeader`** (plain value type, *owns* its 16 bytes of data as decoded fields)

| Member | Type |
|---|---|
| `magic` | `uint32_t` |
| `version` | `uint8_t` |
| `type` | `PacketType` |
| `flags` | `uint16_t` |
| `sequence` | `uint32_t` |
| `payload_length` | `uint16_t` |
| `checksum` | `uint16_t` |

| Method | Purpose |
|---|---|
| `auto operator<=>(const PacketHeader&) const = default` | Defaulted comparison — *member order matters*, so decide ordering semantics (sequence first?) and arrange members, or write explicit version instead |
| `bool operator==(const PacketHeader&) const = default` | Declared separately/implicitly with defaulted `<=>` |
| `static std::optional<PacketHeader> decode(span<const byte>)` | Decode from raw bytes; `nullopt` if too short |
| `bool is_valid() const` | Magic and version check (constexpr) |

**Design choice to make:** if default member-order comparison is *not* the ordering you want (e.g. you want to order by `sequence` only), this struct is where you decide between "reorder members" vs. "write explicit `<=>`". Documenting that decision is part of the week's note.

---

### 4.3 `ByteReader` (cursor over a span)

Sequential, bounds-checked reader. Replaces ad hoc `ptr + offset` arithmetic.

**Members**
- `std::span<const std::byte> data_` — the whole region (non-owning)
- `std::size_t pos_` — current read position

**Methods**
- `explicit ByteReader(span<const byte>)`
- `size_t remaining() const`
- `size_t position() const`
- `bool can_read(size_t n) const`
- `optional<uint8_t> read_u8()`, `read_u16()`, `read_u32()` — advance on success, return `nullopt` and *do not advance* on failure
- `optional<span<const byte>> read_bytes(size_t n)` — returns a **subspan** (this is the key slicing operation)
- `bool skip(size_t n)`
- `span<const byte> rest() const` — everything not yet consumed

**Notes**
- Uses `subspan` / `first` / `last` internally. This is where you get comfortable with them.
- Return type decision: `optional` now; swap to `std::expected` in Week 10 (note this in the README so the future refactor is expected).

---

### 4.4 `PacketView` (the star of the week)

A **non-owning view of one complete packet** inside a larger buffer. Cheap to copy (two words + a decoded header).

**Members**
- `PacketHeader header_` — decoded copy of the header
- `std::span<const std::byte> raw_` — header + payload, exactly this packet's bytes
- `std::size_t offset_in_stream_` — where this packet started in the source buffer (for reporting)

**Methods**
- `static optional<PacketView> from_bytes(span<const byte> region, size_t stream_offset)` — validates header, ensures `region` is long enough, trims to exact packet size
- `const PacketHeader& header() const`
- `span<const byte> raw() const`
- `span<const byte> payload() const` — `raw_.subspan(kHeaderSize)`
- `size_t total_size() const`
- `size_t stream_offset() const`
- `bool checksum_ok() const` — recomputes and compares
- `auto operator<=>(const PacketView&) const` — **explicit** implementation (see below)
- `bool operator==(const PacketView&) const`

**Comparison semantics (explicit `<=>`)**
- Primary ordering: `header_.sequence`
- Tie-break: `header_.type`, then payload bytes (lexicographic)
- Return `std::strong_ordering`
- Define `==` to be consistent with `<=>` (same fields). Write a test proving consistency.
- Do **not** compare `offset_in_stream_` — two identical packets at different positions should be equal. That is a deliberate design point to document.

**Lifetime rule (put in a comment and in the README)**
- A `PacketView` is valid only as long as the underlying buffer lives. The owner is `FileLoader`'s result or whatever buffer the caller holds.

---

### 4.5 `PacketParser`

Turns a contiguous buffer into a sequence of `PacketView`s. Stateless apart from options.

**Members**
- `ParserOptions options_` (struct below)

**`struct ParserOptions`**
- `bool stop_on_error`
- `bool verify_checksums`
- `size_t max_packets` (0 = unlimited)

**`struct ParseDiagnostic`**
- `size_t offset`
- `std::string message`
- (kind enum: TruncatedHeader, BadMagic, TruncatedPayload, BadChecksum, OversizedPayload)

**`struct ParseResult`**
- `std::vector<PacketView> packets`
- `std::vector<ParseDiagnostic> diagnostics`
- `size_t bytes_consumed`
- `size_t trailing_bytes` (garbage at end)

**Methods**
- `explicit PacketParser(ParserOptions = {})`
- `ParseResult parse(span<const byte> buffer) const` — loops using a `ByteReader`, creating `PacketView::from_bytes` per packet
- `optional<PacketView> parse_one(span<const byte> buffer, size_t offset) const` — useful for tests and for random access
- Private: `resync(ByteReader&)` — on bad magic, scan forward to the next magic number (optional stretch goal)

**Notes**
- `ParseResult.packets` holds views into `buffer`, so `parse` takes `span` and never copies bytes. This is the point of the week.
- Consider an overload returning a lazy range later (Week 4 tie-in), but keep the vector version first.

---

### 4.6 `FileLoader`

The only component that *owns* bytes.

**Members**
- `std::vector<std::byte> buffer_`
- `std::filesystem::path path_`

**Methods**
- `static optional<FileLoader> load(const filesystem::path&)` — reads the whole file
- `span<const byte> bytes() const` — what everything else consumes
- `const path& path() const`
- `size_t size() const`

**Notes**
- Make it move-only (copying a large buffer by accident defeats the exercise). If it is moved, spans previously obtained are invalidated — note this.
- A stretch option: `std::span<const std::byte>` from a memory-mapped file instead of a vector (platform-specific, skip unless time permits).

---

### 4.7 `PacketSummary` and `SummaryReport`

Plain value types that **own** their data (no spans), so they can safely outlive the buffer.

**`struct PacketSummary`**
- `size_t index`, `size_t offset`
- `uint32_t sequence`
- `PacketType type`
- `size_t payload_size`
- `bool checksum_ok`
- `std::string payload_preview` (hex of first N bytes)

| Method | Purpose |
|---|---|
| `static PacketSummary from(const PacketView&, size_t index)` | Conversion (view → owning summary) |
| `auto operator<=>(...) const = default` | Defaulted comparison, a second use of `<=>` |

**`class SummaryReport`**
- Members: `vector<PacketSummary> rows_`, aggregate counters (count per type, total payload bytes, bad checksums, sequence gaps)
- Methods:
  - `static SummaryReport build(const ParseResult&)`
  - `void sort_by_sequence()` (uses `<=>`/`std::ranges::sort`)
  - `string to_text() const`
  - `string to_csv() const`
  - `vector<uint32_t> sequence_gaps() const`
  - Accessors for counts

---

### 4.8 CLI (`main.cpp`)

**Responsibilities**
- Parse arguments: input file, `--verify-checksums`, `--max N`, `--format text|csv`, `--sort seq`, `--compare i j`
- Orchestrate: `FileLoader` → `PacketParser` → `SummaryReport` → print

**Suggested small helpers (local to `main.cpp` or a `cli_args` header)**
- `struct CliOptions`
- `optional<CliOptions> parse_args(int, char**)`
- `void print_usage()`
- `int run(const CliOptions&)`

**`--compare i j` command:** pulls two `PacketView`s from the result and prints the `<=>` outcome and *which field decided it*. A good way to exercise the comparison operator interactively.

---

## 5. How the Pieces Interact

```
            ┌──────────────┐
  file ───▶ │  FileLoader  │  owns vector<byte>
            └──────┬───────┘
                   │ bytes()  → span<const byte>
                   ▼
            ┌──────────────┐        uses
            │ PacketParser │ ───────────────▶ ByteReader (cursor over span)
            └──────┬───────┘
                   │ creates
                   ▼
            ┌──────────────┐  decodes   ┌────────────────┐
            │  PacketView  │ ─────────▶ │  PacketHeader  │
            │ (span + hdr) │            └────────────────┘
            └──────┬───────┘
                   │ ParseResult{ vector<PacketView>, diagnostics }
                   ▼
            ┌──────────────┐
            │SummaryReport │  owns PacketSummary rows (no spans)
            └──────┬───────┘
                   ▼
            text / CSV output
```

**Ownership rules**
1. `FileLoader` owns raw bytes.
2. `PacketView`, `ByteReader`, and `ParseResult::packets` borrow from it.
3. `PacketSummary` / `SummaryReport` own copies, so they are safe after the loader dies.

---

## 6. `constexpr` / `consteval` Review Checklist

Go through each and decide yes/no, noting the reasoning in the weekly note:

- [ ] Layout constants and offsets → `constexpr`
- [ ] Layout-consistency check → `consteval` (compile-time fail)
- [ ] `read_u16/u32` helpers → `constexpr`
- [ ] `compute_checksum` → `constexpr`, with a `static_assert` on a literal sample
- [ ] `PacketHeader::is_valid` → `constexpr`
- [ ] `to_string(PacketType)` → `constexpr`
- [ ] Anything that touches the filesystem or allocates a `std::string` → stays runtime (and note why)

---

## 7. Test Plan

**`test_byte_reader`**
- Reads succeed and advance position
- Over-read returns `nullopt` and does not advance
- `read_bytes` returns a span that aliases the original data (compare `.data()` pointers)
- `skip` past the end fails cleanly
- Empty span behavior

**`test_packet_header`**
- Decode a known byte sequence → expected fields
- Too-short input → `nullopt`
- Bad magic → `is_valid() == false`
- `<=>` ordering follows the member order you chose
- `static_assert` tests for constexpr functions (these are compile-time tests)

**`test_packet_view`**
- `payload()` size and content correct
- `raw()` is exactly header+payload (trimmed even when the region is longer)
- Truncated payload → `nullopt`
- Checksum good/bad
- Equality ignores `stream_offset`
- `<=>` consistent with `==` (a == b ⇔ (a <=> b) == 0)
- Tie-break order: sequence → type → payload

**`test_parser`**
- Empty buffer → zero packets, no diagnostics
- N back-to-back valid packets
- Truncated final packet → diagnostic with correct offset
- Garbage trailing bytes → `trailing_bytes` set
- Bad magic mid-stream → stop vs. continue per option
- `max_packets` honored
- Views point into the input buffer (no copy)

**`test_comparisons`**
- Sort a vector of `PacketView`s with `std::ranges::sort` and verify order
- Sort summaries with defaulted `<=>`
- Use `std::is_lt`/`is_eq` helpers on results

**Sample data**
- `make_sample_data` writes: valid file, truncated file, corrupted checksum, bad magic, out-of-order sequences, zero-length payloads, max-size payload.

---

## 8. Suggested Build Order (≈ 5–7 hours)

1. **Hour 1:** Create the solution and four projects, set the shared C++20 / `/W4` settings and project references, confirm an empty build works. Then write down the packet format, create `packet_types` and `byte_utils`, and add constexpr checks.
2. **Hour 2:** `ByteReader` + its tests. Get fully comfortable with `subspan`.
3. **Hour 3:** `PacketHeader::decode`, `PacketView::from_bytes`, tests.
4. **Hour 4:** `PacketParser` + `ParseResult` + diagnostics; sample data generator.
5. **Hour 5:** `FileLoader`, `PacketSummary`, `SummaryReport`, CLI.
6. **Hour 6:** Comparison operators (both styles), `--compare` command, comparison tests.
7. **Hour 7 (buffer):** `constexpr`/`consteval` review, cleanup commit, weekly note.

---

## 9. Pitfalls to Watch For

- **Dangling spans:** moving or destroying `FileLoader` while views are alive. Consider a test that documents the rule.
- **Default `<=>` on mixed members:** floating-point members produce `partial_ordering`; avoid floats in these structs.
- **Defaulted `<=>` and member order:** it compares in declaration order, so reordering members silently changes behavior.
- **Alignment and aliasing:** do **not** `reinterpret_cast` bytes to a struct. Decode field by field (or `memcpy`/`std::bit_cast`) to stay portable and UB-free.
- **Endianness:** centralize in `byte_utils` so there is exactly one place to fix.
- **`span<const byte>` vs `span<byte>`:** use `const` everywhere for read-only inspection.
- **Integer overflow in length math:** `offset + payload_length` can overflow on corrupt data; check by subtraction (`remaining() >= n`) instead of addition.
- **Binary file mode on Windows:** open files with `std::ios::binary`; text mode translates `\r\n` and truncates at Ctrl-Z, which corrupts packet data.
- **Endianness:** MSVC targets (x64/ARM64) are little-endian, so if your packet format is big-endian you need the byte swap; if little-endian, the swap is a no-op, but keep it in the single helper anyway.
- **Mismatched settings across projects:** if the library and tests use different language standards, runtime library (`/MD` vs `/MT`) or iterator debug levels, you get link errors. The shared props file prevents this.
- **MSVC warnings worth reading carefully:** C4244 / C4267 (narrowing, e.g. `size_t` to `uint16_t`) will show up in length and offset math; fix them with explicit, checked conversions rather than silencing them.
- **Static extent vs dynamic extent:** all spans here are dynamic extent; note when a fixed-extent span (e.g. a header-sized one) would add compile-time safety.

---

## 10. Stretch Goals

- Fixed-extent `std::span<const std::byte, kHeaderSize>` for the header decode path.
- `std::as_bytes` / `std::as_writable_bytes` helpers to view typed arrays as bytes (used by the sample data generator).
- Lazy parsing: a range/iterator that yields `PacketView`s on demand (ties back to Week 4 views).
- Resync after corruption by scanning for the next magic number.
- Memory-mapped file input.
- Fuzz-style test: random mutations of a valid file, assert the parser never crashes or reads out of bounds.

---

## 11. Weekly Deliverables (per the README)

- **Runnable program:** `pktinspect` CLI plus sample data generator, built from the VS solution.
- **Markdown note:** where `span` improved the API versus pointer+length; the `<=>` design decisions (defaulted vs explicit, what is excluded from equality); what ended up `constexpr`; lifetime pitfalls you hit.
- **Cleanup commit:** naming pass, test tidy-up, remove duplicated bounds checks.

---

## 12. Definition of Done

- [ ] No function in the public API takes `pointer + length`.
- [ ] No `reinterpret_cast` on packet bytes.
- [ ] All reads go through `ByteReader` or are documented as pre-checked.
- [ ] Both a defaulted and an explicit `<=>` exist, each with tests.
- [ ] `==` and `<=>` are consistent for `PacketView`.
- [ ] At least one `static_assert` and one `consteval` check in the code.
- [ ] Parser never throws and never reads out of bounds on malformed input.
- [ ] Solution builds cleanly in Debug and Release (x64) at `/W4` with no warnings.
- [ ] All projects share one C++20 setting via the common props file.
- [ ] README note and weekly note written.
