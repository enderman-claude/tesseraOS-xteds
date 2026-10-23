# xteds: X11 compatibility for tesseraOS

xteds is an X11 server that runs as a client of teds, the Ender display server, the way
XWayland runs as a client of a Wayland compositor. X11 programs connect to xteds as they
would to any X server. xteds turns their windows into teds surfaces, so they share the
screen with native teds apps.

## Mapping

| X11 concept | teds concept |
|---|---|
| Top-level window | Surface (client mode) |
| Child window | Drawn into its top-level's surface |
| Window pixmap / backing store | Surface pixel buffer |
| Override-redirect window (menus, tooltips) | Surface placed above its parent, same compositing |
| Input from teds (pointer, keys) | X events delivered to the focused window |
| Fullscreen X window | Optional direct mode request, if the manifest grants `display-direct` |

Direct mode is for things like a fullscreen game or an OSD. It is not the default: xteds
uses client mode unless a window asks to be fullscreen and the manifest allows direct mode.

## Layers

1. X protocol front end: listens on a unix socket, performs the connection setup,
   handles core requests (windows, pixmaps, GC, drawing), and the MIT-SHM extension if
   present. Stub for now.
2. Window table (`src/window_map.c`): tracks X window ids, their parent, geometry and
   whether they are mapped. Maps each top-level to a teds surface id. Implemented and
   host-tested.
3. teds bridge: creates, commits and destroys surfaces through the teds client API. Stub
   until the teds client library has sources (teds currently has protocol and API headers
   only).
4. Input bridge: receives pointer and key events from teds, finds the focused X window,
   and sends the matching X event. Stub.

## Scope and non-goals

- No XRender, no GLX, no Composite redirection beyond what layer 2 needs at first.
- No window manager inside xteds. A window manager runs as an X client, as on XWayland.
- Selections and drag-and-drop bridging come after input works.

## Status

| Part | State |
|---|---|
| Window table | Implemented, host-tested |
| X protocol front end | Stub |
| teds bridge | Stub, depends on teds client sources |
| Input bridge | Stub |
