# Changes in the loadfix fork

This fork of `matthiasbock/dymo-cups-drivers` fixes a group of bugs that
cause DYMO LabelWriter 450 Turbo (and the rest of the LabelWriter 450
series: 450, 450 DUO Label, 450 Twin Turbo) prints to hang on modern
Linux. The user-visible symptom is that one job prints, then every
subsequent job queues behind a "zombie" entry marked *now printing* in
`lpstat -p` output. The print queue only recovers after
`cancel -a <printer>` **and** `sudo systemctl restart cups`.

Under the hood all fixes target one subsystem — the back-channel status
monitor in `src/lw/LabelWriterLanguageMonitor.cpp` and the CUPS filter
harness in `src/common/CupsFilter.h` — plus a build-system adjustment so
the whole thing compiles on modern GCC.

Each fix below corresponds to one commit on the `fix/450-turbo-zombie-jobs`
branch. Each commit message contains additional rationale; this file is
the roll-up.

---

## Fix 1 — Disable buggy language-monitor back-channel polling

**File:** `src/lw/raster2dymolw.cpp`
**Commit title:** Fix 1: Disable buggy language-monitor back-channel status polling

`IsBackchannelSupported()` was hardcoded to return `true`. This forced
`raster2dymolw` to instantiate `CLabelWriterLanguageMonitor` on every job.
That class is supposed to poll the printer for status bytes between pages
so the driver can detect paper-out errors and reprint. In practice the
back-channel path is unreliable on modern CUPS + USB, and the polling
loop spins on a read that never returns useful data. The filter
eventually exits but leaves pending status bytes in the USB backend's
write pipe, the backend sits waiting for replies that never arrive, and
the CUPS scheduler never sees the job complete.

**Change:** return `false`. The filter now uses `CDummyLanguageMonitor`
instead. All printer-essential ESC commands (reset, label length, form
feed, short form feed, density, quality) are emitted by the driver
(`CLabelWriterDriver` / `CLabelWriterDriver400`) regardless of which
language monitor is wired in; the dummy just skips the broken status
polling.

**What we lose:** paper-out state reporting to CUPS (no more
`printer-state-reasons=com.dymo.out-of-paper-error`) and automatic
reprint-on-error. Neither is essential for a working print of a good
job; the printer still advances correctly, and if it runs out of paper
the user notices from the silent printer rather than from CUPS's web UI.

**Root cause:** this is the zombie bug. Fixes 2–5 are defence-in-depth
for the same code path if it is ever re-enabled.

---

## Fix 2 — Reinstate `StatusOK` guard in `CheckStatusAndReprint()`

**File:** `src/lw/LabelWriterLanguageMonitor.cpp`

Inside `CheckStatusAndReprint()` the loop that repeatedly issues ESC-A
status requests had its read-failure early-exit commented out. With
`Status` seeded to `0` and `StatusOK` unused, the loop condition
`!((Status & TOF_BIT) || (Status & ERROR_BIT) || (Status & ROLL_CHANGED_BIT))`
is always true until the 10-second wall-clock `ReadStatusTimeout_`
fires. During those 10 seconds the loop emits one ESC-A byte per
iteration into the main stdout stream — polluting the raster pipeline.

**Change:** uncomment the `StatusOK` assignment, use it in the loop
condition, and add an explicit `break` with a log line when the read
fails. The loop now exits in O(100 ms) rather than 10 s on an
unresponsive back-channel.

**Necessary even after Fix 1?** Yes, because:
  * It closes the door if anyone re-enables the language monitor.
  * It dramatically shortens the ESC-A flood in the interim build before
    a downstream user applies the full fix set.

---

## Fix 3 — Initialise `CCupsPrintEnvironmentForLM` members

**File:** `src/common/CupsPrintEnvironment.cpp`

`CCupsPrintEnvironmentForLM::PRNFile_` and `JobStatus_` were left
uninitialised by the default constructor. `JobStatus_` is read by
`CCupsFilter::Run()` at the top of every page iteration (via
`PrintEnvironmentForLM_.GetJobStatus() != IPrintEnvironment::jsOK`)
before any code has had a chance to call `SetJobStatus()`. Reading
an uninitialised non-static member is undefined behaviour
([basic.indet]); in practice it was the garbage value of that memory
region, which on some executions matched one of the `job_status_t`
error enumerators and caused spurious early-aborted jobs.

