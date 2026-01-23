# DYMO Printer Command Protocol Documentation

This document describes the ESC/P command sequences used to communicate with DYMO LabelWriter and LabelManager printers.

## Table of Contents

1. [Basic Concepts](#basic-concepts)
2. [Control Characters](#control-characters)
3. [LabelWriter Commands](#labelwriter-commands)
4. [LabelManager Commands](#labelmanager-commands)
5. [TwinTurbo-Specific Commands](#twin turbo-specific-commands)
6. [Command Format Details](#command-format-details)
7. [Command Sequences](#command-sequences)
8. [Command Summary](#command-summary-table)

---

## Basic Concepts

### ESC Character

All printer commands begin with the **ESC** (Escape) character:

```
ESC = 0x1B (27 decimal, ASCII escape character)
```


### Command Format

Most commands follow the pattern:
```
ESC + [Command Character] + [Optional Parameters]
```

Parameters are typically:
- Single bytes (0x00 - 0xFF)
- Multi-byte values (little-endian or big-endian, depending on command)
- ASCII characters ('0', '1', '2', etc.)

---

## Control Characters

The protocol uses standard ASCII control characters for data transmission:

| Character | Hex Value | Decimal | Purpose |
|-----------|-----------|---------|---------|
| **ESC** | 0x1B | 27 | Escape character - prefix for all commands |
| **SYN** | 0x16 | 22 | Synchronous idle - marks start of uncompressed raster data |
| **ETB** | 0x17 | 23 | End of transmission block - marks start of compressed raster data |

### Usage

- **SYN (0x16)**: Precedes uncompressed raster line data
- **ETB (0x17)**: Precedes compressed raster line data

These are used in the legacy driver implementations for marking different data transmission modes.

---

## LabelWriter Commands

### Reset Command

**Command**: `156 × ESC` (156 consecutive ESC characters)

**Hex**: `0x1B` repeated 156 times

**Purpose**: Resets the printer to a known initial state. Must be sent at the beginning of each print job.

**Note**: The value 156 appears to be a protocol requirement for clearing the printer's command buffer and ensuring a clean state.

---

### Status Request

**Command**: `ESC 'A'`

**Hex**: `0x1B 0x41`

**Purpose**: Requests the current status from the printer.

---

### Start Print Job

**Command**: `ESC 's' [JobID (4 bytes, big-endian)]`

**Hex**: `0x1B 0x73 [JobID bytes]`

**Purpose**: Initiates a new print job with a unique job identifier.

**Parameters**:
- `JobID`: 32-bit unsigned integer (dword), sent as 4 bytes in big-endian format

**Note**: Job ID is typically generated randomly using `std::rand()` seeded with current time.

---

### End Print Job

**Command**: `ESC 'Q'`

**Hex**: `0x1B 0x51`

**Purpose**: Signals the end of a print job.

---

### Set Label Index (Page Number)

**Command**: `ESC 'n' [PageNumber (2 bytes, little-endian)]`

**Hex**: `0x1B 0x6E [PageNumber bytes]`

**Purpose**: Sets the current page/label number within a multi-page job.

**Parameters**:
- `PageNumber`: 16-bit unsigned integer, sent as 2 bytes in little-endian format

---

### Set Print Data Header

**Command**: `ESC 'D' [Flags] [Width (4 bytes)] [Height (4 bytes)]`

**Hex**: `0x1B 0x44 [0x01] [0x02] [Width bytes] [Height bytes]`

**Purpose**: Defines the raster data format and dimensions for the current page.

**Parameters**:
- Byte 2: `0x01` - Data format flags (monochrome)
- Byte 3: `0x02` - Alignment flags (bottom-aligned)
- Bytes 4-7: Width in pixels (32-bit, big-endian)
- Bytes 8-11: Height in pixels (32-bit, big-endian)

**Note**: Height is automatically byte-aligned (rounded up to nearest multiple of 8) before encoding.

---

### Form Feed

**Command**: `ESC 'E'`

**Hex**: `0x1B 0x45`

**Purpose**: Advances the paper to the next label position (full form feed).

---

### Short Form Feed

**Command**: `ESC 'G'`

**Hex**: `0x1B 0x47`

**Purpose**: Advances the paper a short distance (used for 400 series and later models).

**Note**: Used at the end of each page in 400+ series printers. The PBB (printer backend) feeds only the delta between form feed and short form feed, so a short form feed is required at page end.

---

### Set Print Density

**Command**: `ESC [Density Character]`

**Hex**: `0x1B [Density byte]`

**Purpose**: Sets the print density (darkness/intensity).

**Density Values**:
- `'c'` (0x63) - Low density
- `'d'` (0x64) - Medium density
- `'e'` (0x65) - Normal density (default)
- `'g'` (0x67) - High density

---

### Set Print Quality

**Command**: `ESC [Quality Character]`

**Hex**: `0x1B [Quality byte]`

**Purpose**: Sets the print quality mode.

**Quality Values**:
- `'h'` (0x68) - Text quality (faster, optimized for text)
- `'i'` (0x69) - Barcode and graphics quality (higher quality, slower)

---

### Set Print Speed

**Command**: `ESC 'T' [Speed Value]`

**Hex**: `0x1B 0x54 [Speed byte]`

**Purpose**: Sets the print speed (only supported on certain models).

**Speed Values**:
- `0x10` - Normal speed
- `0x20` - High speed

**Note**: Only available on printers that support high-speed printing.

---

### Set Print Media Type

**Command**: `ESC 'M' [Media Type] [Reserved bytes...]`

**Hex**: `0x1B 0x4D [Media byte] [0x00...]`

**Purpose**: Sets the media type for optimized printing.

**Media Values**:
- `0x00` - Default media
- `0x01` - Durable media

---

### Set Label Length

**Command**: `ESC 'L' [Length (2 bytes, little-endian)]`

**Hex**: `0x1B 0x4C [Length bytes]`

**Purpose**: Sets the label length in pixels.

**Parameters**:
- `Length`: 16-bit unsigned integer, sent as 2 bytes in little-endian format
- Special value: `0xFFFF` (65535) indicates continuous paper mode

**Usage**:
- Regular labels: Set to actual label height in pixels
- Continuous paper: Set to `0xFFFF`

---

## LabelManager Commands

LabelManager printers share many commands with LabelWriter, but have additional commands for tape cutting and alignment.

### Shared Commands

The following commands are identical to LabelWriter:
- **Start Print Job**: `ESC 's' [JobID]`
- **End Print Job**: `ESC 'Q'`
- **Set Label Index**: `ESC 'n' [PageNumber]`
- **Set Print Data Header**: `ESC 'D' [Flags] [Width] [Height]`
- **Form Feed**: `ESC 'E'`
- **Short Form Feed**: `ESC 'G'`
- **Status Request**: `ESC 'A'`

### Set Label Leader

**Command**: `ESC 'l' [Length (4 bytes, big-endian)]`

**Hex**: `0x1B 0x6C [Length bytes]`

**Purpose**: Sets the leader (blank space before the label content) in pixels.

**Parameters**:
- `Length`: 32-bit unsigned integer, sent as 4 bytes in big-endian format

**Typical Values**:
- Normal leader: 75 pixels
- Min leader: 55 pixels
- Aligned leader: 43 pixels

---

### Set Label Trailer

**Command**: `ESC 't' [Length (4 bytes, big-endian)]`

**Hex**: `0x1B 0x74 [Length bytes]`

**Purpose**: Sets the trailer (blank space after the label content) in pixels.

**Parameters**:
- `Length`: 32-bit unsigned integer, sent as 4 bytes in big-endian format

---

### Cut Command

**Command**: `ESC 'p' 0x30`

**Hex**: `0x1B 0x70 0x30`

**Purpose**: Cuts the tape at the current position.

---

### Cutter Mark (Chain Mark)

**Command**: `ESC 'p' 0x31`

**Hex**: `0x1B 0x70 0x31`

**Purpose**: Prints a chain mark (dotted line) for manual cutting instead of auto-cutting.

**Note**: Chain marks are used when auto-cut is disabled or when printing multiple labels in continuous mode.

---

### Legacy LabelManager Commands

The following commands are present in the legacy driver but may not be used in the current implementation:

#### Set Dot Tab

**Command**: `ESC 'B' [Value]`

**Hex**: `0x1B 0x42 [Value]`

**Purpose**: Sets horizontal dot tab position (used in legacy driver for chain marks).

#### Set Bytes Per Line

**Command**: `ESC 'D' [Value]`

**Hex**: `0x1B 0x44 [Value]`

**Purpose**: Sets the number of bytes per raster line (legacy command, different from print data header).

#### Set Tape Color

**Command**: `ESC 'C' [Color Value]`

**Hex**: `0x1B 0x43 [Color]`

**Purpose**: Sets the tape color mode (legacy command).

---

## TwinTurbo-Specific Commands

### Roll Selection

**Command**: `ESC 'q' [Roll Selection]`

**Hex**: `0x1B 0x71 [Selection]`

**Purpose**: Selects which roll to use on TwinTurbo printers (models with dual rolls).

**Selection Values**:
- `'0'` (0x30) - Auto (printer selects)
- `'1'` (0x31) - Left roll
- `'2'` (0x32) - Right roll

**Usage**: Must be sent at the start of a document to select the roll before printing begins.

---

## Command Format Details

### Byte Order (Endianness)

Different commands use different byte ordering:

| Command | Parameter Type | Byte Order |
|---------|----------------|------------|
| Start Print Job (`ESC 's'`) | JobID (32-bit) | Big-endian (MSB first) |
| Label Index (`ESC 'n'`) | PageNumber (16-bit) | Little-endian (LSB first) |
| Label Length (`ESC 'L'`) | Length (16-bit) | Little-endian (LSB first) |
| Print Data Header (`ESC 'D'`) | Width/Height (32-bit) | Big-endian (MSB first) |
| Label Leader (`ESC 'l'`) | Length (32-bit) | Big-endian (MSB first) |
| Label Trailer (`ESC 't'`) | Length (32-bit) | Big-endian (MSB first) |

### Encoding Examples

**Example 1: JobID = 0x12345678 (Big-endian)**
```
ESC 's' 0x12 0x34 0x56 0x78
```

**Example 2: PageNumber = 0x0100 (Little-endian)**
```
ESC 'n' 0x00 0x01
```

**Example 3: Width = 0x00000300 pixels (Big-endian)**
```
ESC 'D' ... 0x00 0x00 0x03 0x00 ...
```

---

## Command Sequences

### Typical LabelWriter Print Job Sequence

```
1. Reset:           156 × ESC
2. Start Job:       ESC 's' [JobID]
3. Set Density:     ESC [Density]
4. Set Quality:     ESC [Quality]
5. Set Speed:       ESC 'T' [Speed]  (if supported)
6. Set Media:       ESC 'M' [Media]
7. Set Label Length: ESC 'L' [Length]
8. Set Label Index: ESC 'n' [PageNumber]
9. Set Data Header: ESC 'D' [Flags] [Width] [Height]
10. Raster Data:    [Raster lines...]
11. Short Form Feed: ESC 'G'
12. [Repeat steps 7-11 for each page]
13. Form Feed:      ESC 'E'
14. End Job:        ESC 'Q'
```

### Typical LabelManager Print Job Sequence

```
1. Start Job:       ESC 's' [JobID]
2. Set Label Index: ESC 'n' [PageNumber]
3. Set Leader:      ESC 'l' [Length]
4. Set Trailer:     ESC 't' [Length]
5. Set Data Header: ESC 'D' [Flags] [Width] [Height]
6. Raster Data:     [Raster lines...]
7. [For page > 1:]
   - Form Feed:     ESC 'E'
   - Cut/Mark:      ESC 'p' [0x30 or 0x31]
8. [Repeat steps 2-7 for each page]
9. Form Feed:       ESC 'E'
10. Cut:            ESC 'p' 0x30  (if auto-cut enabled)
11. End Job:        ESC 'Q'
```

### TwinTurbo Print Job Sequence

```
1. Reset:           156 × ESC
2. Start Job:       ESC 's' [JobID]
3. Roll Select:     ESC 'q' [Roll]  ← TwinTurbo specific
4. [Continue with standard LabelWriter sequence...]
```

---

## Raster Data Format

### Uncompressed Raster Data

For uncompressed data, each raster line is preceded by a **SYN** (0x16) character:

```
SYN [Raster line bytes...]
```

The raster line contains bitmap data where:
- Each bit represents one pixel (1 = black, 0 = white)
- Bits are packed into bytes (MSB first)
- Line length must match the width specified in the print data header

### Compressed Raster Data

For compressed data, each raster line is preceded by an **ETB** (0x17) character:

```
ETB [Compressed raster line bytes...]
```

The compression format is proprietary to DYMO printers.

---

## Command Summary

### Command Summary Table

| Command | Hex | Purpose | Printer Type |
|---------|-----|---------|--------------|
| Reset | `0x1B` × 156 | Reset printer | LabelWriter |
| Status Request | `0x1B 0x41` | Get status | Both |
| Start Job | `0x1B 0x73 [ID]` | Begin print job | Both |
| End Job | `0x1B 0x51` | End print job | Both |
| Label Index | `0x1B 0x6E [N]` | Set page number | Both |
| Data Header | `0x1B 0x44 [...]` | Set raster format | Both |
| Form Feed | `0x1B 0x45` | Full paper advance | Both |
| Short Form Feed | `0x1B 0x47` | Short paper advance | Both |
| Label Length | `0x1B 0x4C [L]` | Set label height | LabelWriter |
| Print Density | `0x1B [c/d/e/g]` | Set darkness | LabelWriter |
| Print Quality | `0x1B [h/i]` | Set quality mode | LabelWriter |
| Print Speed | `0x1B 0x54 [S]` | Set speed | LabelWriter |
| Print Media | `0x1B 0x4D [M]` | Set media type | LabelWriter |
| Roll Select | `0x1B 0x71 [R]` | Select roll | TwinTurbo |
| Label Leader | `0x1B 0x6C [L]` | Set leader space | LabelManager |
| Label Trailer | `0x1B 0x74 [L]` | Set trailer space | LabelManager |
| Cut | `0x1B 0x70 0x30` | Cut tape | LabelManager |
| Cutter Mark | `0x1B 0x70 0x31` | Print chain mark | LabelManager |

---

## Notes

1. **Command Origins**: All command values represent the actual protocol used by DYMO printers.

2. **Byte Alignment**: Raster data heights are automatically byte-aligned (rounded up to nearest multiple of 8 pixels) before encoding in the print data header.

3. **Job ID Generation**: Job IDs are typically generated using `std::rand()` seeded with the current time to ensure uniqueness.

4. **Model Differences**: Not all commands are supported on all printer models. The driver checks for feature support before sending certain commands (e.g., high-speed printing).

5. **Protocol Evolution**: The current implementation uses a newer protocol format compared to the legacy driver. Some legacy commands (like `ESC 'B'` for dot tab) may not be used in the current implementation.

6. **Testing**: Command sequences are verified and show the expected command sequence for a complete print job.

