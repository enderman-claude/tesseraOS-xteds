/* xteds entry point. Stub: the X front end, the teds bridge and input are not written yet.
 * This file runs the window table so the binary exercises real code; it exits with
 * status 1 because it cannot serve X clients yet. */
#include <stdio.h>

#include "xteds/window_map.h"

int main(void) {
    xteds_window_map *m = NULL;
    if (xteds_map_init(&m) != 0) {
        return 2;
    }
    fprintf(stderr, "xteds: stub. X front end, teds bridge and input are not implemented.\n");
    xteds_map_free(m);
    return 1;
}
