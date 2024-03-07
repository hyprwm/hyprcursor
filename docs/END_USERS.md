## Using a hyprcursor theme

Download a hyprcursor theme and extract it to a new directory in `~/.local/share/icons`.

Make sure the first directory contains a manifest, for example:

```s
~/.local/share/icons/myCursorTheme/manifest.hl
```

## Overriding a theme

Set the `HYPRCURSOR_THEME` env to your theme directory,
so for example to get the above to always load, use `export HYPRCURSOR_THEME = myCursorTheme`.