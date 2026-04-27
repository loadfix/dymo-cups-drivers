---
title: DYMO LabelWriter 450 Series — CUPS Driver Conformance Review
spec_document: docs/LW 450 Series Technical Reference.pdf
spec_revision: "Rev. 10/09 (© 2009 Sanford, L.P.)"
spec_scope: LabelWriter 450, 450 Turbo, 450 Twin Turbo, 450 Duo (Label + Tape)
driver_under_review: dymo-cups-drivers (upstream DYMO 1.4.0 + local loadfix1 patches)
driver_scope_reviewed:
  source_tree: src/
  ppds:
    - ppd/lw450.ppd    # LabelWriter 450
    - ppd/lw450t.ppd   # LabelWriter 450 Turbo
    - ppd/lw450tt.ppd  # LabelWriter 450 Twin Turbo
    - ppd/lw450dl.ppd  # LabelWriter 450 Duo (Label side)
    - ppd/lw450dt.ppd  # LabelWriter 450 Duo (Tape side)
review_date: 2026-04-27
reviewer_role: technical_reviewer (no source changes made)
severity_levels:
  blocker:     spec violation that will cause incorrect prints or break the protocol
  high:        spec violation visible to users under common configurations
  medium:      spec violation or semantic mismatch with user-visible consequences
  low:         deviation that works by accident or is out-of-scope of the emitted bytes
  info:        non-conformance worth flagging but not a defect
---

# 1. Build-system reconnaissance

## 1.1 `src_v2/` is not built — confirmed stale

**Status:** CONFIRMED stale. Safe to ignore for this review.

Evidence:
- `dymo-cups-drivers/Makefile.am:26` sets `SUBDIRS = src ppd docs samples` — `src_v2` is NOT a subdir.
- `dymo-cups-drivers/Makefile.am:20-25` contains an explicit comment explaining the exclusion: "src_v2/ is DYMO's unfinished 'version 2' tree. It uses std::auto_ptr (removed from the standard library in C++17) and deprecated Boost posix_time APIs, so it does not compile on GCC >= 11 with default flags. The production filter binaries are built from src/; src_v2/ was never wired up to install anywhere."
- `dymo-cups-drivers/configure.ac:126-128` — `AC_CONFIG_FILES` does not list any `src_v2/*/Makefile`; only `src/…` Makefiles are generated. The matching comment at :126 says "src_v2/* removed from AC_CONFIG_FILES — does not compile on modern GCC."
- `dymo-cups-drivers/debian/rules` does not reference `src_v2`.
- The `printer-driver-dymo_1.4.0-12build3+loadfix1_amd64.deb` ships binaries built from `src/` only.

**Conclusion:** `src_v2/` is dead code. Not reviewed.

## 1.2 Filter binaries in scope

The 5 LW 450-family PPDs route as follows:

| PPD                | cupsFilter target    | Filter source directory        |
|--------------------|----------------------|--------------------------------|
| `lw450.ppd`        | `raster2dymolw`      | `src/lw/`                      |
| `lw450t.ppd`       | `raster2dymolw`      | `src/lw/`                      |
| `lw450tt.ppd`      | `raster2dymolw`      | `src/lw/`                      |
| `lw450dl.ppd`      | `raster2dymolw`      | `src/lw/`                      |
| `lw450dt.ppd`      | `raster2dymolm`      | `src/lm/` (LabelManager tape)  |

The Duo Tape path (`lw450dt.ppd` → `raster2dymolm`) implements Appendix B ("Printing to the LabelWriter Duo Tape Printer") of the spec. The Duo Label path (`lw450dl.ppd`) uses the same `raster2dymolw` filter as the other 450-family labels.

---

# 2. Command-set conformance (core LabelWriter commands, spec pages 15-19)

Legend for "Conforms" column: `✓` = matches spec byte-for-byte; `~` = partial / caveated; `✗` = spec violation or missing.

