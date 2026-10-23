# xteds

X11 compatibility for tesseraOS. xteds is an X server that runs as a client of
[teds](https://github.com/enderman-claude/tesseraOS-teds), the Ender display server, the
same way XWayland runs as a client of a Wayland compositor. See
[docs/DESIGN.md](docs/DESIGN.md) for the mapping and the layers.

## Status

The window table is implemented and tested (`make test`). The X protocol front end, the
teds bridge and input are stubs. `build/xteds` runs the stub and exits with status 1.
