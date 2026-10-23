/* Bridge from X windows to teds surfaces. Each top-level X window (including override-redirect
 * windows) owns one teds surface. Child windows are drawn into their top-level's surface.
 * Uses only the teds client API, so it works over any teds transport. */
#ifndef XTEDS_BRIDGE_H
#define XTEDS_BRIDGE_H

#include <stdint.h>

#include "teds/teds.h"
#include "xteds/window_map.h"

typedef struct xteds_bridge xteds_bridge;

/* The bridge does not own the window map or the connection. */
int xteds_bridge_open(xteds_bridge **out, xteds_window_map *map, teds_conn *conn);
void xteds_bridge_close(xteds_bridge *b);

/* Create the teds surface for a mapped top-level window, at its absolute position and size.
 * Returns 0 on success. Negative teds status or -1 if the window is unknown, not top-level,
 * or already has a surface. */
int xteds_bridge_map(xteds_bridge *b, uint32_t xid);

/* Destroy the surface for xid. Returns 0, or -1 if it has none. */
int xteds_bridge_unmap(xteds_bridge *b, uint32_t xid);

/* Copy w*h pixels (0x00RRGGBB) into the window's surface and commit. w and h must match the
 * window's size. If the window moved or resized since xteds_bridge_map, the surface is
 * recreated at the new geometry first (teds v1 has no move operation). */
int xteds_bridge_put(xteds_bridge *b, uint32_t xid, const uint32_t *pixels, uint32_t w, uint32_t h);

uint32_t xteds_bridge_surface_count(const xteds_bridge *b);

#endif