**Change:** ctor initialiser list sets `PRNFile_ = NULL` and
`JobStatus_ = jsOK`. Zero behavioural downside; fixes a class of
"random print aborts with no obvious cause" reports.

---

## Fix 4 — Bound `PollUntilPaperIn()` and scan last status byte

**File:** `src/lw/LabelWriterLanguageMonitor.cpp`

Two sub-issues in the status-reading code path.

(a) `PollUntilPaperIn()` was an unbounded `for (;;)` loop. Its only
documented escape was `Environment_.GetJobStatus() == jsDeleted`, but
nothing in the entire codebase ever sets `jsDeleted` — on modern CUPS,
job cancels arrive as SIGTERM, not as a status write. In practice the
loop only exits on `ReadStatus()` failure; when the printer keeps
answering with a non-TOF status, the filter can spin there for a long
time. Added a wall-clock deadline equal to `ReadStatusTimeout_` so the
loop gives up after the configured budget.

(b) `ReadStatus()` took `buf[0]` from the back-channel buffer. But
`cupsBackChannelRead` can return multiple bytes in one call (one per
previous ESC-A request we issued), in chronological order (oldest
first). Using the first byte therefore reports the *previous* status,
not the one we just asked about. Changed to `buf[buf.size() - 1]` —
the most recent reply.

---

## Fix 5 — Signal handlers, stdout close, empty-input return code

**File:** `src/common/CupsFilter.h`

Three changes to `CCupsFilter::Run()`, all addressing how the filter
terminates.

(a) **Signal handlers.** Upstream installed none. Default SIGPIPE
behaviour is to kill the process — so a failing write to the CUPS USB
backend pipe takes the filter down without running EndDoc (which emits
the final ESC E form feed) or closing stdout. Default SIGTERM is the
same, turning a user-triggered cancel into a half-printed label and a
wedged backend. Added SIGPIPE ignore, and a SIGTERM/SIGINT handler that
sets a `gCancelRequested` flag polled by the main page loop — allowing
the loop to break and fall through to the clean-exit path.

(b) **Explicit `fflush(stdout); close(1);` before return.** Upstream
left `close(1)` commented out. The CUPS USB backend reads from the pipe
connected to this filter's stdout; until that pipe closes, the backend
keeps a `read()` outstanding and the scheduler considers the job still
in flight. On clean return-from-main the C runtime closes fd 1 via
atexit cleanup — but only after running static destructors and library
teardown, which can be milliseconds late. Closing explicitly gives the
backend an immediate EOF and lets the scheduler retire the job.

(c) **Return 0 on empty input.** Upstream did `return (Page == 0);`,
which abort-aborts a legitimately empty CUPS-raster job. Returning 0
with an `ERROR:` line on stderr lets the empty job complete cleanly
instead of wedging as `aborted` in the queue.

---

## Fix 6 — Document harmless-but-misleading Resolution mapping

**File:** `src/lw/CupsFilterLabelWriter.cpp`

`ProcessPPDOptions()` only maps the strings `"203dpi"` and
`"203x138dpi"` to the driver's `resolution_t` enum. The 450-series
PPDs declare `"300dpi"` and `"300x600dpi"`, so `SetResolution()` is
silently never called and the printer runs at its factory-default
300 DPI. This happens to be correct for our hardware but would be
surprising to someone reading the code. Added an explanatory comment
at the site. No runtime change.

A proper fix requires extending the `resolution_t` enum in
`src/lw/LabelWriterDriver.h` and emitting the appropriate ESC y / ESC z
opcodes for 300 DPI mode. Out of scope for this series.

---

## Fix 7 — Remove `src_v2/` entirely

**Files:** the whole `src_v2/` tree, plus `Makefile.am`, `configure.ac`.

