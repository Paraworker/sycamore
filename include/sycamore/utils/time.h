#ifndef SYCAMORE_TIME_H
#define SYCAMORE_TIME_H

#include <chrono>

namespace sycamore
{

template<typename D = std::chrono::milliseconds>
auto getMonotonic()
{
    using namespace std::chrono;
    return time_point_cast<D>(steady_clock::now()).time_since_epoch().count();
}

}

#endif //SYCAMORE_TIME_H