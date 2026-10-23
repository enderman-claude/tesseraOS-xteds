/* Window table. Slots are kept in creation order, so later slots are on top. Override-
 * redirect windows are checked before ordinary ones in hit tests. */
#include <stdlib.h>
#include <string.h>

#include "xteds/window_map.h"

#define MAX_DEPTH 64

struct xteds_window_map {
    xteds_window win[XTEDS_MAX_WINDOWS];
    uint32_t count;
};

int xteds_map_init(xteds_window_map **out) {
    xteds_window_map *m = calloc(1, sizeof *m);
    if (m == NULL) {
        return -1;
    }
    *out = m;
    return 0;
}

void xteds_map_free(xteds_window_map *m) {
    free(m);
}

xteds_window *xteds_map_find(xteds_window_map *m, uint32_t xid) {
    if (m == NULL || xid == 0) {
        return NULL;
    }
    for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
        if (m->win[i].xid == xid) {
            return &m->win[i];
        }
    }
    return NULL;
}

int xteds_map_create(xteds_window_map *m, uint32_t xid, uint32_t parent,
                     int32_t x, int32_t y, uint32_t w, uint32_t h, int override_redirect) {
    if (m == NULL || xid == 0 || xteds_map_find(m, xid) != NULL) {
        return -1;
    }
    if (parent != 0 && xteds_map_find(m, parent) == NULL) {
        return -1;
    }
    for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
        if (m->win[i].xid == 0) {
            xteds_window *win = &m->win[i];
            memset(win, 0, sizeof *win);
            win->xid = xid;
            win->parent = parent;
            win->x = x;
            win->y = y;
            win->w = w;
            win->h = h;
            win->override_redirect = override_redirect;
            m->count++;
            return 0;
        }
    }
    return -1;
}

/* Removes xid and then every window that names it as a parent, at any depth. Repeats
 * until nothing changes, so the order of slots does not matter. */
int xteds_map_destroy(xteds_window_map *m, uint32_t xid) {
    xteds_window *win = xteds_map_find(m, xid);
    if (win == NULL) {
        return -1;
    }
    memset(win, 0, sizeof *win);
    m->count--;

    int removed = 1;
    while (removed) {
        removed = 0;
        for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
            xteds_window *c = &m->win[i];
            if (c->xid != 0 && xteds_map_find(m, c->parent) == NULL && c->parent != 0) {
                memset(c, 0, sizeof *c);
                m->count--;
                removed = 1;
            }
        }
    }
    return 0;
}

int xteds_map_set_mapped(xteds_window_map *m, uint32_t xid, int mapped) {
    xteds_window *win = xteds_map_find(m, xid);
    if (win == NULL) {
        return -1;
    }
    win->mapped = mapped ? 1 : 0;
    return 0;
}

int xteds_map_configure(xteds_window_map *m, uint32_t xid,
                        int32_t x, int32_t y, uint32_t w, uint32_t h) {
    xteds_window *win = xteds_map_find(m, xid);
    if (win == NULL) {
        return -1;
    }
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;
    return 0;
}

uint32_t xteds_map_toplevel(const xteds_window_map *m, uint32_t xid) {
    uint32_t cur = xid;
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        const xteds_window *win = NULL;
        for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
            if (m->win[i].xid == cur) {
                win = &m->win[i];
                break;
            }
        }
        if (win == NULL) {
            return 0;
        }
        if (win->parent == 0) {
            return cur;
        }
        cur = win->parent;
    }
    return 0;
}

/* Absolute origin of a window: the sum of its ancestors' offsets. */
static int absolute_origin(const xteds_window_map *m, const xteds_window *win,
                           int64_t *ox, int64_t *oy) {
    int64_t sx = 0, sy = 0;
    const xteds_window *cur = win;
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        sx += cur->x;
        sy += cur->y;
        if (cur->parent == 0) {
            *ox = sx;
            *oy = sy;
            return 1;
        }
        const xteds_window *next = NULL;
        for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
            if (m->win[i].xid == cur->parent) {
                next = &m->win[i];
                break;
            }
        }
        if (next == NULL) {
            return 0;
        }
        cur = next;
    }
    return 0;
}

int xteds_map_origin(const xteds_window_map *m, uint32_t xid, int32_t *x, int32_t *y) {
    const xteds_window *win = NULL;
    for (uint32_t i = 0; i < XTEDS_MAX_WINDOWS; i++) {
        if (m->win[i].xid == xid) {
            win = &m->win[i];
            break;
        }
    }
    if (win == NULL) {
        return -1;
    }
    int64_t ox, oy;
    if (!absolute_origin(m, win, &ox, &oy)) {
        return -1;
    }
    *x = (int32_t)ox;
    *y = (int32_t)oy;
    return 0;
}

static int contains(const xteds_window_map *m, const xteds_window *win, int32_t px, int32_t py) {
    if (!win->mapped) {
        return 0;
    }
    int64_t ox, oy;
    if (!absolute_origin(m, win, &ox, &oy)) {
        return 0;
    }
    return px >= ox && py >= oy && px < ox + (int64_t)win->w && py < oy + (int64_t)win->h;
}

uint32_t xteds_map_hit_test(const xteds_window_map *m, int32_t x, int32_t y) {
    /* Pass 1: override-redirect windows, topmost (latest slot) first. */
    for (int pass = 0; pass < 2; pass++) {
        int want_override = (pass == 0);
        for (uint32_t i = XTEDS_MAX_WINDOWS; i-- > 0;) {
            const xteds_window *win = &m->win[i];
            if (win->xid == 0 || (win->override_redirect != 0) != want_override) {
                continue;
            }
            if (contains(m, win, x, y)) {
                return win->xid;
            }
        }
    }
    return 0;
}

uint32_t xteds_map_count(const xteds_window_map *m) {
    return m->count;
}