`src_v2/` was DYMO's unfinished "version 2" rewrite, shipped alongside
`src/` in the 1.4.0.5 tarball but never wired up to install anywhere.
Initial treatment in loadfix1 was just to drop it from `SUBDIRS` and
`AC_CONFIG_FILES` so `make` could succeed on modern toolchains (it
uses `std::auto_ptr`, removed in C++17, and deprecated Boost
`posix_time` APIs).

After the loadfix3 code review (`BUGS.md`) an independent agent
surveyed `src_v2/` for anything worth porting to `src/`. The tree
turned out to be strictly worse than `src/` on every axis we care about
for the LabelWriter 450 family:

  * **Regressions.** Drops the RLE compression path (~150 lines);
    removes the `CLabelWriterDriver400` / `CLabelWriterDriverTwinTurbo`
    subclasses — so Twin Turbo roll selection and the 400-family
    EndDoc / EndPage form-feed sequencing are gone.
  * **Same bugs.** `CCupsPrintEnvironmentForLM::JobStatus_` is still
    uninitialised in its ctor (our Fix 3 is not there).
  * **New bugs.** `raster2dymolw_v2.cpp`'s signal handler calls
    `fputs()` and `Abort()` from signal context — both are not
    async-signal-safe per POSIX. Our Fix 5 uses the correct
    `volatile sig_atomic_t` flag.
  * **Incomplete.** Five member functions are declared in the v2
    headers but never implemented in the .cpp files.
  * **Identical test coverage.** The `src_v2/**/tests/` files are
    byte-for-byte copies of `src/**/tests/`, so the v2 tree adds no
    verification.
  * **The one genuine addition** — LW 550 / 5XL / 5XL Pro / 550 Pro /
    550 Twin Pro support — targets hardware a generation newer than
    the 450 family this fork is scoped to. Shipping that would mean
    building a second filter binary (`raster2dymolw_v2`), shipping
    8 extra PPDs, and maintaining a C++17-port of the v2 tree. Not
    proportional to the payoff for our target users.

Resolution: `git rm -r src_v2/`. The tree's git history remains in
the fork if anyone ever wants to resurrect it. `Makefile.am` and
`configure.ac` were simplified back to their pre-fix-7 layout — no
more apologetic comments about an excluded tree.

---

## Deliberately-not-fixed issues

A code review turned up several smaller issues we chose to leave alone
rather than deviate further from upstream than necessary:

  * **`LabelWriterDriver.cpp:141` — `size_t i >= 0` infinite-loop
    potential.** Masked by the early-return at line 138 in all currently-
    reachable inputs. A `for (size_t i = BufSize; i-- > 0;)` idiom would
    be safer but changes behaviour in no tested input.

  * **`LabelWriterDriver.cpp:238, 255` — shift-by-8 UB on byte type
    when `ShiftValue == 0`.** Never hit by our pipeline because
    `cupsInteger[0]` is always 0 and the shift path keyed off it.

  * **`ShiftDataRight` tail-byte OOB when `PageOffset_.x > 0` and
    divisible by 8.** Same conditions; not reached.

  * **Asymmetric `ImageableArea` in `ppd/lw450t.ppd` for 30336.** These
    numbers came from DYMO and correspond to the printer's actual
    physical printable area; changing them may reveal rather than hide
    content clipping. Left as-is and will be re-evaluated with the
    zombie fix in place.

  * **`ESC D` / `ESC B` single-byte column/row-length encoding.** Fine
    for the 450 Turbo's maximum label widths; a 4XL (4" wide) would need
    multi-byte encoding.

  * **Missing 300 DPI entries in `resolution_t`.** See Fix 6.

All of these are candidates for a follow-up series if anyone plans to
use this fork on a wider printer or exotic media.

---

## Fixes 8–10 (loadfix2) — acting on TECH_REVIEW.md

After the loadfix1 build burned in on real hardware, an external agent
reviewed the tree against "DYMO LabelWriter 450 Series Technical
Reference" Rev. 10/09. The review is committed as `TECH_REVIEW.md`.
Eleven findings were flagged (F-1 through F-11, severities MEDIUM →
INFO). Loadfix2 addresses them all.

### Fix 8 — F-1 MEDIUM: Paper Jam vs. Paper Out

