/* Host tests for the window table. Exit status 0 means all checks passed. */
#include <stdio.h>
#include <stdlib.h>

#include "xteds/window_map.h"

static int failures;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++;                                               \
        }                                                             \
    } while (0)

int main(void) {
    xteds_window_map *m = NULL;
    CHECK(xteds_map_init(&m) == 0, "init");

    /* Top-level window and a child inside it. */
    CHECK(xteds_map_create(m, 100, 0, 10, 20, 200, 100, 0) == 0, "create top");
    CHECK(xteds_map_create(m, 101, 100, 5, 5, 50, 50, 0) == 0, "create child");
    CHECK(xteds_map_create(m, 100, 0, 0, 0, 1, 1, 0) == -1, "duplicate xid must fail");
    CHECK(xteds_map_create(m, 102, 999, 0, 0, 1, 1, 0) == -1, "unknown parent must fail");
    CHECK(xteds_map_count(m) == 2, "count after two creates");

    CHECK(xteds_map_toplevel(m, 101) == 100, "child's top-level is 100");
    CHECK(xteds_map_toplevel(m, 100) == 100, "top-level is its own top-level");
    CHECK(xteds_map_toplevel(m, 555) == 0, "unknown window has no top-level");

    /* Nothing is hit while unmapped. */
    CHECK(xteds_map_hit_test(m, 20, 30) == 0, "unmapped windows are not hit");

    CHECK(xteds_map_set_mapped(m, 100, 1) == 0, "map top");
    CHECK(xteds_map_set_mapped(m, 101, 1) == 0, "map child");

    /* Child sits at (10+5, 20+5) in root coordinates, size 50x50. */
    CHECK(xteds_map_hit_test(m, 16, 26) == 101, "child hit at its absolute origin");
    CHECK(xteds_map_hit_test(m, 14, 26) == 100, "outside child, inside top");
    CHECK(xteds_map_hit_test(m, 9, 26) == 0, "outside top");
    CHECK(xteds_map_hit_test(m, 16 + 50, 26) == 100, "right edge of child is exclusive");

    /* Override-redirect menu above the top-level, even though it was created later. */
    CHECK(xteds_map_create(m, 200, 0, 12, 22, 30, 30, 1) == 0, "create menu");
    CHECK(xteds_map_set_mapped(m, 200, 1) == 0, "map menu");
    CHECK(xteds_map_hit_test(m, 13, 23) == 200, "override-redirect window wins");

    /* Move the top-level; the child moves with it. */
    CHECK(xteds_map_configure(m, 100, 300, 300, 200, 100) == 0, "configure top");
    CHECK(xteds_map_hit_test(m, 100, 100) == 0, "old position is empty now");
    CHECK(xteds_map_hit_test(m, 316, 326) == 101, "child follows its parent");

    /* Destroying the top-level removes its children too. */
    CHECK(xteds_map_destroy(m, 100) == 0, "destroy top");
    CHECK(xteds_map_find(m, 101) == NULL, "child removed with parent");
    CHECK(xteds_map_count(m) == 1, "only the menu remains");
    CHECK(xteds_map_destroy(m, 100) == -1, "destroying twice must fail");

    /* Table capacity. */
    xteds_map_free(m);
    CHECK(xteds_map_init(&m) == 0, "re-init");
    uint32_t created = 0;
    for (uint32_t i = 1; i <= XTEDS_MAX_WINDOWS + 4; i++) {
        if (xteds_map_create(m, i, 0, 0, 0, 1, 1, 0) == 0) {
            created++;
        }
    }
    CHECK(created == XTEDS_MAX_WINDOWS, "table fills to its limit");
    xteds_map_free(m);

    if (failures) {
        fprintf(stderr, "test_window_map: %d failure(s)\n", failures);
        return 1;
    }
    printf("test_window_map: all checks passed\n");
    return 0;
}
