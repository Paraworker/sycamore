#ifndef SYCAMORE_TIME_H
#define SYCAMORE_TIME_H

#include <stdint.h>
#include <time.h>

static inline uint32_t get_current_time_msec() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

#endif //SYCAMORE_TIME_H