| Spec command      | Hex         | Semantics                                | Driver implementation                                       | Conforms | Severity / Note |
|-------------------|-------------|------------------------------------------|-------------------------------------------------------------|----------|-----------------|
| `<esc> B n`       | `1B 42 nn`  | Set Dot Tab, 0≤n≤83                       | `src/lw/LabelWriterDriver.cpp:430-436` `{ESC,'B',n}`        | ✓        | — see §3.1 re: bounds |
| `<esc> D n`       | `1B 44 nn`  | Set Bytes/Line, 1≤n≤84                    | `src/lw/LabelWriterDriver.cpp:447-453` `{ESC,'D',n}`        | ✓        | — see §3.2 re: 4XL override |
| `<esc> L nl n2`   | `1B 4C hi lo` | Set Label Length, MSB then LSB           | `src/lw/LabelWriterDriver.cpp:481-488`                      | ✓        | MSB=buf[2], LSB=buf[3] — correct byte order |
| `<esc> E`         | `1B 45`     | Form Feed                                | `src/lw/LabelWriterDriver.cpp:439-444` `{ESC,'E'}`          | ✓        | |
| `<esc> G`         | `1B 47`     | Short Form Feed                          | `src/lw/LabelWriterDriver.cpp:603-616` `{ESC,'G'}`          | ✓        | |
| `<esc> q n`       | `1B 71 3x`  | Select Roll (TwinTurbo only), n ∈ {'0','1','2'} | `src/lw/LabelWriterDriver.cpp:650-663`               | ✓        | '0'=auto, '1'=left, '2'=right — matches spec p.16 |
| `<esc> A`         | `1B 41`     | Get Status (returns 1 byte)              | `src/lw/LabelWriterDriver.cpp:551-557` (in `GetRequestStatusCommand`) | ✓        | — status-byte interpretation reviewed separately in §4 |
| `<esc> @`         | `1B 40`     | Reset Printer                            | NOT EMITTED. Driver substitutes a 156-byte ESC-sync burst at `src/lw/LabelWriterDriver.cpp:545-549` (`buffer_t(156, ESC)`). | ~ | **LOW**. Sync-reset recovers from mid-command state (spec p.9 requires ≥85 ESCs; 156 exceeds), but it does NOT restore firmware defaults — only `<esc> @` or `<esc> *` do. The driver explicitly re-sets every parameter at `StartDoc()` / `StartPage()`, so reliance on defaults never occurs. Substitution is safe in practice. |
| `<esc> *`         | `1B 2A`     | Restore Default Settings                 | NOT EMITTED                                                 | ~        | **INFO**. Not needed; driver does not rely on factory defaults. |
| `<esc> f 1 n`     | `1B 66 01 nn` | Skip n Lines (0-255)                    | `src/lw/LabelWriterDriver.cpp:456-478`                      | ✓        | Correctly emits the literal `0x01` separator byte (buf[2]) mandated by spec. Also correctly chunks runs > 255 lines into multiple commands (`MAX_LINES = 255`). |
| `<esc> V`         | `1B 56`     | Return Revision                          | NOT EMITTED                                                 | ~        | **INFO**. Not needed by a print filter; used by config tools only. |
| `<syn> nl..nx`    | `16 …`      | Uncompressed raster line                 | `src/lw/LabelWriterDriver.cpp:84-101, const SYN=0x16 at :32` | ✓       | |
| `<etb> nl..nx`    | `17 …`      | Compressed raster line                   | `src/lw/LabelWriterDriver.cpp:103-118, const ETB=0x17 at :33` | ✓      | — compression format verified in §5 |
| `<esc> h`         | `1B 68`     | Text Speed Mode (300×300)                | `src/lw/LabelWriterDriver.cpp:530-543` default `buf[1]='h'` | ✓        | |
| `<esc> i`         | `1B 69`     | Barcode & Graphics Mode (300×600)        | `src/lw/LabelWriterDriver.cpp:538` `pqBarcodeAndGraphics → buf[1]='i'` | ✓ | Note that `buf[]` is declared `{ESC,'h'}` and overwritten — final emitted bytes are correct `1B 69`. |
| `<esc> c`         | `1B 63`     | Density Light (75%)                      | `src/lw/LabelWriterDriver.cpp:520` `pdLow → 'c'`            | ✓        | |
| `<esc> d`         | `1B 64`     | Density Medium (87.5%)                   | `src/lw/LabelWriterDriver.cpp:521` `pdMedium → 'd'`         | ✓        | |
| `<esc> e`         | `1B 65`     | Density Normal (100%)                    | `src/lw/LabelWriterDriver.cpp:522` `pdNormal → 'e'`         | ✓        | |
| `<esc> g`         | `1B 67`     | Density Dark (112.5%)                    | `src/lw/LabelWriterDriver.cpp:523` `pdHigh → 'g'`           | ✓        | |

## 2.1 Non-spec commands the driver emits

The driver emits commands that are **not documented** anywhere in the LW 450 Series Technical Reference.

