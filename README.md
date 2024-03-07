## hyprcursor
The hyprland cursor format, library and utilities.

Please note it's in very early development.

## Why?

XCursor sucks, and we still use it today.
 - Scaling of XCursors is horrible
 - XCursor does not support vector cursors
 - XCursor is ridiculously space-inefficient

Hyprcursor fixes all three. It's an efficient cursor theme format that
doesn't suck as much.

### Notable advantages over XCursor
 - Automatic scaling according to a configurable, per-cursor method.
 - Support for SVG cursors (soon, see todo)
 - Way more space-efficient. As an example, Bibata-XCursor is 44.1MB, while it's 6.6MB in hyprcursor.

## Tools

### hyprcursor-util

Utility for creating hyprcursor themes. See its readme in `hyprcursor-util/`

### libhyprcursor

The library to use for implementing hyprcursors in your compositor or app.

It provides C and C++ bindings.

## TODO

Library:
 - [x] Support animated cursors
 - [ ] Support SVG cursors

Util:
 - [ ] Support compiling a theme with X
 - [ ] Support decompiling animated cursors
