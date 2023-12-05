#ifndef SYCAMORE_PROCESS_H
#define SYCAMORE_PROCESS_H

#include <cstdint>

namespace sycamore
{

/**
 * @brief Create process
 */
uint64_t spawn(const char* cmd);

}

#endif //SYCAMORE_PROCESS_H