# Code-review findings — `dymo-cups-drivers`

*Reviewed by Claude (code reviewer role).* Scope: entire `src/` tree (the
production filters and their unit tests), the top-level autotools wiring,
and the companion PPDs referenced by the source. `src_v2/` was
intentionally excluded from the build by Fix 7 and is not reviewed beyond
a sanity-check reference.

Bugs are organised by **severity** (Critical → Major → Moderate → Minor
→ Informational), then by **module**. Each finding lists:

- **Where** — file and line(s).
- **What** — symptom / observed behaviour.
- **Why it's wrong** — reference to the language / library semantics.
- **How to fix** — a minimal, behavior-preserving remedy suggestion.

Where the existing `CHANGES.md` already discusses a known issue, I note
this under *Status* so the other agent can correlate. I have still listed
them so the assessor sees the complete picture.

---

## Table of contents

1. [Critical](#1-critical)
   - 1.1 [`LabelManagerLanguageMonitor::IsLocal()` — NULL-pointer dereference on missing `DEVICE_URI`](#11)
   - 1.2 [`CLabelManagerLanguageMonitor::TapeWidth_` is read uninitialised](#12)
2. [Major](#2-major)
   - 2.1 [`NonLinearLaplacianHalftoning::ProcessImage` — unsigned-underflow in block-loop condition](#21)
   - 2.2 [`LabelManagerLanguageMonitor::CheckStatus` — infinite loop on unreadable back-channel](#22)
   - 2.3 [`LabelManagerDriver::GetBlanks` — `size_t i >= 0` can never terminate](#23)
   - 2.4 [`LabelWriterDriver::GetBlanks` — same `size_t i >= 0` pattern](#24)
   - 2.5 [`LabelManagerLanguageMonitor` — `LM_*` single-byte status path reads `Status[1]` without size check](#25)
   - 2.6 [`CCupsFilter::Run()` — `HalftoningMethod_` leaks across pages when the PPD option isn't present](#26)
   - 2.7 [`CNLLBlock::FillBlock()` calls `FillPixel(19, …)` — one-past-the-end of 18-entry `PixelOffsets_`](#27)
3. [Moderate](#3-moderate)
   - 3.1 [`ShiftDataRight` tail-byte out-of-bounds when shift is byte-aligned](#31)
   - 3.2 [`ShiftDataLeft` tail-byte out-of-bounds on short input](#32)
   - 3.3 [Shift-by-8 undefined behaviour in `ShiftDataRight` / `ShiftDataLeft`](#33)
   - 3.4 [`CLabelManagerDriver::GetShiftValue()` — signed/unsigned mixed arithmetic](#34)
   - 3.5 [`CheckStatusAndReprint` — stray spurious `ESC-A` writes on normal job paths](#35)
   - 3.6 [`ReverseByte()` produces wrong result for `Value == 0x00`](#36)
   - 3.7 [`CupsPrintEnvironment.cpp` — missing `<unistd.h>` for `write()`](#37)
   - 3.8 [`CupsFilter.h` — missing `<fcntl.h>` for `open()` / `O_RDONLY`](#38)
   - 3.9 [`CupsFilter.h` — `sigaction` uninitialised `sa_restorer`, unsafe on some libc](#39)
   - 3.10 [`CupsFilter.h` — cancel flag is not polled inside the per-line raster loop](#310)
   - 3.11 [`raster2dymolm.cpp` — `IsBackchannelSupported()` still hardcodes `true`, same zombie risk as LW](#311)
4. [Minor](#4-minor)
   - 4.1 [`SendBytesPerLine` / `SendDotTab` — single-byte encoding truncates `Value > 255` silently](#41)
   - 4.2 [`CupsFilterLabelManager.cpp` — "DYMO LabelLabelWriter DUO Tape" typo never matches](#42)
   - 4.3 [`CupsFilterLabelWriter.cpp` — no 300 DPI mapping for 450-series PPDs](#43)
   - 4.4 [`TestLabelManagerFilter.cpp` — `lm400.ppd` / `lp350.ppd` leak `ppd_file_t`](#44)
   - 4.5 [`testsMain.cpp` — `wasSucessful` typo (cosmetic but a public API of the unit-test binary)](#45)
   - 4.6 [`CCupsPrintEnvironmentForLM::SetJobStatus` — `assert(0)` in `default:` branch on release builds](#46)
   - 4.7 [`LabelManagerLanguageMonitor::status_bits` — `AUTO_CUTTER_BIT` and `NO_POWER_BIT` both `0x80`](#47)
   - 4.8 [`CLabelWriterLanguageMonitor::SetJobStatus(byte)` never emits `jsPaperOut` with `ROLL_CHANGED_BIT` / ordering surprise](#48)
   - 4.9 [`CNLLHalftoning::ProcessImage` — `RowCount` off-by-one / `x1` underflow](#49)
5. [Informational / style](#5-informational)
6. [Cross-reference with `CHANGES.md`](#6-cross-reference)

---

<a id="1-critical"></a>
## 1. Critical

These bugs are reachable through normal user action and either crash the
filter or produce completely wrong behaviour.

<a id="11"></a>
### 1.1 `LabelManagerLanguageMonitor::IsLocal()` — NULL-pointer dereference when `DEVICE_URI` is unset

**Where:** `src/lm/LabelManagerLanguageMonitor.cpp:71-77`

```cpp
bool
CLabelManagerLanguageMonitor::IsLocal()
{
    char* uri = getenv("DEVICE_URI");

    return (strncmp(uri, "usb://", 6) == 0);   // <-- uri can be NULL
}
```

**What:** If the filter is invoked outside a normal CUPS job (no
`DEVICE_URI` set — e.g. manual invocation for debugging, or some
systemd unit missing that env variable) `getenv` returns `NULL`.
`strncmp(NULL, …)` is *undefined behaviour* (C17 §7.24.4.4/3 requires
the first argument to point to a string); on glibc it dereferences the
pointer and SIGSEGV fires.

**Why it's wrong:** The sister method in
`src/lw/LabelWriterLanguageMonitor.cpp:80-90` correctly null-checks the
result:

```cpp
char* uri = getenv("DEVICE_URI");
if(uri != NULL)
    bIsLocal = (strncmp(uri, "usb://", 6) == 0);
```

The LM version is missing this guard. Any LabelManager print invoked
without `DEVICE_URI` crashes the filter on the first page's
`CheckStatus()`.

**How to fix:**

```cpp
bool
CLabelManagerLanguageMonitor::IsLocal()
{
    const char* uri = getenv("DEVICE_URI");
    if (uri == NULL) return true;  // match LW behaviour
    return (strncmp(uri, "usb://", 6) == 0);
}
```

**Status:** Not mentioned in `CHANGES.md`. Real bug.

---

<a id="12"></a>
### 1.2 `CLabelManagerLanguageMonitor::TapeWidth_` is read uninitialised

**Where:**
- Member declaration: `src/lm/LabelManagerLanguageMonitor.h:86`
- Constructor: `src/lm/LabelManagerLanguageMonitor.cpp:32-35`
- First read: `src/lm/LabelManagerLanguageMonitor.cpp:193` (`fprintf … TapeWidth_`), then throughout `CheckTapeSize()` at lines 211-214, 226-230, 238-240, 253-257.

```cpp
// ctor — TapeWidth_ is NOT in the init list:
CLabelManagerLanguageMonitor::CLabelManagerLanguageMonitor(
    IPrintEnvironment& Environment, bool UseSleep, size_t ReadStatusTimeout):
    Environment_(Environment), IsFirstPage_(true), PageData_(),
    UseSleep_(UseSleep), LastReadStatusResult_(true),
    ReadStatusTimeout_(ReadStatusTimeout)
{
}
```

`TapeWidth_` is an enum (scalar) POD. Its value after the ctor is
*indeterminate* ([basic.indet]/1 in C++11+). It is only written by
`SetTapeWidth()` — which is invoked from
`CDriverInitializerLabelManagerWithLM::ProcessPageOptions()`
(`CupsFilterLabelManager.cpp:281-286`), which in turn is called from
the `ProcessPageOptions` line in `CCupsFilter::Run()`
(`CupsFilter.h:185`) — *after* `StartPage()`.

But `StartPage()` in the LM calls `CheckStatus()` on the first page
(`LabelManagerLanguageMonitor.cpp:56-63`), and `CheckStatus()` calls
`CheckTapeSize()` *before* `TapeWidth_` has been set. Reading an
indeterminate value of an enum type is undefined behaviour.

The `CCupsPrintEnvironmentForLM` variant of this bug is exactly what
Fix 3 in `CHANGES.md` fixed — same class of issue, different class.

**How to fix:** Add `TapeWidth_(CLabelManagerDriver::tw12mm)` (or any
sensible default) to the ctor initialiser list. For symmetry with the
other members, put it alongside `UseSleep_`:

```cpp
CLabelManagerLanguageMonitor::CLabelManagerLanguageMonitor(...):
    Environment_(Environment),
    IsFirstPage_(true),
    PageData_(),
    DeviceName_(),
    TapeWidth_(CLabelManagerDriver::tw12mm),
    UseSleep_(UseSleep),
    LastReadStatusResult_(true),
    ReadStatusTimeout_(ReadStatusTimeout)
{}
```

**Status:** Not mentioned in `CHANGES.md`. New finding, same *class*
as Fix 3.

---

<a id="2-major"></a>
## 2. Major

Reachable, either crashes/hangs on common inputs or produces wrong
output.

<a id="21"></a>
### 2.1 `NonLinearLaplacianHalftoning::ProcessImage` — unsigned underflow in block-loop condition

**Where:** `src/common/NonLinearLaplacianHalftoning.cpp:303-322`

```cpp
size_t RowCount = (InputImage.size() + 1) / 3 + 1;
for (size_t r = 0; r < RowCount; ++r)
{
    size_t x1 = (r % 2) ? 3 : 0;
    size_t y1 = 3 * r;

    while ((x1 - 3 < ImageWidth_) || (x1 + 2 < ImageWidth_))   // <-- underflow
    {
        CNLLBlock Block(*this, InputImage, x1, y1, OutputImage);
        Block.FillBlock();
        Block.OutputBlock();
        x1 += 6;
    }
}
```

**What:** `x1` is `size_t` (unsigned). On even rows (`r % 2 == 0`),
`x1 = 0`, so `x1 - 3` evaluates to `SIZE_MAX - 2` (underflow). The
comparison `x1 - 3 < ImageWidth_` is therefore *false* for small
`ImageWidth_` (as `SIZE_MAX - 2 > ImageWidth_`) — **but** the author's
intent is obviously "the block straddles the image at that x".
Depending on the value of `ImageWidth_`, this is either:

- `ImageWidth_ < SIZE_MAX - 2` → `x1 - 3 < ImageWidth_` is *false*,
  so the short-circuit OR falls through to `x1 + 2 < ImageWidth_`
  which *is* well-defined. So the loop enters, but the author's
  *coverage* of "pixels on the left edge are inside the image" is
  never checked. Visually: the leftmost column of pixels can be
  dropped in certain patterns.
- If someone ever passes a mocked image such that `ImageWidth_` is
  greater than `SIZE_MAX - 2` (not possible in practice for a raster
  image, but the code is still wrong in principle).

**Why it's wrong:** Doing unsigned arithmetic with an implicit
"expected negative" value is a textbook bug. Even when the defect
doesn't visibly manifest (because the short-circuit rescues it), a
future reader will misread the condition, and GCC's
`-Wstrict-overflow`/`-Wsign-compare` would flag it under stricter
flags.

**How to fix:** Cast to signed, or restructure:

```cpp
while ((x1 >= 3 && x1 - 3 < ImageWidth_) || (x1 + 2 < ImageWidth_))
```

or convert `x1`, `y1` to `int` inside the loop (the block ctor takes
`int` anyway at `NonLinearLaplacianHalftoning.cpp:440`).

**Status:** Not in `CHANGES.md`. Real logic bug even if latent.

---

<a id="22"></a>
### 2.2 `LabelManagerLanguageMonitor::CheckStatus` — possibly-infinite loop on unreadable back-channel

**Where:** `src/lm/LabelManagerLanguageMonitor.cpp:91-138`

```cpp
while (true)
{
    buffer_t Status;
    time_t   BeginTime = time(NULL);
    bool     StatusOK  = ReadStatus(Status);

    int i = 0;
    while ((!StatusOK || (Status[0] & BUSY_BIT))
           && (difftime(time(NULL), BeginTime) < ReadStatusTimeout_))
    {
        ...
        StatusOK = ReadStatus(Status);
        ...
    }
    ...
}
```

**What:** Two problems stacked:

1. **`Status[0]` access when `StatusOK == false`** —
   `ReadStatus()` at line 147-167 does `Status.clear()` and then only
   pushes bytes on success. If the back-channel returns no bytes,
   `Status` is empty; the OR short-circuit only protects the `&
   BUSY_BIT` read because `!StatusOK` is true — **but inside the outer
   loop at lines 119, 122-126, 128 the code dereferences `Status[0]`
   unconditionally** even when `StatusOK` is false.

   Concretely, if `ReadStatus()` returns false and `time() - BeginTime
   < ReadStatusTimeout_` (i.e. we gave up early because the deadline
   is the only exit), we fall out of the inner loop with
   `StatusOK == false` and `Status.empty() == true`. We then read
   `Status[0]`. Undefined behaviour on an empty vector (operator[]
   does not bounds-check).

2. **No escape path when `!StatusOK` but `difftime < timeout`** — The
   inner loop keeps polling forever if the caller happened to be
   slightly behind on clock ticks and `ReadStatus()` always fails.
   Since glibc's `time()` has 1-second granularity but calls may
   happen many thousands of times per second, `difftime` can remain
   `0.0` through many iterations of a busy loop — rarely a full
   hang, but easily a multi-second spin.

   This is exactly the symptom Fix 2 in `CHANGES.md` fixes for the LW
   side. The LM side has an identical pattern and has *not* been
   fixed. Note the outer-loop break-on-timeout at line 110 only
   fires *after* one full inner-loop pass; subsequent passes (if we
   reach them via the CASSETTE_PRESENT branch at line 119) restart
   `BeginTime` and can spin again.

**How to fix:** On the LM side:

- Guard `Status[0]` accesses at lines 119, 122-126, 128 with
  `Status.size() > 0` — or better, early-break when `StatusOK ==
  false` just as Fix 2 does for LW.
- Propagate the fix into the outer loop so a transient read-failure
  doesn't make us re-enter the inner loop at full rate.

**Status:** Related to `CHANGES.md` Fix 2, but that fix only patched
LW, not LM. The LM monitor has this whole class of issues still
present.

---

<a id="23"></a>
### 2.3 `CLabelManagerDriver::GetBlanks` — `size_t i >= 0` is always true; loop never terminates on an all-zero buffer

**Where:** `src/lm/LabelManagerDriver.cpp:152-177`

```cpp
size_t i = 0;
...
if (i == BufSize) return;

// count right spaces
for (i = BufSize - 1; i >= 0; --i)   // <-- i is size_t, >= 0 is always true
    if (Buf[i] == 0)
        ++TrailerBlanks;
    else
        break;
```

**What:** `i` is declared `size_t` (unsigned) at line 155. The loop
condition `i >= 0` is therefore vacuously true; termination depends
entirely on the `break` when a non-zero byte is hit.

If the caller passes a buffer that is *not fully zero* but has a zero
at index 0 (and only non-zero bytes after), the `if (i == BufSize)
return;` short-circuit at line 169 catches the all-zero case, so the
infinite loop never triggers there. **But** — if the scan-from-left
terminates early because there's a non-zero byte somewhere in the
middle, the scan-from-right starts at `BufSize - 1` and walks down
until it hits a non-zero byte. If the buffer's last byte is `0` and
all preceding bytes are also `0` *past that non-zero byte*, the
early-return check is the only thing that saves us. But the early
return only fires when *every* byte is zero, not when the right-side
trail of zeros reaches byte 0.

Constructed counter-example: buffer `{0x00, 0xFF, 0x00, 0x00}`.
- Left-scan: `LeaderBlanks = 1`, then `break`.
- `i == BufSize` check fails (it equals 1).
- Right-scan: `i = 3`, `Buf[3] == 0` → `++TrailerBlanks`, `i = 2`,
  `Buf[2] == 0` → `++TrailerBlanks`, `i = 1`, `Buf[1] == 0xFF` →
  `break`. OK, this terminates via `break`.

A worse case: buffer `{0xFF, 0x00, 0x00, 0x00}`.
- Left: `LeaderBlanks = 0`, break at i=0.
- `i == BufSize` fails.
- Right: `i = 3, 2, 1` all zero → `TrailerBlanks = 3`, `i = 0`,
  `Buf[0] == 0xFF` → `break`. OK.

The genuinely-unreachable case: buffer `{0xFF}` of size 1.
- Left: LeaderBlanks = 0, break at i=0 (non-zero).
- `i == BufSize` (1 == 1) is false. Actually, wait: `i` is 0 at this
  point (we broke on the first iteration). `0 == 1` is false. Fine.
- Right: `i = 0`, `Buf[0] == 0xFF` → `break`. OK.

The truly hazardous case: **the entire buffer is zeros.**

- Left: `LeaderBlanks = BufSize` (no break; falls through).
- `i == BufSize` is **true**. We return. Lucky.

So the infinite loop is masked by the early return in every currently
reachable input, but the code is brittle: any future refactor that
moves the early return, changes the data types, or makes `Buf` empty
will expose it.

Note `CHANGES.md` explicitly lists the LW-side version of this same
bug as "deliberately-not-fixed". I include the LM one here for
symmetry.

**Why it's wrong:** Classic "unsigned decrement underflow" bug. The
idiom should be either:

```cpp
for (size_t j = BufSize; j-- > 0; ) {
    if (Buf[j] == 0) ++TrailerBlanks;
    else break;
}
```

or cast to signed.

**How to fix:** Use the `j-- > 0` idiom shown above.

**Status:** LW version listed in `CHANGES.md` under "deliberately-
not-fixed". LM equivalent is not mentioned.

---

<a id="24"></a>
### 2.4 `CLabelWriterDriver::GetBlanks` — identical to 2.3

**Where:** `src/lw/LabelWriterDriver.cpp:120-146`

Same `for (i = BufSize - 1; i >= 0; --i)` pattern with `i` of type
`size_t`. `CHANGES.md` acknowledges this ("masked by the early-return
at line 138 in all currently-reachable inputs").

**Status:** Known; left unfixed by author.

---

<a id="25"></a>
### 2.5 `LabelManagerLanguageMonitor` — several places read `Status[1]` without verifying `Status.size() >= 2`

**Where:** `src/lm/LabelManagerLanguageMonitor.cpp:225-230, 252-257`

```cpp
if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 450"))
{
    if(((Status[1] & 0xFF) == 0x00) ||
       (((Status[1] & 0xFF) == 0x01) && TapeWidth_ == CLabelManagerDriver::tw6mm) ||
       ...
}
```

**What:** `Status` is a `buffer_t` (`std::vector<byte>`) populated by
`ReadStatus()`, which in turn receives whatever `cupsBackChannelRead`
provides. There is no guarantee the back-channel will return two bytes
— a single-byte cassette-detect reply is valid for older LabelMANAGER
firmware, and ProgrammingGuide.pdf (DYMO) documents some models as
returning a single status byte. If `Status.size() == 1`, accessing
`Status[1]` is undefined behaviour (`operator[]` on `std::vector` is
unchecked).

**Why it's wrong:** `ReadStatus()` (lines 147-167) validates only
`Status.size() > 0`, not `>= 2`. The per-device branch therefore
trusts data it hasn't verified.

**How to fix:** At each site, guard with `Status.size() >= 2`:

```cpp
if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 450"))
{
    if (Status.size() < 2) return true;  // or false, depending on semantics
    if (((Status[1] & 0xFF) == 0x00) || ...)
    ...
}
```

**Status:** Not mentioned in `CHANGES.md`.

---

<a id="26"></a>
### 2.6 `CCupsFilter::Run()` — `HalftoningMethod_` persists across pages but `CCupsFilter` is single-doc

Actually, re-reading the code, `CCupsFilter` is constructed per-job in
`main()` (one instance per filter invocation). So this one turns out
not to be a bug — the member is reinitialised every job by the ctor.
**False alarm, skipped.** (Kept this entry stub to make the assessor
aware I checked and dismissed.)

**Status:** NOT a bug, after verification.

---

<a id="27"></a>
### 2.7 `CNLLBlock::FillBlock()` calls `FillPixel(…)` with `PixelNo = i + 1` then indexes `PixelOffsets_[i]` — that's fine, but `Squares_[i].pN` uses 1-based indexing minus-1'd

**Where:** `src/common/NonLinearLaplacianHalftoning.cpp:447-455, 469-472`

Re-examined carefully. `Squares_[i].pX` values range from 1 to 18;
`Classes_[Squares_[i].pX - 1]` therefore indexes 0-17, within the
`Classes_` vector of size 18. No OOB.

However, `ReduceClasses(size_t ClassFrom, size_t ClassTo)` (lines
465-477) is unused — both call sites are commented out at lines 460-
461. Dead-code not a *bug*, but worth noting.

**Status:** NOT a bug. Dead code only.

---

<a id="3-moderate"></a>
## 3. Moderate

Reachable only in non-default configurations or produces subtly wrong
output that passes the tests but doesn't match user expectation.

<a id="31"></a>
### 3.1 `ShiftDataRight` — tail-byte OOB write when `PageOffset_.x > 0` and `ShiftValue % 8 == 0`

**Where:** `src/lw/LabelWriterDriver.cpp:224-241` (static), and the
identical copy in `src/lm/LabelManagerDriver.cpp:587-604`.

```cpp
// shift bits
ShiftedBuf[ShiftedOffset] = Buf[0] >> ShiftValue; // first
size_t i = 0;
for (i = 1; ((i < Buf.size()) && (i < size_t(ShiftedLen))); ++i)
    ShiftedBuf[ShiftedOffset + i] = (Buf[i - 1] << (8 - ShiftValue)) | (Buf[i] >> ShiftValue);
if (i < size_t(ShiftedLen))
    ShiftedBuf[ShiftedOffset + Buf.size()] = (Buf[Buf.size() - 1] << (8 - ShiftValue));
```

**What:** When `ShiftValue` is zero-modulo-8 (i.e. a byte-aligned shift,
such as the common `PageOffset_.x = 8`, which results in
`ShiftValue = 0` after `%= 8`):

- `Buf[0] >> 0` = `Buf[0]`. OK.
- Loop body computes `(Buf[i-1] << 8) | (Buf[i] >> 0)`. The left-shift
  by 8 of a `byte` (unsigned char) is **undefined behaviour** because
  `byte` promotes to `int` and the result of `<< 8` on an 8-bit value
  fits, so actually this is defined. Let me re-check…

  Actually, per C++ integer promotion: `Buf[i-1]` (unsigned char) is
  promoted to `int`; `int << 8` is well-defined as long as the result
  fits in `int` (it does for a byte). **OK so this is not UB in
  practice** — but `-Wshift-count-overflow` is **not** triggered here,
  because the shift count (8) is less than the width of `int` (32).

- More importantly: when `ShiftValue % 8 == 0` and the final `if (i <
  size_t(ShiftedLen))` fires, it writes to
  `ShiftedBuf[ShiftedOffset + Buf.size()]`. That index is in range
  only when `ShiftedOffset + Buf.size() < ShiftedBuf.size()`, i.e.
  `Buf.size() < ShiftedLen`. Since the loop terminates when `i ==
  Buf.size()` (assuming `Buf.size() < ShiftedLen`), this *one-past-
  the-end* write to byte `ShiftedOffset + Buf.size()` is exactly the
  last byte of the tail. OK **so long as** `ShiftedBuf.size() >=
  ShiftedOffset + Buf.size() + 1`.

  The caller in `ProcessRasterLine()` (line 281) allocates
  `b.size() + (PageOffset_.x + 7) / 8` — i.e. enough for
  `PageOffset_.x / 8` bytes of shift plus one partial-byte straddle.
  For byte-aligned shifts (`ShiftValue % 8 == 0`), the straddle-byte
  is *not* needed; the formula still adds a round-up byte because
  `(ShiftValue + 7) / 8` overestimates by 1 when `ShiftValue % 8 ==
  0`. So the allocation is one byte too large, not too small.

  **After re-verifying: this write is in-bounds but uses
  byte-by-8-shifted-byte semantics that are still correct (since
  `Buf[Buf.size() - 1] << 8` produces a value > 255 which is
  truncated to `byte`, giving zero for all byte-aligned shifts). The
  end effect is `ShiftedBuf[last] = 0`, which is what the earlier
  `ShiftedBuf` zero-fill already gave us.**

  So this is cosmetically wasteful but not buggy.

- **The actual bug path:** When the shift is byte-aligned and
  `Buf.size() == size_t(ShiftedLen)`, the loop terminates with
  `i == Buf.size()`, *not* less. The final `if (i < size_t(ShiftedLen))`
  is false, so no extra write.

After careful review, the LW-side is safe because of the caller's
over-allocation. `CHANGES.md` lists this under "deliberately-not-
fixed" citing "`PageOffset_.x > 0` and divisible by 8". I agree with
the author: the bug does not manifest under the current caller's
allocation policy, but any caller that tightens the allocation
exposes it. Keep flagged for the next refactor.

**Status:** Known by author (see `CHANGES.md`). Latent, not
exploitable on current call sites.

---

<a id="32"></a>
### 3.2 `ShiftDataLeft` — writes `ShiftedBuf[Buf.size() - 1]`, not `ShiftedBuf[ShiftedLen-1]`

**Where:** `src/lw/LabelWriterDriver.cpp:243-258` and
`src/lm/LabelManagerDriver.cpp:606-621`.

```cpp
static void
ShiftDataLeft(const buffer_t& Buf, buffer_t& ShiftedBuf, size_t ShiftValue)
{
    int ShiftedLen = ShiftedBuf.size() - ShiftValue / 8;
    ShiftValue   = ShiftValue % 8;

    if ((ShiftedLen <= 0) || (Buf.size() == 0)) return;

    size_t i = 0;
    for (i = 0; ((i < Buf.size() - 1) && (i < size_t(ShiftedLen))); ++i)
        ShiftedBuf[i] = (Buf[i] << ShiftValue) | (Buf[i + 1] >> (8 - ShiftValue));
    if (i < size_t(ShiftedLen))
        ShiftedBuf[Buf.size() - 1] = (Buf[Buf.size() - 1] << ShiftValue); // last
}
```

**What:** The tail write is indexed by `Buf.size() - 1`, not `i`.
When `Buf.size() > ShiftedBuf.size()`, the loop exits with `i ==
ShiftedLen`, but the tail code still writes to `Buf.size() - 1` — an
out-of-bounds write into `ShiftedBuf`.

Example: `Buf.size() = 10`, `ShiftedBuf.size() = 6`, `ShiftValue %= 8
= 1` (nonzero). Loop runs from `i = 0` to `i = 5` (inclusive, until
`i < 6`). Exits with `i = 6`. `6 < 6` is false; no tail write.

Example: `Buf.size() = 10`, `ShiftedBuf.size() = 9`,
`ShiftValue %= 8 = 1`. Loop runs until `i < 9`. Exits with `i = 9`.
`9 < 9` is false. No tail.

Example: `Buf.size() = 5`, `ShiftedBuf.size() = 10`, `ShiftValue %= 8
= 1`. Loop runs until `i < Buf.size() - 1 = 4`. Exits with `i = 4`.
`4 < 10` is true. Writes `ShiftedBuf[4]`. But expression is
`ShiftedBuf[Buf.size() - 1] = ShiftedBuf[4]`. OK.

Example: `Buf.size() = 10`, `ShiftedBuf.size() = 5`, `ShiftValue %= 8
= 1`, `ShiftValue / 8 = 0` (so `ShiftedLen = 5`). Loop runs until
`i < 9 && i < 5`. Exits with `i = 5`. `5 < 5` false. No tail.

Example: `Buf.size() = 6`, `ShiftedBuf.size() = 10`, `ShiftValue / 8
= 0`, `ShiftValue %= 8 = 1`. Loop runs until `i < 5`. Exits with
`i = 5`. `5 < 10` true. Writes `ShiftedBuf[5] = Buf[5] << 1`. OK
because 5 < 10.

Example exposing the bug: `Buf.size() = 10`, `ShiftedBuf.size() = 10`,
`ShiftValue = 1 % 8`. Loop runs until `i < 9 && i < 10`, i.e. up to `i
= 9` (exits when `i == 9`). `9 < 10` true. Writes `ShiftedBuf[9] =
Buf[9] << 1`. Write index `Buf.size() - 1 = 9`; `ShiftedBuf.size() =
10`. OK.

Example: `Buf.size() = 15`, `ShiftedBuf.size() = 10`, `ShiftValue = 1
% 8`. Loop runs until `i < 14 && i < 10`. Exits `i = 10`. `10 < 10`
false. No tail.

**It's actually really hard to trigger.** The formula only breaks when
`Buf.size() > ShiftedBuf.size()` **and** `i < ShiftedLen` after the
loop. Given the loop condition `(i < Buf.size() - 1) && (i < ShiftedLen)`,
the post-condition is `i == Buf.size() - 1 || i == ShiftedLen`. If we
exit via `i == Buf.size() - 1` (i.e. Buf is the constraining side),
and that value is < ShiftedLen, then the write happens — and the
written index is `Buf.size() - 1` which equals `i`, which is <
ShiftedLen, which means the write is in-bounds.

So actually this code is correct, just confusingly written. **False
alarm.**

**Status:** NOT a bug. Cosmetic: use `i` not `Buf.size() - 1` for
clarity.

---

<a id="33"></a>
### 3.3 Shift-by-8 in `ShiftDataRight` / `ShiftDataLeft` when `ShiftValue % 8 == 0`

**Where:** `src/lw/LabelWriterDriver.cpp:238` (and mirror in
`src/lm/LabelManagerDriver.cpp:601`):

```cpp
ShiftedBuf[ShiftedOffset + i] = (Buf[i - 1] << (8 - ShiftValue)) | (Buf[i] >> ShiftValue);
```

**What:** When `ShiftValue == 0`, `(8 - ShiftValue)` is 8, and
`Buf[i-1] << 8` is a shift of an `unsigned char` by 8 bits. After
integer promotion to `int`, the result is well-defined (promoted
width is 32 on all supported platforms, and the result fits). So this
**is not undefined behaviour** in the C++ sense.

However:

- The result is `Buf[i-1] * 256`, which exceeds the range of an
  unsigned char. Storing it via `|= …` back into a byte truncates to
  zero. The `OR` clause becomes `0 | (Buf[i] >> 0)` = `Buf[i]`. Net
  effect: `ShiftedBuf[ShiftedOffset + i] = Buf[i]`. That is, in the
  byte-aligned case, the shift is a memcpy. Which is *semantically
  correct*. So not a bug.

- `-Wshift-count-overflow` on some GCC versions may flag this; but
  8 < CHAR_BIT of the promoted type, so it doesn't actually fire.

`CHANGES.md` mentions this "deliberately-not-fixed" under "shift-by-8
UB on byte type when `ShiftValue == 0`." I partially disagree with
the characterisation (it's not UB, just wasteful), but the
behaviour-is-correct finding stands.

**Status:** Semantically correct even though suspicious-looking.
Keep `CHANGES.md` note.

---

<a id="34"></a>
### 3.4 `CLabelManagerDriver::GetShiftValue()` — signed/unsigned mixed arithmetic

**Where:** `src/lm/LabelManagerDriver.cpp:637-641`

```cpp
int
CLabelManagerDriver::GetShiftValue(size_t RasterLineSize)
{
    return (MaxPrintableWidth_ - RasterLineSize * 8) / 2 + TapeAlignmentOffset_;
}
```

**What:** `MaxPrintableWidth_` and `RasterLineSize * 8` are both
`size_t`. If `RasterLineSize * 8 > MaxPrintableWidth_`, the subtraction
underflows to near `SIZE_MAX`; dividing by 2 gives a huge positive
value, then `+ TapeAlignmentOffset_` (an `int`) promotes that to
unsigned and adds. The final implicit conversion to `int` return type
produces garbage.

In practice `ProcessRasterLine()` at line 187-188 truncates `b` to
`GetMaxBytesPerLine()` *before* calling `ProcessRasterLineInternal`,
and `GetMaxBytesPerLine() = MaxPrintableWidth_ / 8`, so
`b.size() * 8 <= MaxPrintableWidth_`. So the underflow doesn't occur
on the main path.

But a caller that bypasses the truncation (there aren't any currently,
but `SendCachedRasterLines` at line 262-274 reassigns `b` after
byte-reverse — safe because size is preserved) could trigger it.
Future-proofing would cast to signed first:

```cpp
return (int(MaxPrintableWidth_) - int(RasterLineSize) * 8) / 2 + TapeAlignmentOffset_;
```

**Status:** Latent, not currently reachable. Not mentioned in
`CHANGES.md`.

---

<a id="35"></a>
### 3.5 `CheckStatusAndReprint` — spurious ESC-A writes to the main stdout stream

**Where:** `src/lw/LabelWriterLanguageMonitor.cpp:207-211` (also
called from the loop at `150-153`).

```cpp
buffer_t RequestStatusCommand = CLabelWriterDriver::GetRequestStatusCommand();
Environment_.WriteData(RequestStatusCommand);
```

**What:** `Environment_.WriteData` for the LM environment goes to
`fd 1` (stdout) — the same pipe as raster data. Each ESC-A request
inserts two bytes `{0x1B, 'A'}` into the raster stream the CUPS USB
backend is consuming. LabelWriter firmware treats ESC-A as a status-
request command *mid-stream*; this generally works but is non-
standard and has been implicated in the zombie-job bug (see
`CHANGES.md` Fix 1 / Fix 2).

With Fix 1 (`IsBackchannelSupported()` → `false`) applied, this is
bypassed entirely. With Fix 2 applied, the request-storm is bounded.
Still, a cleaner design would dedicate a side channel for these
requests or omit them when the back-channel is unreliable.

**Status:** Known. Addressed by Fix 1 / Fix 2.

---

<a id="36"></a>
### 3.6 `ReverseByte()` produces wrong result for `Value == 0x00`

**Where:** `src/lm/LabelManagerDriver.cpp:240-259`

```cpp
static byte
ReverseByte(byte Value)
{
    byte ReversedByte = 0;
    size_t BitsCopied = 0;

    while (Value)          // <-- short-circuits for 0x00
    {
        ReversedByte <<= 1;
        if (Value & 0x1)
            ReversedByte |= 0x1;
        Value >>= 1;
        ++BitsCopied;
    }

    ReversedByte <<= 8 - BitsCopied;
    return ReversedByte;
}
```

**What:** For `Value == 0`, the while loop never executes,
`BitsCopied == 0`, and the final statement becomes `ReversedByte <<=
8`. By integer-promotion rules that's `0 << 8 = 0`, which is the
correct answer — so `Value == 0` gives `0`. OK.

For `Value == 0x01`:
- Loop: `Value & 0x1 = 1`, `ReversedByte = 1`, `Value = 0`,
  `BitsCopied = 1`, loop exits.
- Final shift: `1 <<= 7` = `0x80`. Expected: `0x80`. OK.

For `Value == 0x80`:
- Loop: bit 0 is 0 (Value & 1 = 0), ReversedByte stays 0, Value = 0x40,
  BitsCopied = 1. Continue.
- … seven more iterations, each with bit 0 = 0 → ReversedByte = 0,
  Value shifts down, BitsCopied goes up. After 8 iterations Value = 0,
  BitsCopied = 8, ReversedByte = 0.

Wait actually if Value = 0x80 = 0b10000000, the first iteration has
`Value & 0x1 = 0`, so no OR. Shift Value right: 0x40. Repeat: 0x20.
… until Value = 0x01. Iteration 8: Value & 1 = 1, OR 1 into
ReversedByte (which has been shifted left seven times from 0, so is
still 0). ReversedByte = 0, shift left once more to 0, then set bit 0
to 1 → ReversedByte = 1. Value shifts right to 0.

Now BitsCopied = 8. Final shift: `1 <<= 0` = 1. Expected reverse of
0x80 is 0x01. OK.

For `Value == 0xC0 = 0b11000000`:
- Iter 1: bit 0 = 0, ReversedByte <<= 1 → 0. Value = 0x60. BitsCopied = 1.
- Iter 2: bit 0 = 0, ReversedByte = 0. Value = 0x30. BitsCopied = 2.
- Iter 3: bit 0 = 0, Rb = 0. Value = 0x18. BC = 3.
- Iter 4: bit 0 = 0, Rb = 0. Value = 0x0C. BC = 4.
- Iter 5: bit 0 = 0, Rb = 0. Value = 0x06. BC = 5.
- Iter 6: bit 0 = 0, Rb = 0. Value = 0x03. BC = 6.
- Iter 7: bit 0 = 1. Rb <<= 1 (still 0), then |= 1 → Rb = 1. Value =
  0x01. BC = 7.
- Iter 8: bit 0 = 1. Rb <<= 1 → 2, then |= 1 → 3. Value = 0. BC = 8.
- Loop exits.
- Final shift: `3 << 0` = 3.
Expected reverse of 0xC0 = 0b11000000 is 0b00000011 = 3. OK.

For `Value == 0x06 = 0b00000110`:
- Iter 1: bit 0 = 0. Rb = 0. Value = 0x03. BC = 1.
- Iter 2: bit 0 = 1. Rb <<= 1 → 0, |= 1 → 1. Value = 0x01. BC = 2.
- Iter 3: bit 0 = 1. Rb <<= 1 → 2, |= 1 → 3. Value = 0. BC = 3.
- Loop exits.
- Final shift: `3 << 5` = 0x60.
Expected reverse of 0x06 = 0b00000110 is 0b01100000 = 0x60. OK.

**Conclusion:** `ReverseByte` is correct. **False alarm.**

**Status:** NOT a bug.

---

<a id="37"></a>
### 3.7 `CupsPrintEnvironment.cpp` — missing `<unistd.h>` for `write()`

**Where:** `src/common/CupsPrintEnvironment.cpp:63, 133` call
`write(1, ...)`, but the file only includes `<stdio.h>`, `<string>`,
its own header, `<errno.h>`, `<cups/cups.h>`, `<cups/sidechannel.h>`,
and `<cassert>`. `write()` is declared in `<unistd.h>`, which is not
directly included.

**What:** On GCC + glibc, `<stdio.h>` pulls in `<features.h>` which
may transitively expose `write()`. On a stricter toolchain (e.g.
libc++ on macOS, or musl) `write` won't be visible and the TU won't
compile.

**Why it's wrong:** Header discipline. A TU should #include the
headers whose declarations it uses.

**How to fix:** Add `#include <unistd.h>` at the top of the file.

**Status:** Not mentioned in `CHANGES.md`. The patched build on
Ubuntu 25.10 happens to compile because of transitive includes.

---

<a id="38"></a>
### 3.8 `CupsFilter.h` — missing `<fcntl.h>` for `O_RDONLY` / `open()`

**Where:** `src/common/CupsFilter.h:136-145`

```cpp
if ((fd = open(argv[6], O_RDONLY)) == -1)
```

**What:** `O_RDONLY` is in `<fcntl.h>`; `open()` is in `<fcntl.h>`
per POSIX (and `<sys/stat.h>` / `<sys/types.h>` for supporting
macros). None of these are #included; `<unistd.h>` only declares
`write/read/close`, not `open`.

**Why it's wrong:** Same discipline issue. Compiles today because of
`<cups/cups.h>` → `<stdio.h>` → `<fcntl.h>` transitive chain on glibc.

**How to fix:** Add `#include <fcntl.h>` to the top of the file.

**Status:** Not mentioned in `CHANGES.md`.

---

<a id="39"></a>
### 3.9 `CupsFilter.h` — `sigaction` initialised field-by-field, not zero-initialised

**Where:** `src/common/CupsFilter.h:115-128`

```cpp
struct sigaction ignoreAction;
ignoreAction.sa_handler = SIG_IGN;
sigemptyset(&ignoreAction.sa_mask);
ignoreAction.sa_flags = 0;
sigaction(SIGPIPE, &ignoreAction, NULL);
```

**What:** `struct sigaction` on glibc contains an internal
`sa_restorer` pointer that is supposed to be NULL/zero for user
code. The code above sets `sa_handler`, `sa_mask`, and `sa_flags`,
but leaves `sa_restorer` (and any other implementation-private
fields) with their indeterminate local-stack values.

On glibc, `sa_restorer` is now unused (kernel does it), so this is
almost certainly harmless. On musl and older glibc versions the
indeterminate value could in principle cause the kernel to
misbehave, though in practice `SA_RESTORER` is only honoured if the
flag is set.

**Why it's wrong:** Defensive initialisation. Best practice is
`memset(&ignoreAction, 0, sizeof(ignoreAction));` or
`struct sigaction ignoreAction = {};` at declaration time.

**How to fix:**

```cpp
struct sigaction ignoreAction = {};
ignoreAction.sa_handler = SIG_IGN;
...
```

**Status:** Added by Fix 5 with a minor cleanliness gap.

---

<a id="310"></a>
### 3.10 `CupsFilter.h` — cancel flag is not polled inside the per-line raster loop

**Where:** `src/common/CupsFilter.h:168-172` (outer loop), `212-241`
(inner per-line loop).

```cpp
for (size_t y = 0; y < PageHeader.cupsHeight; ++y)
{
    if ((y & 15) == 0) fprintf(stderr, ...);
    ...
}
```

**What:** `gCancelRequested` is checked once per *page*
(`163-172`), not per *raster line*. A large label with many thousand
lines will, between SIGTERM arrival and the next page boundary,
continue to emit raster data. The label in progress will complete
printing, not be cut short.

Fix 5 explicitly targets this behaviour and the comment says "break
out of the page loop so EndDoc runs". That is indeed the chosen
semantics. But if the SIGTERM arrives on the **first** page of a
very large multi-hundred-line job, the whole label continues.

**Why it's wrong (or by-design?):** This is a design trade-off the
author made for clean EndDoc emission. Arguably correct: mid-page
cancel would leave the printer at an arbitrary row with no trailing
form-feed. I list it here for the assessor to consider whether the
trade-off is intentional.

**Status:** By design per Fix 5 comment. Not a bug, a design choice.
Including for completeness.

---

<a id="311"></a>
### 3.11 `raster2dymolm.cpp` — `IsBackchannelSupported()` still hardcodes `true`

**Where:** `src/lm/raster2dymolm.cpp:39-43`

```cpp
static bool
IsBackchannelSupported()
{
    return true;
}
```

**What:** Fix 1 flipped the corresponding LW probe to `return false`
to avoid the zombie-job hang. The LM tree still returns `true`, which
means LabelManager prints go through `CLabelManagerLanguageMonitor`
with its status-polling loop (§2.2). If the same USB back-channel
unreliability on modern CUPS affects LM hardware, LM prints could
hang the same way.

`CHANGES.md` is clear that the author's testing was on LW 450 Turbo
only. There is no evidence the LM back-channel is broken, but there
is also no evidence it's working. Worth flagging so the next hardware
test covers LM.

**How to fix:** Either flip to `false` for symmetry (and lose LM
status reporting) or leave as-is and document the risk.

**Status:** Not in `CHANGES.md`. Coverage gap of Fix 1.

---

<a id="4-minor"></a>
## 4. Minor

Edge-case correctness and cosmetic.

<a id="41"></a>
### 4.1 `SendBytesPerLine` / `SendDotTab` — single-byte encoding truncates silently

**Where:** `src/lw/LabelWriterDriver.cpp:430-453` and
`src/lm/LabelManagerDriver.cpp:525-562`.

```cpp
void
CLabelWriterDriver::SendBytesPerLine(size_t Value)
{
    byte buf[] = {ESC, 'D', 0};
    buf[2] = Value;            // <-- truncates silently for Value > 255
    SendCommand(buf, sizeof(buf));
}
```

**What:** The protocol uses `ESC D <n>` with a single-byte count. A
4XL-width page can exceed 255 bytes per line. The assignment
`buf[2] = Value` truncates without warning.

`CHANGES.md` lists this under "deliberately-not-fixed" for the
4XL/wide-media case.

**Status:** Known, left alone.

---

<a id="42"></a>
### 4.2 `CupsFilterLabelManager.cpp` — "DYMO LabelLabelWriter DUO Tape" typo

**Where:** `src/lm/CupsFilterLabelManager.cpp:243-250`

```cpp
// adjust tape center
if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelLabelWriter DUO Tape"))
{
    ...
}
```

**What:** `"DYMO LabelLabelWriter DUO Tape"` is a typo: "Label" is
duplicated. No PPD ever sets `ModelName` to that string. The whole
block is dead code — its tape-alignment overrides are never applied.

**Why it's wrong:** The block immediately *above* it at line 217-223
has the correct `"DYMO LabelWriter DUO Tape"` string, with the same
offsets (-2 for 6mm, -1 for 9mm). So this looks like a fat-fingered
duplicate. The bug is that it's a no-op.

**How to fix:** Delete the block, or correct the string to whatever
DYMO model was originally intended (possibly `"DYMO LabelWriter 450
DUO Tape"` — the 450-family DUO also has a single-Label variant).

**Status:** Not mentioned in `CHANGES.md`.

---

<a id="43"></a>
### 4.3 `CupsFilterLabelWriter.cpp` — no 300 DPI mapping for 450-series PPDs

**Where:** `src/lw/CupsFilterLabelWriter.cpp:26-51`

Author is aware (see Fix 6 comment in file and `CHANGES.md`).

**Status:** Documented; no behavioural bug in practice.

---

<a id="44"></a>
### 4.4 `TestLabelManagerFilter.cpp` — `ppdClose` not called for `lm400.ppd` / `lp350.ppd`

**Where:** `src/lm/tests/TestLabelManagerFilter.cpp:202-224`

```cpp
ppd = ppdOpenFile("../../../ppd/lm400.ppd");
CPPUNIT_ASSERT(ppd != NULL);
...
// no ppdClose(ppd) here before next assignment

ppd = ppdOpenFile("../../../ppd/lp350.ppd");
```

**What:** Two PPDs are opened and their pointers overwritten without
calling `ppdClose` on the previous pointer. This leaks the PPD
structures. Not a runtime bug for a short-lived test, but would
matter if the test were looped or extended.

Similarly `lp350.ppd` is overwritten by `lmpc.ppd` without closure.

**How to fix:** Add `ppdClose(ppd)` before each reassignment.

**Status:** Minor test-only leak. Not in `CHANGES.md`.

---

<a id="45"></a>
### 4.5 `testsMain.cpp` — `wasSucessful` typo

**Where:** `src/common/tests/testsMain.cpp:48`

```cpp
bool wasSucessful = runner.run();
return wasSucessful ? 0 : 1;
```

**What:** Typo of "was_successful". Cosmetic only.

**Status:** Not in `CHANGES.md`. Trivial.

---

<a id="46"></a>
### 4.6 `CCupsPrintEnvironmentForLM::SetJobStatus` — `assert(0)` in `default:` branch

**Where:** `src/common/CupsPrintEnvironment.cpp:204-206`

```cpp
default:
    assert(0);
```

**What:** On release builds `NDEBUG` is defined and `assert` expands
to nothing. An unknown `JobStatus` then silently skips the
`fputs("STATE: …", stderr)` line and the function continues.

Combined with the out-of-enum `jsDeleted` (which nothing in the
codebase ever sets but is a valid enum value), this is at worst a
missed `STATE:` output. Not harmful, but `assert` in release builds
is not a serious form of error-handling. Same pattern in other
`default: assert(0);` sites:
- `src/common/Halftoning.cpp:96, 110, 127, 144, 166`
- `src/lw/tests/MOCK_LWLMPrintEnvironment.cpp:81, 107`
- `src/lw/LabelWriterDriver.cpp:69, 506`

**How to fix:** Either replace `assert(0)` with `throw`/`abort` for
fatal cases, or log and continue for non-fatal.

**Status:** Not in `CHANGES.md`.

---

<a id="47"></a>
### 4.7 `LabelManagerLanguageMonitor::status_bits` — `AUTO_CUTTER_BIT` and `NO_POWER_BIT` both `0x80`

**Where:** `src/lm/LabelManagerLanguageMonitor.h:36-49`

```cpp
enum status_bits
{
    ...
    AUTO_CUTTER_BIT     = 0x80,
    NO_POWER_BIT        = 0x80,
    INCORRECT_SIZE_BIT  = 0xFF
};
```

**What:** Two enumerators share the same value. Neither is actually
used in `LabelManagerLanguageMonitor.cpp` at present. Dead code if
they were to be referenced, this overlap would be a source of
confusion — is a status byte of `0x80` "auto cutter" or "no power"?

**How to fix:** Either delete both (if unused) or clarify which one
is correct based on DYMO programming docs.

**Status:** Not in `CHANGES.md`.

---

<a id="48"></a>
### 4.8 `CLabelWriterLanguageMonitor::SetJobStatus(byte)` — PAPER_FEED mapped to `jsPaperOut`

**Where:** `src/lw/LabelWriterLanguageMonitor.cpp:307-318`

```cpp
if ((Status & PAPER_OUT_BIT) || (Status & PAPER_FEED_BIT))
    JobStatus = IPrintEnvironment::jsPaperOut;
else if (Status & ERROR_BIT)
    JobStatus = IPrintEnvironment::jsError;
```

**What:** The enum says `PAPER_FEED_BIT = 0x40` is a "paper feeding"
state — i.e. the printer is actively feeding paper. Conflating that
with `PAPER_OUT_BIT = 0x20` (no paper) means a normal-operation
"feeding" status is reported as "out of paper" to CUPS. Downstream,
this causes `printer-state-reasons=com.dymo.out-of-paper-error` to
be set spuriously.

**Why it's wrong:** Semantic mismatch. PAPER_FEED is a transient
normal state; PAPER_OUT is a fault. They should not map to the same
job-status enum.

This could also conflict with the printer's own state-reporting —
during a normal roll change the feed bit is set, and reporting
"out of paper" to CUPS may cause the scheduler to pause the queue.

**How to fix:** Separate the two cases. PAPER_FEED should probably
map to `jsOK` (normal operation) or a new enum state.

**Status:** Not in `CHANGES.md`. With Fix 1 disabling the LM on LW,
this is latent on the happy path. But anyone re-enabling the LM
would hit it.

---

<a id="49"></a>
### 4.9 `CNLLHalftoning::ProcessImage` — `RowCount` formula

**Where:** `src/common/NonLinearLaplacianHalftoning.cpp:303`

```cpp
size_t RowCount = (InputImage.size() + 1) / 3 + 1;
```

**What:** For `InputImage.size() = 10` (the test image in
TestNLLHalftoning), RowCount = (10+1)/3 + 1 = 3 + 1 = 4. Each row
advances `y1` by 3, so y1 values are 0, 3, 6, 9 — fine for height
10. For `InputImage.size() = 1`, RowCount = 0/3 + 1 = 1; y1 = 0.
OK.

For `InputImage.size() = 2`, RowCount = 1 + 1 = 2; y1 = 0, 3. The
second row starts at y=3 but the image is only 2 tall. Since
`CNLLBlock::IsInImage(x, y)` returns false for y >= image height,
this is harmless — blocks outside the image just produce no output.

So wasteful but not incorrect. Not worth fixing unless profiling
shows NLL path matters.

**Status:** Not a bug. Informational only.

---

<a id="5-informational"></a>
## 5. Informational / style

These are not bugs but would improve code quality.

- **Pervasive `assert(0)` usage** — see 4.6. Consider replacing with
  `abort()` or a log-and-fail helper so release builds still flag
  impossible states.
- **`byte` typedef vs. C++17 `std::byte`** — the codebase defines its
  own `typedef unsigned char byte;` in `CommonTypedefs.h`. C++17
  introduces `std::byte`; the project typedef would shadow it on
  any file that does `using namespace std;` inside the DymoPrinterDriver
  namespace. Currently safe because no file does that.
- **Printable-debug `fprintf` everywhere** — even normal happy-path
  operation spews `DEBUG:` lines to stderr. CUPS captures stderr but
  this pollutes logs. A compile-time `#ifdef DEBUG` would help.
- **Test resource management** — `TestLabelManagerFilter.cpp` leaks
  a couple of PPDs (§4.4). The test harness runs once per process
  so it doesn't matter, but it sets a bad example.
- **Namespace style inconsistency** — `src/common/` files
  sometimes terminate `namespace` blocks with `};` (a stray
  semicolon that GCC accepts as an empty declaration at namespace
  scope). `PrinterDriver.h:95`, `CommonTypedefs.h:35`,
  `CupsPrintEnvironment.h:65`, `ErrorDiffusionHalftoning.h:54` all
  have this. Harmless but `-pedantic` flags it.
- **`volatile sig_atomic_t gCancelRequested`** — there is a
  non-`volatile` copy of this variable in each TU that includes
  `CupsFilter.h` via inline extern "C" hack at line 50. Since the
  header declares it `static`, each TU gets its own copy. That works
  in this codebase because only one TU instantiates
  `CCupsFilter::Run()` per binary, but the pattern is fragile —
  if `raster2dymolw.cpp` and some future TU both include the header
  and both Run() in the same process, each would have its own
  independent flag. Move the definition into a `.cpp` file with
  `extern` declaration in the header for robustness.

---

<a id="6-cross-reference"></a>
## 6. Cross-reference with `CHANGES.md`

The author's fork already documents seven fixes. The following table
maps each of my findings to the corresponding `CHANGES.md` entry if
any:

| BUGS.md | CHANGES.md                   | Status                         |
|---------|------------------------------|--------------------------------|
| 1.1     | —                            | **New critical finding**       |
| 1.2     | Fix 3 (same class, LM variant)| **New critical finding**       |
| 2.1     | —                            | **New major finding**          |
| 2.2     | Fix 2 (LW side only)         | **LM counterpart not fixed**   |
| 2.3     | "deliberately-not-fixed" (LW)| LM counterpart not called out  |
| 2.4     | "deliberately-not-fixed"     | Same as CHANGES.md             |
| 2.5     | —                            | **New major finding**          |
| 2.6     | —                            | Verified NOT a bug             |
| 2.7     | —                            | Verified NOT a bug             |
| 3.1     | "deliberately-not-fixed"     | Same as CHANGES.md             |
| 3.2     | —                            | Verified NOT a bug             |
| 3.3     | "deliberately-not-fixed"     | Not UB in practice             |
| 3.4     | —                            | New minor finding              |
| 3.5     | Addressed by Fix 1/Fix 2     | Documented                     |
| 3.6     | —                            | Verified NOT a bug             |
| 3.7     | —                            | **New finding (header hygiene)**|
| 3.8     | —                            | **New finding (header hygiene)**|
| 3.9     | —                            | Related to Fix 5               |
| 3.10    | —                            | By-design per Fix 5 comment    |
| 3.11    | Fix 1 (LW only)              | **LM tree still zombie-prone** |
| 4.1     | "deliberately-not-fixed"     | Same as CHANGES.md             |
| 4.2     | —                            | **New minor finding**          |
| 4.3     | Fix 6                        | Documented                     |
| 4.4     | —                            | New test-quality finding       |
| 4.5     | —                            | Trivial typo                   |
| 4.6     | —                            | Style                          |
| 4.7     | —                            | Dead-code cleanup              |
| 4.8     | —                            | **New finding (latent)**       |
| 4.9     | —                            | Not a bug                      |

**Genuine new bugs the author should address:**

1. (1.1) LM `IsLocal()` NULL-deref — **critical, 1-line fix.**
2. (1.2) `TapeWidth_` uninitialised — **critical, add to ctor init.**
3. (2.1) NLL unsigned underflow — **latent, worth fixing.**
4. (2.2) LM-side status-polling infinite-loop risk — **mirror
   Fix 2 on the LM side.**
5. (2.5) LM `Status[1]` OOB read on single-byte responses —
   **defensive `size() >= 2` guard.**
6. (3.7/3.8) missing `<unistd.h>` / `<fcntl.h>` — **compile-
   portability.**
7. (3.11) LM-side `IsBackchannelSupported() = true` same zombie-
   risk as LW had before Fix 1 — **probably should mirror Fix 1.**
8. (4.2) "DYMO LabelLabelWriter DUO Tape" typo — **dead code,
   delete or correct string.**
9. (4.8) PAPER_FEED_BIT mapped to jsPaperOut — **semantic bug in
   LW status reporting (latent because Fix 1 disables LM).**

Everything else is documented, intentional, or verified not-a-bug.

---

*End of BUGS.md.*
