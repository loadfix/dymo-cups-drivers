# Translation Files for DYMO CUPS Drivers

This directory contains `gettext`-compatible `.po` files for internationalization of the DYMO CUPS printer drivers.

## Files

- `en.po`:  English (source language)
- `de.po`:  German
- `es.po`:  Spanish
- `es_co.po`:  Spanish (Colombia)
- `fr.po`:  French
- `fr_ca.po`:  French (Canada)
- `it.po`:  Italian
- `nl.po`:  Dutch
- `pt.po`:  Portuguese
- `pt_br.po`:  Portuguese (Brazil)

## Format

The `.po` files follow the standard GNU gettext format:

```
#: source_key
msgid "English text"
msgstr "Translated text"
```

## Coverage

The translation files include translations for:

- **Option names**: Print Density, Print Quality, Label Alignment, etc.
- **Choice values**: Light, Medium, Normal, Dark, etc.
- **Media types**: Label widths (06mm, 09mm, 12mm, etc.)
- **Tape colors**: Black on White, Black on Blue, etc.
- **Cut options**: Cut, Chain Marks, etc.
- **Page sizes**: Various label sizes (Address, Shipping, etc.)

## Usage with `ppdc`

These `.po` files can be used with the `ppdc` compiler using the `-c` option:

```bash
ppdc -c i18n/de.po -l de ppd/lw450.drv
```

Or specifying multiple languages:

```bash
ppdc -c i18n/de.po -c i18n/fr.po -l de,fr ppd/lw450.drv
```

## Notes

- Some entries may have empty `msgstr ""` if translations are not yet available
- The source keys (e.g., `DymoPrintDensity.Light`) are included as comments for reference
- All files use UTF-8 encoding
- The files are compatible with standard gettext tools, such as `msgfmt`, `msgmerge`, etc.

## Contributing

To update translations:

1. Edit the appropriate `.po` file
2. Fill in empty `msgstr ""` entries with translations
3. Use `msgfmt` to validate the file:
   ```bash
   msgfmt --check de.po
   ```
4. Commit your changes with git and create a pull-request

## Links

- [ppdc man page](https://man.freebsd.org/cgi/man.cgi?manpath=freebsd-ports&query=ppdc&sektion=1)
- [GNU gettext documentation](https://www.gnu.org/software/gettext/)

