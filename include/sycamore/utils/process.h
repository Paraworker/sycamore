#ifndef SYCAMORE_PROCESS_H
#define SYCAMORE_PROCESS_H

#include "sycamore/defines.h"

#include <cstdint>

NAMESPACE_SYCAMORE_BEGIN

/**
 * @brief Create process
 */
uint64_t spawn(const char* cmd);

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_PROCESS_H