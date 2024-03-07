## Creating a theme

Familiarize yourself with the README of `hyprcursor-util`.

## Creating a theme from an XCursor theme

Download an XCursor theme, extract it, and then use `--extract`, and then on the resulting output, `--create`.

## Creating a theme from scratch

The directory structure looks like this:
```ini
directory
 ┣ manifest.hl
 ┗ hyprcursors
   ┣ left_ptr
     ┣ image32.png
     ┣ image64.png
     ┗ meta.hl
   ┣ hand
     ┣ image32.png
     ┣ image64.png
     ┗ meta.hl
   ...
```

### Manifest

The manifest describes your theme, in hyprlang:
```ini
name = My theme!
description = Very cool!
version = 0.1
cursors_directory = hyprcursors # has to match the directory in the structure
```

### Cursors

Each cursor image is a separate directory. In it, multiple size variations can be put.

`meta.hl` describes the cursor:
```ini
# what resize algorithm to use when a size is requested
# that doesn't match any of your predefined ones.
# available: bilinear, nearest, none. None will pick the closest. Nearest is nearest neighbor.
resize_algorithm = bilinear

# "hotspot" is where in your cursor the actual "click point" should be.
# this is in absolute coordinates. x+ is east, y+ is north.
hotspot_x = 0.0 # this goes 0 - 1
hotspot_y = 0.0 # this goes 0 - 1

# Define what cursor images this one should override.
# What this means is that a request for a cursor name e.g. "arrow"
# will instead use this one, even if this one is named something else.
define_override = arrow
define_override = default

# define your size variants.
# Multiple size variants for the same size are treated as an animation.
define_size = 64, image64.png
define_size = 32, image32.png

# If you want to animate it, add a timeout in ms at the end:
# define_size = 64, anim1.png, 500
# define_size = 64, anim2.png, 500
# define_size = 64, anim3.png, 500
# define_size = 64, anim4.png, 500
```

Supported cursor image types are png and soon svg.