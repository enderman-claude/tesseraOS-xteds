#include <stdlib.h>
#include <string.h>

#include "xteds/bridge.h"

typedef struct entry {
    uint32_t xid;               /* 0 means the slot is free */
    teds_surface *surface;
    int32_t x, y;
    uint32_t w, h;
} entry;

struct xteds_bridge {
    xteds_window_map *map;
    teds_conn *conn;
    entry entries[XTEDS_MAX_WINDOWS];
    uint32_t count;
};

int xteds_bridge_open(xteds_bridge **out, xteds_window_map *map, teds_conn *conn) {
    xteds_bridge *b = calloc(1, sizeof *b);
    if (b == NULL) {
        return TEDS_ERR_NOMEM;
    }
    b->map = map;
    b->conn = conn;
    *out = b;
    return 0;
}

void xteds_bridge_close(xteds_bridge *b) {
    if (b == NULL) {
        return;
    }
    for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
        if (b->entries[i].xid != 0) {
            teds_surface_destroy(b->entries[i].surface);
        }
    }
    free(b);
}

static entry *find(xteds_bridge *b, uint32_t xid) {
    for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
        if (b->entries[i].xid == xid) {
            return &b->entries[i];
        }
    }
    return NULL;
}

static int geometry(xteds_bridge *b, uint32_t xid, int32_t *x, int32_t *y, uint32_t *w, uint32_t *h) {
    xteds_window *win = xteds_map_find(b->map, xid);
    if (win == NULL || xteds_map_toplevel(b->map, xid) != xid) {
        return -1;
    }
    if (xteds_map_origin(b->map, xid, x, y) != 0) {
        return -1;
    }
    *w = win->w;
    *h = win->h;
    return 0;
}

int xteds_bridge_map(xteds_bridge *b, uint32_t xid) {
    if (find(b, xid) != NULL) {
        return -1;
    }
    int32_t x, y;
    uint32_t w, h;
    if (geometry(b, xid, &x, &y, &w, &h) != 0 || w == 0 || h == 0) {
        return -1;
    }
    for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
        if (b->entries[i].xid == 0) {
            teds_surface *s = NULL;
            int st = teds_surface_create(b->conn, w, h, x, y, &s);
            if (st != TEDS_OK) {
                return st;
            }
            b->entries[i] = (entry){ xid, s, x, y, w, h };
            b->count++;
            return 0;
        }
    }
    return TEDS_ERR_NOMEM;
}

int xteds_bridge_unmap(xteds_bridge *b, uint32_t xid) {
    entry *e = find(b, xid);
    if (e == NULL) {
        return -1;
    }
    int st = teds_surface_destroy(e->surface);
    if (st != TEDS_OK) {
        return st;
    }
    memset(e, 0, sizeof *e);
    b->count--;
    return 0;
}

int xteds_bridge_put(xteds_bridge *b, uint32_t xid, const uint32_t *pixels, uint32_t w, uint32_t h) {
    entry *e = find(b, xid);
    if (e == NULL) {
        return -1;
    }
    int32_t x, y;
    uint32_t ww, hh;
    if (geometry(b, xid, &x, &y, &ww, &hh) != 0 || ww != w || hh != h) {
        return TEDS_ERR_INVALID;
    }
    if (x != e->x || y != e->y || w != e->w || h != e->h) {
        /* Moved or resized: recreate the surface at the new geometry. */
        if (teds_surface_destroy(e->surface) != TEDS_OK) {
            return TEDS_ERR_PROTOCOL;
        }
        teds_surface *s = NULL;
        int st = teds_surface_create(b->conn, w, h, x, y, &s);
        if (st != TEDS_OK) {
            memset(e, 0, sizeof *e);
            b->count--;
            return st;
        }
        e->surface = s;
        e->x = x;
        e->y = y;
        e->w = w;
        e->h = h;
    }
    memcpy(teds_surface_pixels(e->surface), pixels, (size_t)w * h * sizeof *pixels);
    return teds_surface_commit(e->surface, 0, 0, w, h);
}

uint32_t xteds_bridge_surface_count(const xteds_bridge *b) {
    return b->count;
}
