## Creating a theme

Familiarize yourself with the README of `hyprcursor-util`.

## Creating a theme from an XCursor theme

Download an XCursor theme, extract it, and then use `--extract`, and then on the resulting output, `--create`.

Before `--create`, you probably should walk through the `manifest.hl` and all the `meta.hl` files to make sure they're correct,
and adjust them to your taste.

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
# this is in absolute coordinates. x+ is east, y+ is south.
# the pixel coordinates of the hotspot at size are rounded to the nearest:
# (round(size * hotspot_x), round(size * hotspot_y))
hotspot_x = 0.0 # this goes 0 - 1
hotspot_y = 0.0 # this goes 0 - 1

# Define what cursor images this one should override.
# What this means is that a request for a cursor name e.g. "arrow"
# will instead use this one, even if this one is named something else.
# There is no unified list for all the available cursor names but this wayland list could be used as a reference https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/staging/cursor-shape/cursor-shape-v1.xml#L71 for wayland specific cursors.
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
# Make sure the timeout is > 0, as otherwise the consumer might ignore your timeouts for being invalid.
```

Supported cursor image types are png and svg.

If you are using an svg cursor, the size parameter will be ignored. 

Mixing png and svg cursor images in one shape will result in an error.

All cursors are required to have an aspect ratio of 1:1.

Please note animated svgs are not supported, you need to add a separate svg for every frame.

### TOML

You are allowed to use TOML for all .hl files. Make sure to change the extension from `.hl` to `.toml`!

#### Manifest

Append `[General]` to the top, and wrap all the values in quotes.

#### Meta

Append `[General]` to the top, and wrap all values except hotspot in quotes.

Additionally, if you have multiple `define_*` keys, merge them into one like this:
```toml
define_override = 'shape1;shape2;shape3'
define_size = '24,image1.png,200;24,image2.png,200;32,image3.png,200'
```

You can put spaces around the semicolons if you prefer to.
