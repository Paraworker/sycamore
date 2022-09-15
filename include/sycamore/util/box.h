#ifndef SYCAMORE_BOX_H
#define SYCAMORE_BOX_H

#include <stdint.h>
#include <wlr/util/box.h>

static inline void box_get_center_coords(const struct wlr_box *box, int32_t *x, int32_t *y) {
    *x = box->x + (box->width / 2);
    *y = box->y + (box->height / 2);
}

#endif //SYCAMORE_BOX_H