/* Window table: X11 windows and their teds surfaces. Not thread-safe. */
#ifndef XTEDS_WINDOW_MAP_H
#define XTEDS_WINDOW_MAP_H

#include <stdint.h>

#define XTEDS_MAX_WINDOWS 256

typedef struct xteds_window {
    uint32_t xid;          /* X window id; 0 means the slot is free */
    uint32_t parent;       /* parent X window id; 0 for the root */
    int32_t x, y;
    uint32_t w, h;
    int mapped;
    int override_redirect; /* menus and tooltips: placed above the parent */
    uint32_t surface;      /* teds surface id; 0 if this window has none yet */
} xteds_window;

typedef struct xteds_window_map xteds_window_map;

/* Returns 0 on success. Negative on allocation failure. */
int xteds_map_init(xteds_window_map **out);
void xteds_map_free(xteds_window_map *m);

/* Create a window. Returns 0 on success, -1 if xid is taken or the table is full. */
int xteds_map_create(xteds_window_map *m, uint32_t xid, uint32_t parent,
                     int32_t x, int32_t y, uint32_t w, uint32_t h, int override_redirect);
int xteds_map_destroy(xteds_window_map *m, uint32_t xid);  /* also removes children */

/* Map or unmap a window. Returns -1 if xid is unknown. */
int xteds_map_set_mapped(xteds_window_map *m, uint32_t xid, int mapped);
int xteds_map_configure(xteds_window_map *m, uint32_t xid,
                        int32_t x, int32_t y, uint32_t w, uint32_t h);

/* Lookup. Returns NULL if unknown. */
xteds_window *xteds_map_find(xteds_window_map *m, uint32_t xid);

/* Top-level ancestor of xid (the window that owns a teds surface). Returns 0 if unknown. */
uint32_t xteds_map_toplevel(const xteds_window_map *m, uint32_t xid);

/* Absolute position of xid in root coordinates. Returns 0 on success, -1 if unknown. */
int xteds_map_origin(const xteds_window_map *m, uint32_t xid, int32_t *x, int32_t *y);

/* Hit test in root coordinates, topmost first. Returns the X window id, or 0 for none. */
uint32_t xteds_map_hit_test(const xteds_window_map *m, int32_t x, int32_t y);

/* Number of windows currently in the table. */
uint32_t xteds_map_count(const xteds_window_map *m);

#endif
