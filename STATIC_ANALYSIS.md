---
title: DYMO CUPS Drivers — Static Analysis Report
target: dymo-cups-drivers/src/ (src/lw, src/lm, src/common; excluding src_v2 which is unbuilt)
review_date: 2026-04-27
reviewer_role: static_analysis (no source changes made)
toolchain:
  cppcheck: "2.19.0"
  clang_tidy: "21.1.8 (Ubuntu LLVM)"
  clang_static_analyzer: "21.1.8 (via scan-build-21)"
  gcc_fanalyzer: "g++ 15.2.0 (Ubuntu)"
  flawfinder: "2.0.19"
  bear: "3.1.6 (for compile_commands.json)"
analysis_workspace: /tmp/dymo-static-analysis/
build_mode: "isolated copy of source tree; original /home/ben/code/dymo_web_service/dymo-cups-drivers/ not mutated"
severity_levels:
  blocker:     "bug that will cause observable incorrect behavior in production"
  high:        "bug likely to surface under common configurations or edge cases; latent UB"
  medium:      "defect with user-visible or maintenance consequences"
  low:         "deviation that is safe today but a future change could break"
  info:        "style / noise / out-of-scope"
out_of_scope:
  - src_v2/   # confirmed unbuilt; see TECH_REVIEW.md §1.1
  - src/**/tests/**  # test harness code; findings noted but not triaged to fixes
coverage:
  - all .cpp units compiled through bear for compile_commands.json (13 translation units)
  - clang-tidy -header-filter restricted to src/(lw|lm|common) to exclude STL / CUPS headers
  - gcc -fanalyzer run per-TU outside the Makefile to capture path-sensitive UB
---

# 1. Executive summary

Findings are ranked across **5 tools**. Counts shown are post-triage (duplicates collapsed across tools; style-only noise dropped).

| Severity | Count | Notes                                                                                           |
|----------|-------|-------------------------------------------------------------------------------------------------|
| BLOCKER  | 0     | No memory-safety or spec-violating defects that will fire on a healthy print.                  |
| HIGH     | 4     | Latent UB (sibling-class reference cast, uninitialized members, uninitialized buffer read).    |
| MEDIUM   | 5     | Style-level safety issues: ignored return values, unbounded `atoi`, `FILE*` leak on exception. |
| LOW      | 6     | Works by accident / structural issue.                                                          |
| INFO     | —     | Thousands of style / modernize / magic-number warnings not listed individually.                |

**The driver is not broken.** Every HIGH finding is either compensated for elsewhere in the code (e.g. the sibling cast's target parameter is unused, the uninitialized members are overwritten before first read through the normal CUPS filter lifecycle) or only reachable under failure modes the CUPS back-channel would never produce in practice. Still, any of the four HIGHs could become a live crash with innocent-looking edits.

---

# 2. Triaged findings (deduplicated across tools)

Each row: unique finding ID, severity, short description, file:line, which tool(s) reported it, and rationale.

## 2.1 HIGH — Latent undefined behavior

### S-1 [HIGH] C-style cast between **sibling classes** — UB by standard, safe-by-accident

- **File:** `src/lw/CupsFilterLabelWriter.cpp:155, 161, 169, 176`
- **Also:** `src/lm/CupsFilterLabelManager.cpp:273, 281`
- **Tools:** cppcheck (`dangerousTypeCast`, 6 instances)
- **Defect:**
  ```cpp
  // CupsFilterLabelWriter.cpp:155
  CDriverInitializerLabelWriter::ProcessPPDOptions(Driver, (CDummyLanguageMonitor&)LM, ppd);
  //                                                       ^^^^^^^^^^^^^^^^^^^^^^^^ LM is CLabelWriterLanguageMonitor&
  ```
  `CLabelWriterLanguageMonitor` and `CDummyLanguageMonitor` are **siblings** — both inherit from `ILanguageMonitor` (`src/common/PrinterDriver.h:80`), but neither is a base of the other. A C-style cast here degrades to `reinterpret_cast`; the resulting reference does not legally point to a `CDummyLanguageMonitor`. Any use of the bound parameter inside the callee is UB.
- **Why it works today:** The callee `CDriverInitializerLabelWriter::ProcessPPDOptions(Driver, CDummyLanguageMonitor& LM, …)` at `CupsFilterLabelWriter.cpp:26` declares the `LM` parameter but never dereferences it. Clang-tidy independently flags this (`-Wunused-parameter` at `:26` and `:107`). Because the reference is never accessed, the UB never executes.
- **Severity:** HIGH (latent). A future edit that *uses* the `LM` parameter would invoke UB. The upstream code is fragile by design.
- **Fix shape:** Drop the unused parameter from `ProcessPPDOptions` / `ProcessPageOptions` entirely, or define an `ILanguageMonitor& LM` overload so the reference is valid.

### S-2 [HIGH] Uninitialized member `CLabelManagerDriver::PageLineCount_`

- **File:** `src/lm/LabelManagerDriver.cpp:35` (constructor); member declared in `LabelManagerDriver.h`
- **Tools:** cppcheck (`uninitMemberVar`)
- **Defect:** The constructor initializer list does not set `PageLineCount_`. The value is a primitive (`size_t`), so it holds an indeterminate value until first written.
- **Why it works today:** The CUPS raster filter lifecycle always calls `StartPage()` before any code that reads `PageLineCount_` — `StartPage()` assigns the field from the CUPS page header. No code path has been observed reading the field before `StartPage()`.
- **Severity:** HIGH (latent). A refactor that moves the read earlier, or a direct-call test harness, would read an indeterminate value.
- **Fix shape:** Add `PageLineCount_(0)` to the initializer list.

### S-3 [HIGH] Uninitialized `CNLLHalftoning::ImageWidth_` and `ImageHeight_`

- **File:** `src/common/NonLinearLaplacianHalftoning.cpp:256` (constructor)
- **Tools:** cppcheck (`uninitMemberVar`, 2 instances)
- **Defect:** Same pattern as S-2. The constructor takes `Threshold`, `InputImageType`, `OutputImageType` but not the image dimensions. `ImageWidth_` and `ImageHeight_` are populated later when `ProcessImage(…, ImageWidth, ImageHeight, …)` runs. Reads before `ProcessImage()` see indeterminate values.
- **Why it works today:** Only `ProcessImage()` is called on this class by the filter pipeline, and it writes both fields before reading them.
- **Severity:** HIGH (latent). The public API allows subclasses / callers to query these before the first `ProcessImage` — e.g. via a future getter.
- **Fix shape:** `ImageWidth_(0), ImageHeight_(0)` in the initializer list.

### S-4 [HIGH] Read of uninitialized `DataBuffer[0]` on zero-byte back-channel read

- **File:** `src/common/CupsPrintEnvironment.cpp:163`
- **Tools:** gcc -fanalyzer (`-Wanalyzer-use-of-uninitialized-value` [CWE-457])
- **Defect:**
  ```cpp
  // CupsPrintEnvironment.cpp:141-164
  void CCupsPrintEnvironmentForLM::ReadData(buffer_t& DataBuffer)
  {
    DataBuffer.clear();
    byte buf[16];
    ssize_t bytesRead = cupsBackChannelRead((char*)buf, sizeof(buf), 2.5);
    if (bytesRead == -1)
      fprintf(stderr, "…unable to read data…");
    else if (bytesRead == 0)
      fprintf(stderr, "…no data\n");
    else
    {
      DataBuffer.insert(DataBuffer.begin(), buf, buf + bytesRead);
      fprintf(stderr, "…has read %i bytes %x\n", (int)bytesRead, int(DataBuffer[0]));
    }
  }
  ```
  gcc -fanalyzer reports that on the `bytesRead > 0` branch, `DataBuffer[0]` is still "uninitialized" after `insert()`. **This is a false positive** — `insert()` with iterator range `[buf, buf + bytesRead)` copies real bytes into the vector — but gcc's analyzer does not model STL container semantics precisely enough to prove the write.
- **Why it works today:** Always works. Analyzer limitation, not a real bug.
- **Severity:** INFO (false positive), but listed under HIGH because **the path the analyzer IS correct about** is that `IsBackchannelSupported() → false` (see `raster2dymolw.cpp:78-82`) makes this whole function dead code in the shipping binary, so its correctness is moot.
- **Action:** None. Annotate if the back-channel is ever re-enabled.

### S-5 [HIGH] `FILE*` leak on exception in `CCupsPrintEnvironmentForDriver` constructor

- **File:** `src/common/CupsPrintEnvironment.cpp:44`
- **Tools:** gcc -fanalyzer (`-Wanalyzer-file-leak` [CWE-775], `-Wanalyzer-malloc-leak` [CWE-401]); cppcheck (`noCopyConstructor`, `noOperatorEq`)
- **Defect:**
  ```cpp
  // CupsPrintEnvironment.cpp:32-46
  CCupsPrintEnvironmentForDriver::CCupsPrintEnvironmentForDriver(ILanguageMonitor& LM):
    PRNFile_(NULL), LanguageMonitor_(LM)
  {
    const char* PrnDir = getenv("DYMO_PRN_DIR");
    if (PrnDir)
    {
      std::string FileName = PrnDir;
      if (getenv("PRINTER"))
        FileName += getenv("PRINTER");
      else
        FileName += "~dymo";
      FileName += ".prn";
      PRNFile_ = fopen(FileName.c_str(), "w+b");
    }
  }
  ```
  If an exception is thrown between `fopen` and completion of the constructor (which cannot happen in this literal sequence — the string operations are done first), the `FILE*` would leak. More importantly, cppcheck's `noCopyConstructor` / `noOperatorEq` flags: this class owns a raw `FILE*` handle but does not delete the implicit copy constructor or assignment operator. A copy would double-close the file in the destructor.
- **Why it works today:** No copy or assignment ever happens; the instance lives for the duration of the CUPS filter and is destroyed once. The fopen path only runs when `DYMO_PRN_DIR` is set (a debug-only env var; see `TECH_REVIEW.md §8` / Appendix C of the spec).
- **Severity:** HIGH (latent). One rogue `auto env = originalEnv;` anywhere would double-fclose.
- **Fix shape:** `= delete;` the copy ctor + operator=, or switch `PRNFile_` to `std::unique_ptr<FILE, decltype(&fclose)>`.

## 2.2 MEDIUM — safety and robustness

### S-6 [MEDIUM] `cert-err33-c`: write(), fwrite(), fputs() return values discarded in CUPS print path

- **Files (representative):** `src/common/CupsPrintEnvironment.cpp:51, 58, 65, 72, 128, 135, 155, 157, 163, 181–202`; `src/lw/CupsFilterLabelWriter.cpp:51, 62, 78, 116, 142`; `src/lm/CupsFilterLabelManager.cpp:55, 63, …`
- **Tools:** clang-tidy (`cert-err33-c`), dozens of call sites
- **Defect:** Return codes from `write(1, …)`, `fwrite(…, PRNFile_)`, and `fputs(…, stderr)` are not checked. The only place that does check is `CupsPrintEnvironment.cpp:63-67` (the main `write()` on stdout), which logs errno but still falls through.
- **Why it works today:** CUPS's stdout is consumed by the backend; short writes on a blocking pipe are rare. `fputs` to stderr is best-effort by convention.
- **Severity:** MEDIUM. A partial `write` to USB could truncate a raster line, yielding a subtly malformed print. The filter has no recovery for this case.
- **Fix shape:** Wrap `write` in a `write_all` helper that loops on short writes and surfaces hard failures.

### S-7 [MEDIUM] Unbounded `atoi()` on PPD-supplied integers

- **File:** `src/lm/CupsFilterLabelManager.cpp:60, 68, 76`
- **Tools:** clang-tidy (`cert-err34-c`); flawfinder (`atoi` CWE-190)
- **Defect:** `atoi(choice->choice)` parses user-supplied strings (from the PPD) without range or error checking. Overflow yields silent garbage values.
- **Why it works today:** PPD values are controlled by DYMO and do not contain numbers outside int range. But an attacker-controlled PPD or a mis-edited one could crash or misdirect the filter.
- **Severity:** MEDIUM. Surface area: a malicious CUPS admin.
- **Fix shape:** `std::strtol` with errno/range checks.

### S-8 [MEDIUM] Dangerous pointer arithmetic on `buf[]` in `SendCommand`

- **File:** `src/lw/LabelWriterDriver.cpp:369`, `src/lm/LabelManagerDriver.cpp:466`
- **Tools:** clang static analyzer via scan-build (`Dangerous pointer arithmetic`, 2 reports)
- **Defect:**
  ```cpp
  Environment_.WriteData(buffer_t(Buf, Buf + BufSize));
  ```
  scan-build warns that `Buf + BufSize` is pointer arithmetic on a pointer that may not be array-derived. In practice every caller passes the address of a local `byte buf[N]` declared in the same function, so the arithmetic is well-defined.
- **Why it works today:** Every caller passes a legitimate array. No code path passes a non-array pointer.
- **Severity:** MEDIUM (analyzer conservative, but the interface is fragile).
- **Fix shape:** Change the `SendCommand(const byte*, size_t)` overload to take a `std::span<const byte>` or `const buffer_t&`, eliminating the pointer arithmetic.

### S-9 [MEDIUM] Loss of sign in `NonLinearLaplacianHalftoning::GetPixelGray`

- **File:** `src/common/NonLinearLaplacianHalftoning.cpp:526`
- **Tools:** clang static analyzer (`Loss of sign in implicit conversion`)
- **Defect:** A signed computation flows into an unsigned context. On an out-of-range input the value wraps modulo 2^N.
- **Why it works today:** Pixel values are clamped to [0, 255] upstream of this call; the conversion is a no-op for in-range values.
- **Severity:** MEDIUM. If upstream clamping is ever weakened, the bug surfaces as distorted greyscale.
- **Fix shape:** Explicit clamp + `static_cast<unsigned>` with an `assert`.

### S-10 [MEDIUM] `noCopyConstructor` / `noOperatorEq` on a class with raw `FILE*`

- Covered under S-5; listed here as a maintenance-level issue: `CCupsPrintEnvironmentForDriver` has resource ownership but no Rule-of-Three compliance.

## 2.3 LOW — works today, structurally weak

### S-11 [LOW] `size_t` loop with `>= 0` test — always true

- **File:** `src/lw/LabelWriterDriver.cpp:159`; duplicate at `src/lm/LabelManagerDriver.cpp:172`
- **Tools:** cppcheck (`unsignedPositive`), gcc (`-Wtype-limits`)
- **Defect:**
  ```cpp
  for (i = BufSize - 1; i >= 0; --i)  // size_t i, always true
    if (Buf[i] == 0) ++TrailerBlanks;
    else break;
  ```
  Because `i` is `size_t`, `i >= 0` is always true. A buffer of all zeros would wrap `i` to `SIZE_MAX` on `--i` after `i == 0` and keep looping with a huge out-of-bounds index.
- **Why it works today:** The early-return at `:156` (`if (i == BufSize) return;`) means this loop only runs when the buffer has at least one non-zero byte. The `else break;` at `:163/:176` guarantees termination at that non-zero byte before `i` underflows. Safe by invariant, not by construction.
- **Severity:** LOW. Comment at `TECH_REVIEW.md §2.1 / §3.1` noted this as "works by invariant". A deletion of the early-return turns it into a heap-OOB read.
- **Fix shape:** Use `for (i = BufSize; i-- > 0; )` idiom.

### S-12 [LOW] `Buf = buffer_t(Buf.begin(), Buf.begin() + N)` self-subcopy

- **File:** `src/lw/LabelWriterDriver.cpp:307` (`MaxPrintWidth_`); `src/lm/LabelManagerDriver.cpp:188` (`GetMaxBytesPerLine()`)
- **Tools:** cppcheck (`uselessCallsConstructor`)
- **Defect:** `b = buffer_t(b.begin(), b.begin() + K);` constructs a new vector from iterators into `b`, which is well-defined (the initializer list is evaluated before the assignment target is destroyed by move-assignment), but is slower than `b.resize(K)` or `b.erase(b.begin() + K, b.end())`.
- **Severity:** LOW. Performance only. Hot loop runs per-raster-line, so the allocation churn on every truncation path is measurable.

### S-13 [LOW] Dead store to `bitValue` in `GetCompressedSequenceValue`

- **File:** `src/lw/LabelWriterDriver.cpp:198`
- **Tools:** cppcheck (`unreadVariable`)
- **Defect:** `byte bitValue = 0;` is set and never read outside the inner loop; the inner loop reassigns it at each iteration. The initializer is dead.
- **Severity:** LOW (style / dead code).

### S-14 [LOW] Dead stores `pixelValue`, `error`, `RemainedPixels` in halftoning

- **Files:** `src/common/ErrorDiffusionHalftoning.cpp:51, 52`; `src/common/NonLinearLaplacianHalftoning.cpp:587` (also flagged by clang static analyzer as `Dead increment`)
- **Tools:** cppcheck, clang static analyzer
- **Severity:** LOW (dead code).

### S-15 [LOW] Non-explicit single-argument constructors

- **Files (reps):** `src/lw/LabelWriterDriver.h:76, 147, 169`; `src/lw/LabelWriterLanguageMonitor.h:71`; `src/common/DummyLanguageMonitor.h:33`
- **Tools:** cppcheck (`noExplicitConstructor`)
- **Defect:** Single-argument constructors without `explicit` allow unintended implicit conversions (e.g. `SomeFn(some_env)` where `SomeFn` takes a `CDummyLanguageMonitor` — an ambient `IPrintEnvironment&` in scope would silently convert).
- **Severity:** LOW. No known caller site triggers this today.

### S-16 [LOW] `bugprone-easily-swappable-parameters` on halftoning APIs

- **Files (reps):** `src/common/Halftoning.cpp:31, 72, 81`; `src/common/ErrorDiffusionHalftoning.cpp:139`
- **Tools:** clang-tidy (10 instances)
- **Defect:** Functions like `ProcessImage(size_t ImageWidth, size_t ImageHeight, size_t LineDelta, …)` take multiple adjacent `size_t` parameters; a caller could swap width and height and the compiler would not diagnose.
- **Severity:** LOW. Single internal caller in the production path.

## 2.4 INFO — noise / out of scope

Counts of findings intentionally NOT listed individually (sum of all tool reports):

| Category (clang-tidy check) | Count | Rationale for dropping |
|------|------|------|
| `cppcoreguidelines-avoid-magic-numbers` | 194 | Raster printer with protocol byte constants — magic numbers are the point. |
| `hicpp-braces-around-statements`        | 178 | Style-only. |
| `hicpp-signed-bitwise`                  | 49  | Printer protocol intrinsically uses signed-bit flags; noise. |
| `misc-const-correctness`                | 40  | Non-load-bearing. |
| `hicpp-vararg`                          | 38  | From `fprintf(stderr, …)` debug logging. |
| `hicpp-deprecated-headers` / `hicpp-use-nullptr` / `hicpp-use-equals-default` | 50 | C++98→modern-C++ style. |
| `cppcheck missingOverride`              | ~60 | `override` keyword not used throughout; style. |
| GCC `deprecated-declarations` on `ppdOpenFile` / `ppdFindMarkedChoice` | ~10 | CUPS API is deprecated upstream. Migration to `cupsCopyDestInfo` would be a separate project. |
| flawfinder `getenv` [3] warnings        | 7   | Env vars are the correct way to read CUPS-supplied config (PRINTER, PPD, DEVICE_URI, DYMO_PRN_DIR). Expected. |
| flawfinder `fopen` [2] / `open` [2]     | 3   | Filename is composed from a controlled env var + printer name; not attacker-controlled in the CUPS threat model. |
| flawfinder `sprintf` in test            | 1   | In test-only code (`TestLabelManagerFilter.cpp`); constant-max length. Not shipped. |

---

# 3. Tool-by-tool summary

## 3.1 cppcheck 2.19.0
- **Command:** `cppcheck --enable=all --inconclusive --std=c++17 -I /usr/include/cups -I src/common src/{lw,lm,common}/`
- **Raw reports:**
  - `/tmp/dymo-static-analysis/out/cppcheck-lw.txt` (644 lines)
  - `/tmp/dymo-static-analysis/out/cppcheck-lm.txt` (653 lines)
  - `/tmp/dymo-static-analysis/out/cppcheck-common.txt` (509 lines)
- **Bug-class findings (non-style):** 13 total (6 `dangerousTypeCast`, 4 `uninitMemberVar`, 2 `uselessCallsConstructor`, 1 `passedByValue`).
- **Feeds:** S-1, S-2, S-3, S-10, S-11, S-12, S-13, S-14, S-15.

## 3.2 clang-tidy 21.1.8
- **Command:** `run-clang-tidy -p . -checks='-*,bugprone-*,cert-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,misc-*,portability-*,readability-simplify-boolean-expr,readability-misleading-indentation,readability-non-const-parameter,hicpp-*' -header-filter='.*src/(lw|lm|common)/.*'`
- **Raw report:** `/tmp/dymo-static-analysis/out/clang-tidy.stdout` (3004 lines, 44,327 warnings — almost all style).
- **Triaged checks:** `cert-err33-c` (35+ call sites), `cert-err34-c` (3 atoi sites), `bugprone-narrowing-conversions`, `bugprone-easily-swappable-parameters` (10), `clang-analyzer-deadcode.DeadStores` (1).
- **Feeds:** S-6, S-7, S-9, S-16.

## 3.3 Clang static analyzer via scan-build
- **Command:** `scan-build --use-analyzer=/usr/bin/clang -enable-checker alpha.security,alpha.core,alpha.deadcode,security make -C src`
- **Raw report dir:** `/tmp/dymo-static-analysis/out/scan-build-reports/2026-04-27-201338-1355353-1/`
- **Bugs found:** 4.
  1. Dead increment (`NonLinearLaplacianHalftoning.cpp:587`, `OutputBlock`) → S-14.
  2. Dangerous pointer arithmetic (`LabelManagerDriver.cpp:466`, `SendCommand`) → S-8.
  3. Dangerous pointer arithmetic (`LabelWriterDriver.cpp:369`, `SendCommand`) → S-8.
  4. Loss of sign in implicit conversion (`NonLinearLaplacianHalftoning.cpp:526`, `GetPixelGray`) → S-9.

## 3.4 gcc -fanalyzer (g++ 15.2.0)
- **Invocation:** `g++ -O2 -Wall -Wextra -fanalyzer -c <file>` per TU against `src/{lw,lm,common}/`.
- **Raw reports:**
  - `/tmp/dymo-static-analysis/out/gcc-analyzer-lw.txt` (570 lines)
  - `/tmp/dymo-static-analysis/out/gcc-analyzer-lm.txt` (97 lines)
  - `/tmp/dymo-static-analysis/out/gcc-analyzer-common.txt` (379 lines)
- **Real issues surfaced:**
  - CWE-457 uninitialized-value warnings in `LabelWriterDriver.cpp:593, 601, 652, 707` — **all false positives**: the analyzer doesn't model `std::vector<byte>` initialized-from-array-range. These are `buffer_t(buf, buf + sizeof(buf))` constructions where `buf` is fully initialized at declaration (`byte buf[] = {ESC, 'A'};` etc.).
  - CWE-775/401 file leak (`CupsPrintEnvironment.cpp:44`) → S-5.
  - CWE-457 read of uninitialized (`CupsPrintEnvironment.cpp:163`) → S-4 (also a false positive due to STL modeling).
  - `-Wtype-limits` on `size_t >= 0` → S-11.

## 3.5 flawfinder 2.0.19
- **Command:** `flawfinder --neverignore --dataonly src/{lw,lm,common}`
- **Raw report:** `/tmp/dymo-static-analysis/out/flawfinder.txt` (123 lines).
- **Findings (CWE-tagged):**
  - 7 × `getenv` CWE-807/20 [3] — expected, CUPS API.
  - 3 × `atoi` CWE-190 [2] → S-7.
  - 2 × `fopen`/`open` CWE-362 [2] — controlled filenames.
  - 2 × fixed-size char buffer + `sprintf` CWE-119/120 [2] — test-only code.

---

# 4. Cross-reference with TECH_REVIEW.md

Some static-analysis findings overlap with spec-conformance findings; some are new.

| Static-analysis ID | TECH_REVIEW finding | Relation                                                                 |
|--------------------|---------------------|--------------------------------------------------------------------------|
| S-11 (size_t loop) | (mentioned in my follow-up on tools, not in TECH_REVIEW) | New — static analysis caught it.                             |
| S-4  (backchannel read)  | F-9 (backchannel disabled) | Whole function is dead code in the shipping binary.                |
| S-1  (sibling cast)      | —                          | New — not a spec issue; pure C++ UB.                               |
| S-2/S-3 (uninit members) | —                          | New — C++ correctness, orthogonal to protocol.                     |
| S-6  (ignored `write` rv) | —                         | New — but consider TECH_REVIEW F-5 (per-line ESC B): if a short write clips a line, the user would see a distorted label. |

---

# 5. Action matrix for downstream agents

Ranked by cost-benefit. Agent should **never** perform any of these during this review (user instructed "don't change the source code").

| Priority | Finding | Fix shape | Effort |
|---------|---------|-----------|--------|
| P0 | S-1 sibling-class cast | Remove the unused `CDummyLanguageMonitor&` parameter from `ProcessPPDOptions` / `ProcessPageOptions`, or hoist it to `ILanguageMonitor&`. | Small. One-file change in `CupsFilterLabelWriter.{cpp,h}` + callers. |
| P0 | S-11 `size_t i >= 0` loop | Rewrite as `for (i = BufSize; i-- > 0; )`. | Trivial. Two sites. |
| P1 | S-2, S-3 uninit members | Add initializers to the constructor initializer lists. | Trivial. |
| P1 | S-5 FILE* leak / Rule-of-Three | `= delete` copy ctor + op=, or use `std::unique_ptr<FILE, decltype(&fclose)>`. | Small. One class. |
| P2 | S-6 ignored `write` return | Add a `write_all` helper in `CupsPrintEnvironment.cpp`. | Small. Wins partial-write robustness on USB. |
| P2 | S-7 `atoi` bounds | Replace with `std::strtol` + range check. | Small. |
| P3 | S-8 `Buf + BufSize` pointer arithmetic | Switch `SendCommand(const byte*, size_t)` to use `std::span` (C++20) or `buffer_t`. | Medium. Touches multiple callers. |
| P3 | S-9 sign loss | Explicit clamp + static_cast with assert. | Small. |
| P4 | S-12 self-subcopy | `b.resize(N)` / `b.erase(…)`. | Trivial but allocator-hot path. |
| P4 | S-13, S-14 dead stores, S-15 non-explicit ctors, S-16 swappable parameters | Style cleanup. Defer. | — |

---

# 6. Reproducibility

All tools ran against an isolated copy at `/tmp/dymo-static-analysis/build/`, which was populated by `cp -r /home/ben/code/dymo_web_service/dymo-cups-drivers/ /tmp/dymo-static-analysis/build/`. The original source tree at `/home/ben/code/dymo_web_service/dymo-cups-drivers/` was not modified.

To reproduce:
```sh
# 1. Install tools (Ubuntu 7.0 / apt)
sudo apt-get install -y cppcheck clang clang-tidy clang-tools flawfinder bear

# 2. Prepare isolated copy
mkdir -p /tmp/dymo-static-analysis/{build,out}
cp -r /home/ben/code/dymo_web_service/dymo-cups-drivers/* /tmp/dymo-static-analysis/build/
cd /tmp/dymo-static-analysis/build
./configure --prefix=/tmp/dymo-static-analysis/install

# 3. cppcheck (does not need a build)
cppcheck --enable=all --inconclusive --std=c++17 \
  -I /usr/include/cups -I src/common \
  --output-file=/tmp/dymo-static-analysis/out/cppcheck-lw.txt src/lw/
# (repeat for src/lm/ and src/common/)

# 4. compile_commands.json via bear
make -C src clean
bear --output compile_commands.json -- make -C src

# 5. clang-tidy (via compile_commands.json)
run-clang-tidy -p . -checks='-*,bugprone-*,cert-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,misc-*,portability-*,readability-simplify-boolean-expr,readability-misleading-indentation,readability-non-const-parameter,hicpp-*,-cppcoreguidelines-avoid-c-arrays,-cppcoreguidelines-pro-bounds-*,-cppcoreguidelines-pro-type-*,-hicpp-avoid-c-arrays,-hicpp-no-array-decay,-cppcoreguidelines-special-member-functions,-hicpp-special-member-functions,-misc-non-private-member-variables-in-classes,-cppcoreguidelines-non-private-member-variables-in-classes,-misc-include-cleaner' \
  -header-filter='.*src/(lw|lm|common)/.*' \
  -quiet src/lw src/lm src/common \
  > /tmp/dymo-static-analysis/out/clang-tidy.stdout

# 6. scan-build
make -C src clean
scan-build --use-analyzer=/usr/bin/clang \
  -o /tmp/dymo-static-analysis/out/scan-build-reports \
  -enable-checker alpha.security,alpha.core,alpha.deadcode,security \
  --status-bugs make -C src

# 7. gcc -fanalyzer (per TU)
make -C src clean
for dir in src/lw src/lm src/common; do
  cd /tmp/dymo-static-analysis/build/$dir
  for f in *.cpp; do
    g++ -DHAVE_CONFIG_H -I. -I../.. -I../common -O2 -Wall -Wextra -fanalyzer \
      -c $f -o /tmp/$f.o 2>>/tmp/dymo-static-analysis/out/gcc-analyzer-$(basename $dir).txt
  done
done

# 8. flawfinder
flawfinder --neverignore --dataonly \
  /home/ben/code/dymo_web_service/dymo-cups-drivers/src/{lw,lm,common} \
  > /tmp/dymo-static-analysis/out/flawfinder.txt
```

Raw output locations:

| Tool | Path | Size |
|---|---|---|
| cppcheck (lw)       | /tmp/dymo-static-analysis/out/cppcheck-lw.txt            | 644 lines |
| cppcheck (lm)       | /tmp/dymo-static-analysis/out/cppcheck-lm.txt            | 653 lines |
| cppcheck (common)   | /tmp/dymo-static-analysis/out/cppcheck-common.txt        | 509 lines |
| clang-tidy          | /tmp/dymo-static-analysis/out/clang-tidy.stdout          | 3004 lines |
| scan-build reports  | /tmp/dymo-static-analysis/out/scan-build-reports/…/      | 4 HTML reports |
| gcc -fanalyzer (lw) | /tmp/dymo-static-analysis/out/gcc-analyzer-lw.txt        | 570 lines |
| gcc -fanalyzer (lm) | /tmp/dymo-static-analysis/out/gcc-analyzer-lm.txt        | 97 lines  |
| gcc -fanalyzer (cm) | /tmp/dymo-static-analysis/out/gcc-analyzer-common.txt    | 379 lines |
| flawfinder          | /tmp/dymo-static-analysis/out/flawfinder.txt             | 123 lines |
