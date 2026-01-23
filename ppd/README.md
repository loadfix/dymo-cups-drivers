
# PostScript Printer Description files (.ppd)

This directory contains the source files for PPD generation using PPDC.

## Shared `.def` files

- `printer.def` - Common printer attributes
- `colorspace.def` - Color device settings
- `fonts.def` - Font definitions
- `errors.def` - IPP error messages and translations
- `halftoning.def` - Halftoning options
- `options.def` - Print density and quality options
- `mediatype.def` - Media type options
- `labelalignment.def` - Label alignment options
- `continuous.def` - Continuous paper options
- `continuous_lw.def` - LabelWriter continuous paper options
- `chainmarks.def` - Chain marks options
- `tapecolor.def` - Tape color options
- `cutoptions.def` - Cut options

## PPDC Format Reference

Key differences from PPD syntax:

| PPD Syntax | PPDC Format |
|------------|-------------|
| `*Attribute: value` | `Attribute Attribute "" value` |
| `*ModelName: "name"` | `ModelName "name"` |
| `*PCFileName: "file.ppd"` | `PCFileName "file.ppd"` |
| `*OpenUI *Option/Name: PickOne` | `Option Option/Name PickOne AnySetup 20` |
| `*Option Choice/Text: "code"` | `Choice Choice/Text "code"` |
| `*cupsFilter: "mime cost program"` | `Filter mime cost program` |
| `*de.Translation Option/Text: ""` | `LocAttribute "de" Translation "Option" "Text"` |

## Links

* About PPD: https://www.cups.org/doc/postscript-driver.html
* Compiling PPD files: https://www.cups.org/doc/ppd-compiler.html

