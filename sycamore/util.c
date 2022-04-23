#include <time.h>
#include <sycamore/util.h>

uint32_t get_current_time_msec() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}