| Emitted bytes      | Code site                                      | Analysis | Severity |
|--------------------|------------------------------------------------|----------|----------|
| `1B 51 hi lo` (`<esc> Q`, "Line Tab") | `src/lw/LabelWriterDriver.cpp:420-427` via `SendLineTab(0)` at `StartDoc()` :51 | Not in the LW 450 command table (spec pp. 15-19). Comment at `LabelWriterDriver.cpp:313-319` identifies this as required for the **LW 3xx series**. On LW 450 hardware, the printer receives 4 stray bytes (`1B 51 00 00`) at the start of every document. Spec p.9 says "Any other characters are ignored" when the printer is expecting ESC commands — so this is effectively a no-op on the 450. | **LOW** (4 wasted bytes per job; no functional impact) |
| `1B 79` (`<esc> y`) / `1B 7A` (`<esc> z`) | `src/lw/LabelWriterDriver.cpp:490-511` via `SendResolution()` | Used only for SE450 (`res204` / `res136`). For the LW 450 family, `SendResolution` is never reached because `CDriverInitializerLabelWriter::ProcessPPDOptions` (at `CupsFilterLabelWriter.cpp:28-52`) only maps `"203dpi"` / `"203x138dpi"` PPD values, neither of which appears in the 450 PPDs — confirmed by explicit comment at `CupsFilterLabelWriter.cpp:31-44`. Resolution stays `resUnknown`, and `SendResolution` returns at line 493. | **INFO** (dead path for LW 450; no bytes emitted) |

---

# 3. Parameter-bounds conformance

## 3.1 Dot Tab bounds (spec p.15: `0 ≤ n ≤ 83`)

- **Code:** `src/lw/LabelWriterDriver.cpp:317` passes `LeaderBlanks` (leading zero-byte count) straight into `SendDotTab` with no explicit clamp.
- **Analysis:** `LeaderBlanks` is derived from a line that has already been truncated to `MaxPrintWidth_` bytes (default 84; see `LabelWriterDriver.cpp:286-289`). If *all* 84 bytes are zero, the line is classified as empty at `:298-302` and `SendDotTab` is not called at all. Therefore in practice `LeaderBlanks ∈ [0, 83]` when `ESC B n` is emitted, which matches the spec.
- **Severity:** LOW. Works by construction, but the invariant is implicit; a future edit to the empty-line detection could silently break it. No defect.

## 3.2 Bytes-per-Line bounds (spec p.15: `1 ≤ n ≤ 84`)

