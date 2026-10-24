# xteds

X11 compatibility for tesseraOS. xteds is an X server that runs as a client of
[teds](https://github.com/enderman-claude/tesseraOS-teds), the Ender display server, the
same way XWayland runs as a client of a Wayland compositor. See
[docs/DESIGN.md](docs/DESIGN.md) for the mapping and the layers.

## Status

The window table and the teds bridge are implemented. `make test` runs the window table
tests and an integration test that drives a real teds server over loopback (set `TEDS_REPO`
to a teds checkout, default `../tesseraOS-teds`). The X protocol front end and input are
stubs. `build/xteds` runs the stub and exits with status 1.

## Related repos

- [tesseraOS-teds](https://github.com/enderman-claude/tesseraOS-teds): the display server xteds runs on as a client. Its window table and X windows map to teds surfaces.
- [tesseraOS](https://github.com/enderman-claude/tesseraOS): the OS that both servers are part of.

*Footnote: xteds speaks X fluently, so it never has to explain the conversation.*