Spec p.10 / p.17 define the ESC-A status byte bit 6 (0x40) as **Paper
Jam**. Upstream DYMO named it `PAPER_FEED_BIT` and routed it through
`SetJobStatus` as `jsPaperOut`, causing the user to see a physical jam
reported as "out of paper". Renamed to `PAPER_JAM_BIT` and routed to
`jsSlotStatusError` (already surfaced via the CUPS environment as
`STATE: com.dymo.slot-status-error`, declared in the LW 450 PPDs).
Activates only if the back-channel language monitor is re-enabled.

### Fix 9 — F-3, F-4, F-7 LOW: Document StartDoc + reset + PageHeight

Three small notes-only changes consolidated:

  * F-3: `SendLineTab(0)` in StartDoc emits `ESC Q 00 00`. Not in the
    LW 450 opcode table; silently ignored by 450 firmware per spec p.9.
    Required for LW 3xx firmware (which shares this base class), so
    removing it would break 3xx. Comment added explaining the
    constraint and what to do if 450-specific divergence is needed.
  * F-4: `GetResetCommand()` returns 156 consecutive `ESC` bytes, which
    is spec p.9's sync-recovery sequence (minimum 85) — NOT the
    spec's "Reset Printer" command (ESC @, 0x1B 0x40) which would
    also restore firmware defaults. The driver re-sets every parameter
    in StartDoc, so final state is deterministic. Comment added to
    make the distinction obvious.
  * F-7: `PageHeight_` default was 0x0800 (2048), not the spec's
    documented firmware default of 3058. Never actually emitted in
    practice (StartPage always recomputes from the CUPS page header)
    but the misleading constant caused confusion. Swapped to 3058.

### Fix 10 — F-5, F-8, F-11 LOW/INFO: Document deliberate deviations

Three performance-advice deviations from the spec that are intentional:

  * F-5: `ESC B n` (dot tab) is emitted per raster line, not "only on
    change" as spec p.11 recommends. The commented-out optimisation
    breaks LW 3xx output per DYMO DLS80AM-1421. Inline comment
    expanded to explain.
  * F-8: Quality / density / dot-tab / bytes-per-line are re-sent on
    every job even when matching firmware defaults. Accepted trade-off
    for deterministic post-reset state. Covered by the StartDoc
    comment added in Fix 9.
  * F-11: `MaxPrintWidth_ = 156` for LW 4XL exceeds the LW 450 Series
    spec's 84-byte cap. 4XL is outside this spec document's scope
    (different physical hardware, different print-head width). Comment
    added to the override block explaining what each per-model value
    corresponds to and why 4XL looks out-of-bounds.

### Findings F-2, F-6, F-9, F-10 — already addressed or N/A

  * F-2 ROLL_CHANGED_BIT (LOW) — Twin-Turbo-specific undocumented
    extension; dead bit on single-roll LW 450. The enum comment block
    added in Fix 8 already documents this.
  * F-6 cosmetic PPD Resolution option (LOW) — already documented at
    the code site in loadfix1's Fix 6. No source change.
  * F-9 disabled back-channel (INFO) — intentional and spec-compliant
    per the review's own analysis. Already documented at the
    `IsBackchannelSupported()` site in Fix 1.
  * F-10 unimplemented ESC @ / ESC * / ESC V (INFO) — spec commands a
    print filter does not need.

---

## Testing

Tested on Ubuntu 25.10 with CUPS 2.4.16 against a DYMO LabelWriter
450 Turbo (VID 0922:0021) over USB, printing wine labels from
CellarTracker at 30336 media size (1" × 2⅛"). The zombie-job pattern
that was present on every second print under the Ubuntu-packaged
`printer-driver-dymo` 1.4.0-12build3 does not occur with the patched
filter installed. Prints complete cleanly; `lpstat -p` returns to
`idle` immediately after each job.

---

## Upstream

These patches will be submitted upstream to
`matthiasbock/dymo-cups-drivers` as individual PRs once they have burn-in
time on real hardware. In the meantime users can build a Debian package
from this tree — see `debian/` and `build-deb.sh`.