- **Code default:** `MaxPrintWidth_ = 84` at `src/lw/LabelWriterDriver.cpp:38`. ✓
- **Override for LabelWriter 4XL:** `src/lw/CupsFilterLabelWriter.cpp:85-86` calls `SetMaxPrintWidth(156)`. **This exceeds the 84-byte cap the LW 450 Series spec defines.** 
- **Assessment:** Out of the LW 450 Series spec's scope — the 4XL is not covered by the spec under review. The 4XL has a wider print head (hence a different `bytes-per-line` limit per its own — unlisted here — technical reference). Flagging for completeness only.
- **Severity:** INFO (out of spec document's scope).
- **Override for LW 300/310/315 / SE450:** `CupsFilterLabelWriter.cpp:83,89` sets 58 / 56. Also out of scope (not LW 450 family).

## 3.3 Label Length default (spec p.9: default = 3058 = 10.2″)

- **Code:** `PageHeight_(0x0800)` at `src/lw/LabelWriterDriver.cpp:37` — this is `2048` (= 6.83″), **not** `3058`.
- **Assessment:** The driver ALWAYS sends `ESC L` explicitly in `StartPage()` (`LabelWriterDriver.cpp:63-70`) using `PageHeight_` computed from the CUPS page header at `CupsFilterLabelWriter.cpp:105` (`PageSize[1] * HWResolution[1] / 72`). The `0x0800` default is only used if `StartPage` runs without a page header — an impossible path in the CUPS raster filter lifecycle. Firmware default (`3058`) is similarly unreached.
- **Severity:** LOW. The `0x0800` constant is misleading but unreachable.

## 3.4 Continuous-feed sentinel (spec p.9: "any negative 2-byte integer 0x8000-0xFFFF")

- **Code:** `LabelWriterDriver.cpp:68` — `case ptContinuous: SendLabelLength(0xffff); break;`
- **Analysis:** `0xFFFF` is within the spec-mandated `0x8000-0xFFFF` range for continuous mode. ✓
- **Severity:** — (conforms).

## 3.5 Reset-sync minimum (spec p.9: "at least 85 continuous `<esc>` characters")

- **Code:** `LabelWriterDriver.cpp:545-549` returns `buffer_t(156, ESC)`.
- **Analysis:** 156 ≥ 85. ✓
- **Severity:** — (conforms).

---

# 4. Status-byte interpretation (spec p.10 / p.17)

Spec definition of the `<esc> A` status byte (pp. 10 and 17):

| Bit | Mask   | Meaning per spec                                 |
|-----|--------|--------------------------------------------------|
| 0   | `0x01` | Ready (always 1)                                 |
| 1   | `0x02` | Top-of-Form                                      |
| 2   | `0x04` | Not used / reserved                              |
| 3   | `0x08` | Not used / reserved                              |
| 4   | `0x10` | Not used / reserved                              |
| 5   | `0x20` | Out of Paper                                     |
| 6   | `0x40` | Paper Jam                                        |
| 7   | `0x80` | Error (also set on out-of-paper)                 |

Driver's named bitmask at `src/lw/LabelWriterLanguageMonitor.h:38-45`:

```
TOF_BIT             = 0x02
ROLL_CHANGED_BIT    = 0x08
PAPER_OUT_BIT       = 0x20
PAPER_FEED_BIT      = 0x40
ERROR_BIT           = 0x80
```

## 4.1 `ROLL_CHANGED_BIT = 0x08` — spec says bit 3 is "Not used"

- **Finding:** The LW 450 Series spec (reviewed revision 10/09) explicitly documents bit 3 as "reserved" (p.10) and "Not used" (p.17 table). The driver nevertheless treats `0x08` as a semantically meaningful "Roll Changed" flag and branches on it in `LabelWriterLanguageMonitor.cpp:147, :171, :174, :326`.
- **Analysis:** This is almost certainly an **undocumented Twin Turbo extension**. The `ROLL_CHANGED_BIT` codepath is only exercised when a roll has been selected (the Twin Turbo variant), and the behavior — suppress reverse-feed on reprint — matches what you would want when the printer autonomously swapped rolls mid-job. The LW 450 spec was published 10/09 and may predate the firmware behavior, or DYMO may have chosen not to document it publicly. The code's use of this bit is not unsafe — on a single-roll 450 / 450T / Duo, the bit is guaranteed to be `0` per spec, so all branches guarded by `ROLL_CHANGED_BIT` are dead on those models.
- **Severity:** LOW for LW 450 / 450T / 450 Duo (bit is never set by spec-compliant firmware). INFO for 450 Twin Turbo (relies on undocumented firmware behavior).

## 4.2 `PAPER_FEED_BIT = 0x40` — spec says bit 6 is "Paper Jam"

- **Finding:** The constant is named `PAPER_FEED_BIT` but the spec defines bit 6 as **Paper Jam**. Usage at `LabelWriterLanguageMonitor.cpp:312-313`:

  ```cpp
  if ((Status & PAPER_OUT_BIT) || (Status & PAPER_FEED_BIT))
      JobStatus = IPrintEnvironment::jsPaperOut;
  ```

  Both "out of paper" AND "paper jam" are reported to CUPS as `jsPaperOut`. There is no distinct `jsPaperJam` state being set.
- **Severity:** MEDIUM (naming and UX). A real paper jam will surface to the user as "out of paper", potentially leading them to load more labels into a jammed mechanism. The underlying spec-level status is read correctly; it's the translation to the CUPS-facing state that is wrong.
- **Scope:** User-visible in CUPS error reporting (`com.dymo.out-of-paper-error` vs. a missing `com.dymo.paper-jam-error`). The LW 450 PPDs declare `com.dymo.slot-status-error/Label path is blocked` (e.g. `lw450.ppd:51`) but the driver never routes there.

## 4.3 "Ready" bit (bit 0) and TOF interaction

- **Spec p.10:** "Bit 0 (Ready Bit): This bit is always returned as a 1."
- **Spec p.17:** Note: "printer ready is returned as 03h (Ready and Top of form)."
- **Code:** Driver never examines bit 0 directly; it keys off `TOF_BIT (0x02)` at `LabelWriterLanguageMonitor.cpp:147, :171, :302`. Because bit 0 is constant, this is equivalent.
- **Severity:** — (conforms).

## 4.4 Continuous-media TOF synthesis

- **Code:** `LabelWriterLanguageMonitor.cpp:235-236`:

  ```cpp
  if (PaperType_ == CLabelWriterDriver::ptContinuous)
      Status |= TOF_BIT;
  ```

- **Analysis:** On continuous media there are no top-of-form sense holes, so the printer never physically reaches TOF and the bit stays `0`. The driver synthesises it as `1` to let the polling loop exit. This is a reasonable workaround for a spec silence (spec p.10 describes TOF as "inter-label gap over the cutter bar", which is undefined for continuous media).
- **Severity:** — (defensible).

---

# 5. Compressed data format (Appendix A, spec p.21)

Spec encoding:
- Bit 7: pixel color. `0` = white, `1` = black.
- Bits 0-6: `(count - 1)`, valid range `0..127`.
- Examples: `0x00` = 1 white px, `0x80` = 1 black px, `0x0F` = 16 white px, `0xFF` = 128 black px.

Driver implementation: `src/lw/LabelWriterDriver.cpp:176-222` (`GetCompressedSequenceValue`, `CompressData`).

| Property                    | Spec                         | Code                                                             | Conforms |
|-----------------------------|------------------------------|------------------------------------------------------------------|----------|
| Color bit position          | Bit 7                        | `bitCount | 0x80` at `:200`                                      | ✓ |
| Color encoding              | 0=white, 1=black             | `if (startBitValue) … | 0x80` (startBitValue=1 → black)         | ✓ |
| Count encoding              | `count - 1` in bits 0-6      | `bitCount` starts at 0, increments once per *additional* matching bit; returned as-is. So N matching bits → `bitCount = N-1`. | ✓ |
| Max run length              | 128 (`count-1 ≤ 127`)        | Loop guard `bitCount < 0x7f` at `:185` caps `bitCount` at `0x7F` (= 127). First bit is counted before loop, so total run = 1 + 127 = 128. | ✓ |
| Bytes-per-line equality     | `Σ pixels == BytesPerLine*8` | `SendCompressedData(CompressedBuf, NotCompressedSize)` at `:326` passes the uncompressed pixel-byte count into `SendBytesPerLine` at `:110-113`. | ✓ |
| Fallback when compression expands | — (spec silent)        | `CompressData` aborts and clears `CompressedData` if compressed output would reach `DataSize - 1` (`:214-218`); `ProcessRasterLine` detects the empty buffer and falls back to `SendNotCompressedData` (`:325-328`). | ✓ (safe optimisation) |

Conclusion: Appendix A encoding is implemented correctly. No defects.

---

# 6. PPD conformance

## 6.1 Shared settings across lw450{,t,tt,dl,dt}.ppd

All 5 PPDs in scope declare:
- `*PSVersion: "(3010.000) 550"` / `*LanguageLevel: "3"`
- `*DefaultDymoPrintDensity: Normal` — maps to `ESC e` (spec's "standard duty cycle 100%"). ✓
- `*DefaultDymoPrintQuality: Text` — maps to `ESC h` (spec's 300×300 Text Speed Mode). ✓
- `*HWMargins: 0 0 0 0` — driver emits its own margins via `ESC B` (dot tab). Per-media `ImageableArea` values carry the physical margins. ✓ (consistent with raster-based design, spec p.7).
- `*cupsFilter: "application/vnd.cups-raster 0 raster2dymolw"` (except Duo Tape → `raster2dymolm`).

## 6.2 Resolution / Quality UI-option mismatch

The 4 label PPDs (`lw450.ppd`, `lw450t.ppd`, `lw450tt.ppd`, `lw450dl.ppd`) expose **two overlapping controls** that both claim to set the printer resolution:

```ppd
*Resolution 300dpi/300 DPI:        "<</HWResolution[300 300]>>setpagedevice"
*Resolution 300x600dpi/300x600 DPI: "<</HWResolution[300 600]>>setpagedevice"
…
*DymoPrintQuality Text/Text Only:          "<</HWResolution[300 300]>>setpagedevice"
*DymoPrintQuality Graphics/Barcodes and Graphics: "<</HWResolution[300 600]>>setpagedevice"
```

Only `DymoPrintQuality` is consumed by the driver to emit `ESC h` / `ESC i` (`src/lw/CupsFilterLabelWriter.cpp:53-60`). The `Resolution` PPD option has NO code path in the driver — an explicit comment at `src/lw/CupsFilterLabelWriter.cpp:31-44` acknowledges that the driver's `resolution_t` enum only knows `res136` / `res204` (SE450), not 300 or 300×600, so `SetResolution` is silently skipped for all LW 450 PPDs.

The PPD guards the overlap with `*UIConstraints` (e.g. `lw450.ppd:583-586`), forbidding the user from picking a mismatched pair:

```ppd
*UIConstraints: *DymoPrintQuality Text       *Resolution       300x600dpi
*UIConstraints: *DymoPrintQuality Graphics   *Resolution       300dpi
```

- **Severity:** LOW. The duplicate option is confusing PPD design and means the PPD-level `Resolution` dropdown is cosmetic, but UIConstraints prevent user-visible misconfiguration and the effective resolution is set correctly via `DymoPrintQuality`. The spec (p.11 "Barcode and Graphics Print Mode") is honored.

## 6.3 InputSlot (Twin Turbo only) — spec p.16 `<esc> q n`

`lw450tt.ppd` declares:
```ppd
*InputSlot Auto/Auto:           "<</cupsMediaPosition 0>>setpagedevice"
*InputSlot Left/Left Roll:      "<</cupsMediaPosition 1>>setpagedevice"
*InputSlot Right/Right Roll:    "<</cupsMediaPosition 2>>setpagedevice"
```

Handled by `CDriverInitializerLabelWriterTwinTurbo::ProcessPPDOptions` at `src/lw/CupsFilterLabelWriter.cpp:113-129`. Values map to `rtAuto`/`rtLeft`/`rtRight`, then to ASCII `'0'`/`'1'`/`'2'` in the emitted `ESC q n` command at `src/lw/LabelWriterDriver.cpp:653-662`. ✓

## 6.4 Page-size registry

All 4 label-side PPDs declare `*DefaultPageSize: w167h288` (30256 "Shipping", 4″×6″). The shipping-label format matches the "reference label" used in the spec's maximum-current-draw measurement (p.12: "3.3 Amps based on printing a shipping label (30256) with a maximum-size filled rectangle"). ✓

---

# 7. Protocol-sequence conformance

## 7.1 StartDoc ordering (spec p.11: "Optimization of Throughput")

`CLabelWriterDriver::StartDoc()` at `src/lw/LabelWriterDriver.cpp:46-55` emits, in order:

1. 156-byte ESC-sync reset (satisfies spec p.9 minimum of 85).
2. `SendResolution` — no-op for LW 450 (see §2.1).
3. `SendLineTab(0)` — **not in spec** (see §2.1).
4. `SendDotTab(0)` — `ESC B 00`.
5. `SendPrintQuality` — `ESC h` or `ESC i`.
6. `SendPrintDensity` — `ESC c`/`d`/`e`/`g`.

Spec p.11 says command sequences "should be sent only when a change is desired"; driver sends at least `ESC B 0` and `ESC h/i`, `ESC c/d/e/g` on every job even if they match firmware defaults. Harmless but suboptimal.

- **Severity:** INFO.

## 7.2 EndPage / EndDoc on the 450-class driver (spec p.10)

Spec guidance: use `ESC G` between labels, `ESC E` after the last label, to "eliminate the reverse feed" reverse-feed optimization.

Code:
- `CLabelWriterDriver400::EndPage()` → `SendShortFormFeed()` = `ESC G` (`LabelWriterDriver.cpp:596-600, :611-616`). ✓
- `CLabelWriterDriver400::EndDoc()` → `SendFormFeed()` = `ESC E` (`LabelWriterDriver.cpp:590-594`). ✓

This is the path used by LW 450 family PPDs — see the `modelname` dispatch at `src/lw/raster2dymolw.cpp:121-139`, which instantiates `CLabelWriterDriver400` for `"DYMO LabelWriter 450"`, `"… 450 Turbo"`, and `"… 450 DUO Label"`.

- **Severity:** — (spec-compliant and optimized).

## 7.3 Dot-tab emission per raster line (spec p.11: send only on change)

Code at `LabelWriterDriver.cpp:311-319`:

```cpp
    // NOTE: an ESC B needs to be send for each raster line. Otherwise the LW 3xx series output
    // will be distorted.
    //if (LastDotTab_ != LeaderBlanks)
    //{
      SendDotTab(LeaderBlanks);
      LastDotTab_ = LeaderBlanks;
    //}
```

Every non-empty raster line is preceded by `ESC B n`, violating the spec's "only when a change is desired" throughput guidance (p.11). Retained as a workaround for LW 3xx firmware quirks per the inline comment. For LW 450 hardware this produces ~3 extra bytes per raster line (worst case ~10 KiB per shipping label) — well under USB 2.0 FS bandwidth.

- **Severity:** LOW. Wastes some bytes but does not violate the command semantics.

## 7.4 Reset / power-on vs. software-reset

- Spec p.8 states: "All printer parameters are set to specific default values by a power-on reset or software reset command from the host computer."
- Driver emits neither `ESC @` nor `ESC *`; it relies on the 156-ESC sync-reset (§2 row 8). As documented in §2 and §3.5, this restores **sync** but NOT parameter defaults. The driver works around this by re-sending every parameter in `StartDoc`/`StartPage`, so the end-state is correct.
- **Severity:** LOW (deviation is safe because subsequent commands overwrite everything the default would have set).

---

# 8. Back-channel / language-monitor conformance (spec pp. 10-11 status byte)

## 8.1 Back-channel is intentionally disabled on this build

At `src/lw/raster2dymolw.cpp:78-82`:

```cpp
static bool
IsBackchannelSupported()
{
  return false;
}
```

The preceding comment block (`raster2dymolw.cpp:45-77`) records that upstream DYMO returned `true` unconditionally and that doing so causes the filter to hang ("zombie 'now printing'") under modern CUPS + USB on LW 450-series hardware. The local fix forces selection of `CDummyLanguageMonitor` instead of `CLabelWriterLanguageMonitor`.

**Consequence for spec conformance:** `ESC A` (Get Status) is never emitted during a print job on this build. The `CheckStatusAndReprint`, `ReadStatus`, `PollUntilPaperIn`, and `ReprintLabel` helpers in `LabelWriterLanguageMonitor.cpp:107-339` are all dormant. The driver still complies with spec pp. 8-9 for all *outbound* command sequences; it simply does not use the spec's optional status inbound path.

- **Severity:** INFO. The spec does not *require* the host to poll `ESC A`; it only documents the bits returned if it does. The CUPS state reporting (`STATE: com.dymo.out-of-paper-error`, etc.) and the auto-reprint-on-roll-change feature are disabled as a consequence.

## 8.2 Residual code paths if back-channel were re-enabled

If `IsBackchannelSupported()` returned `true`, the following behaviors would apply (documented for completeness — not exercised by the shipping binary):

- `LabelWriterLanguageMonitor.cpp:226-253` reads the USB back-channel and **keeps only the *last* byte** of whatever `cupsBackChannelRead` delivers. The in-source comment (`:226-232`) explains this is a deliberate fix over upstream DYMO's "keep the first byte", as CUPS may buffer multiple `ESC A` replies. This is consistent with spec p.10 (one byte per `ESC A` request) — but there is no cross-checking that the byte count returned equals the number of requests issued.
- `PollUntilPaperIn` at `LabelWriterLanguageMonitor.cpp:255-305` is bounded by a `ReadStatusTimeout_` (default 10 s) deadline; the upstream DYMO code had no wall-clock bound. Non-conformance with spec is impossible here (spec p.11 is silent on timeouts), but the change is relevant if this path is ever re-enabled.

---

# 9. USB interface conformance (spec p.14)

- Spec: VID `0x0922`, PID `0x0020` (LW 450), `0x0021` (LW 450 Turbo), `0x0022` (LW 450 Twin Turbo), `0x0023` (LW Duo).
- This driver: **No VID/PID code presence.** VID/PID matching is the CUPS backend's responsibility (not `raster2dymolw`), typically via `cups-browsed` / `udev` / `/etc/cups/printers.conf` and the `usb://` backend URI. The driver operates on whatever device URI CUPS hands it.
- **Severity:** — (out of scope for the filter binary).

PPD-side DeviceID / autoconfig strings are not declared in the LW 450-family PPDs reviewed (no `*1284DeviceID` entries). That's permissible; the PPDs install via standard product-name matching.

---

# 10. Summary of findings

## 10.1 Spec-violating or potentially spec-violating items

| ID  | Severity | Summary                                                                                          | File:line(s) |
|-----|----------|--------------------------------------------------------------------------------------------------|--------------|
| F-1 | MEDIUM   | `PAPER_FEED_BIT (0x40)` is named and treated as "paper feed / out of paper" but spec defines bit 6 as "Paper Jam". User sees jams reported as "out of paper". | `src/lw/LabelWriterLanguageMonitor.h:43`, usage at `.cpp:312-313` |
| F-2 | LOW      | `ROLL_CHANGED_BIT (0x08)` is a Twin-Turbo-specific extension; LW 450 spec documents bit 3 as "Not used". Harmless on non-TT models (bit is guaranteed `0`); undocumented firmware assumption on TT. | `src/lw/LabelWriterLanguageMonitor.h:41`, usage at `.cpp:147, 171, 174, 326` |
| F-3 | LOW      | `ESC Q 00 00` (LW 3xx Line Tab) is emitted on every `StartDoc` but is not in the LW 450 command set. Ignored by LW 450 firmware per spec p.9. | `src/lw/LabelWriterDriver.cpp:51, 420-427` |
| F-4 | LOW      | Driver substitutes a 156-ESC sync-reset for `ESC @`. Sync is satisfied; parameter-default reset is NOT, but every parameter is explicitly re-set afterward so state is correct. | `src/lw/LabelWriterDriver.cpp:545-549` |
| F-5 | LOW      | `ESC B n` is emitted on every non-empty raster line rather than only on change — violates spec p.11 throughput advice. Comment on code (`:313`) explains it's a deliberate workaround for LW 3xx distortion. | `src/lw/LabelWriterDriver.cpp:311-319` |
| F-6 | LOW      | PPD `Resolution` UI option is cosmetic on LW 450 family — driver only honors `DymoPrintQuality` for choosing `ESC h` / `ESC i`. `*UIConstraints` prevent user-visible conflict. | `src/lw/CupsFilterLabelWriter.cpp:28-52`; e.g. `ppd/lw450.ppd:414-419, 583-586` |
| F-7 | LOW      | `PageHeight_` default `0x0800` at `LabelWriterDriver.cpp:37` does not match spec default `3058`. Unreachable in practice because `StartPage` always re-computes from CUPS page header. | `src/lw/LabelWriterDriver.cpp:37` |
| F-8 | INFO     | Duty-cycle `ESC c/d/e/g`, `ESC h/i`, `ESC B 0`, `ESC D nn` are emitted even when equal to firmware default — minor throughput violation of spec p.11. | `src/lw/LabelWriterDriver.cpp:46-55, 311-319` |
| F-9 | INFO     | Back-channel (`ESC A`) polling is disabled by `IsBackchannelSupported() → false`. CUPS state reporting (`com.dymo.out-of-paper-error` etc., declared in PPDs) and auto-reprint-on-roll-change do not function. Spec does not require polling. | `src/lw/raster2dymolw.cpp:78-82` |
| F-10| INFO     | `ESC @`, `ESC *`, `ESC V` spec commands are not implemented. Not needed for printing. | — |
| F-11| INFO     | `MaxPrintWidth_ = 156` for LW 4XL exceeds LW 450 Series spec's `n ≤ 84`, but 4XL is explicitly outside this spec document's scope. | `src/lw/CupsFilterLabelWriter.cpp:85-86` |

## 10.2 Spec-conformant core

All outbound commands that matter for the LW 450 family's end-to-end print lifecycle are byte-for-byte correct:

- `ESC B`, `ESC D`, `ESC L`, `ESC E`, `ESC G`, `ESC f 1 n`, `ESC q n`, `ESC A`, `ESC h/i`, `ESC c/d/e/g`, `SYN`/`ETB` data prefixes.
- Label Length MSB/LSB byte order (spec p.16).
- Continuous-feed sentinel `0xFFFF` (in spec's `0x8000-0xFFFF` range).
- Skip-Lines chunking at 255 (spec max per-instance of `ESC f 1 n`).
- Appendix A RLE compression (color bit, count-minus-one encoding, 128-pixel max run, `Σ pixels == bytes-per-line × 8`).
- 85-char ESC sync-recovery minimum (spec p.9).

## 10.3 Recommendation matrix for downstream agents

An agent acting on this review should:

1. **Fix first:** F-1 (rename `PAPER_FEED_BIT` → `PAPER_JAM_BIT`, route to a new `jsPaperJam` state, surface `com.dymo.slot-status-error` already declared in the PPDs). Medium-severity UX defect with clear spec reference.
2. **Investigate:** F-2 (confirm whether current Twin Turbo firmware actually signals roll-change via bit 3; if not, the code is dead). Not urgent.
3. **Ignore unless modernizing:** F-3 through F-11. Either defensible as a conscious workaround, already compensated for elsewhere in the code, or outside the spec document's scope.
4. **Do not touch:** `src_v2/`. Dead tree, confirmed unbuilt (§1.1).

---

# 11. Files consulted

Spec:
- `docs/LW 450 Series Technical Reference.pdf` (pages 1-27, Rev. 10/09)

Driver (production tree `src/`):
- `src/lw/LabelWriterDriver.cpp`
- `src/lw/LabelWriterDriver.h`
- `src/lw/LabelWriterLanguageMonitor.cpp`
- `src/lw/LabelWriterLanguageMonitor.h`
- `src/lw/CupsFilterLabelWriter.cpp`
- `src/lw/CupsFilterLabelWriter.h`
- `src/lw/raster2dymolw.cpp`
- `src/lw/Makefile.am`

PPDs:
- `ppd/lw450.ppd`
- `ppd/lw450t.ppd`
- `ppd/lw450tt.ppd`
- `ppd/lw450dl.ppd`
- `ppd/lw450dt.ppd`

Build-system (for src_v2 validation):
- `Makefile.am`
- `configure.ac`
- `debian/rules`

Dead tree (NOT reviewed, confirmed unbuilt):
- `src_v2/**`
