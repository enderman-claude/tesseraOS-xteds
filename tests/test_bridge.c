/* Integration test: the xteds bridge drives a real teds server over the loopback transport,
 * and the checks read the pixels that reach the teds framebuffer. Exit 0 means every check
 * passed. */
#include <stdio.h>
#include <stdlib.h>

#include "teds/loopback.h"
#include "teds/server.h"
#include "teds/teds.h"
#include "xteds/bridge.h"
#include "xteds/window_map.h"

static int failures;

#define CHECK(cond, msg)                                                  \
    do {                                                                  \
        if (!(cond)) {                                                    \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++;                                                   \
        }                                                                 \
    } while (0)

#define SCREEN_W 64u
#define SCREEN_H 48u
#define BG 0x101820u

static uint32_t screen_px(const uint32_t *screen, uint32_t x, uint32_t y) {
    return screen[(size_t)y * SCREEN_W + x];
}

static void fill(uint32_t *p, uint32_t n, uint32_t colour) {
    for (uint32_t i = 0; i < n; i++) {
        p[i] = colour;
    }
}

int main(void) {
    uint32_t *screen = calloc((size_t)SCREEN_W * SCREEN_H, sizeof *screen);
    teds_fb fb = { screen, SCREEN_W, SCREEN_H, SCREEN_W, 0 };
    teds_server *srv = NULL;
    CHECK(teds_server_init(&srv, &fb) == TEDS_OK, "teds server init");

    teds_loop *loop = NULL;
    CHECK(teds_loop_create(&loop, srv) == TEDS_OK, "loop create");
    uint32_t slot;
    CHECK(teds_server_attach(srv, teds_loop_server(loop), TEDS_CAP_CLIENT, &slot) == TEDS_OK, "attach");
    teds_conn *conn = NULL;
    CHECK(teds_connect(&conn, teds_loop_client(loop), TEDS_CAP_CLIENT) == TEDS_OK, "connect");

    xteds_window_map *map = NULL;
    xteds_map_init(&map);
    xteds_bridge *br = NULL;
    CHECK(xteds_bridge_open(&br, map, conn) == 0, "bridge open");

    /* Top-level window at (10,5), 8x8, and a child at (2,2) inside it. */
    CHECK(xteds_map_create(map, 100, 0, 10, 5, 8, 8, 0) == 0, "create top");
    CHECK(xteds_map_create(map, 101, 100, 2, 2, 4, 4, 0) == 0, "create child");
    CHECK(xteds_map_set_mapped(map, 100, 1) == 0, "map top");
    CHECK(xteds_bridge_map(br, 100) == 0, "bridge maps the top-level");
    CHECK(xteds_bridge_map(br, 101) != 0, "child windows do not get their own surface");
    CHECK(xteds_bridge_surface_count(br) == 1, "one surface after mapping");
    CHECK(teds_server_surface_count(srv) == 1, "teds server sees one surface");

    static uint32_t red[8 * 8];
    fill(red, 8 * 8, 0xFF0000u);
    CHECK(xteds_bridge_put(br, 100, red, 8, 8) == 0, "put red");
    CHECK(screen_px(screen, 10, 5) == 0xFF0000u, "top-level pixel reaches the screen");
    CHECK(screen_px(screen, 17, 12) == 0xFF0000u, "bottom-right of top-level");
    CHECK(screen_px(screen, 18, 12) == BG, "outside the window is background");

    /* Wrong size is rejected and nothing is drawn. */
    static uint32_t small[4 * 4];
    fill(small, 4 * 4, 0x00FF00u);
    CHECK(xteds_bridge_put(br, 100, small, 4, 4) == TEDS_ERR_INVALID, "size mismatch rejected");
    CHECK(screen_px(screen, 10, 5) == 0xFF0000u, "rejected put draws nothing");

    /* An override-redirect menu over the top-level gets its own surface, above it. */
    CHECK(xteds_map_create(map, 200, 0, 12, 7, 4, 4, 1) == 0, "create menu");
    CHECK(xteds_map_set_mapped(map, 200, 1) == 0, "map menu");
    CHECK(xteds_bridge_map(br, 200) == 0, "bridge maps the menu");
    static uint32_t green[4 * 4];
    fill(green, 4 * 4, 0x00FF00u);
    CHECK(xteds_bridge_put(br, 200, green, 4, 4) == 0, "put green menu");
    CHECK(screen_px(screen, 13, 8) == 0x00FF00u, "menu is above the top-level");
    CHECK(screen_px(screen, 11, 6) == 0xFF0000u, "top-level still visible around the menu");

    CHECK(xteds_bridge_unmap(br, 200) == 0, "unmap menu");
    CHECK(screen_px(screen, 13, 8) == 0xFF0000u, "menu removed, top-level shows through");

    /* Move the top-level. The next put recreates its surface at the new position. */
    CHECK(xteds_map_configure(map, 100, 30, 20, 8, 8) == 0, "move top");
    CHECK(xteds_bridge_put(br, 100, red, 8, 8) == 0, "put after move");
    CHECK(screen_px(screen, 10, 5) == BG, "old position is empty");
    CHECK(screen_px(screen, 30, 20) == 0xFF0000u, "window drawn at its new position");
    CHECK(teds_server_surface_count(srv) == 1, "still exactly one surface after the move");

    CHECK(xteds_bridge_unmap(br, 100) == 0, "unmap top");
    CHECK(teds_server_surface_count(srv) == 0, "no surfaces left on the server");
    CHECK(xteds_bridge_unmap(br, 100) != 0, "unmapping twice fails");

    xteds_bridge_close(br);
    teds_disconnect(conn);
    teds_loop_free(loop);
    teds_server_free(srv);
    xteds_map_free(map);
    free(screen);

    if (failures) {
        fprintf(stderr, "test_bridge: %d failure(s)\n", failures);
        return 1;
    }
    printf("test_bridge: all checks passed\n");
    return 0;
}
