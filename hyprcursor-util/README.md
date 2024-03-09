## hyprcursor-util

A utility to compile, pack, unpack, etc, hyprcursor and xcursor themes.

## Runtime deps
 - xcur2png

## States

Cursor themes can be in 3 states:
 - compiled hyprcursor - these can be used by apps / compositors.
 - compiled xcursor - these can be used by xcursor
 - working state - an easy to navigate mode where every cursor is a png / svg, and all the meta is in files.

## Commands

`--create | -c [path]` -> create a compiled hyprcursor theme from a working state

`--extract | -x [path]` -> extract an xcursor theme into a working state

both commands support `--output | -o` to specify an output directory. For safety reasons, **do not use this on versions below 0.1.1** as it will
nuke the specified directory without asking.

Since v0.1.2, this directory is the parent, the theme will be written to a subdirectory in it called `$ACTION_$NAME`.

### Flags

`--resize [mode]` - for `extract`: specify a default resize algorithm for shapes. Default is `none`.