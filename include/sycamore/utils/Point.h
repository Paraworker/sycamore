#ifndef SYCAMORE_POINT_H
#define SYCAMORE_POINT_H

#include "sycamore/defines.h"
#include <type_traits>

NAMESPACE_SYCAMORE_BEGIN

template<typename T>
struct Point {
    T x;
    T y;

    constexpr Point<T> operator+(const Point<T>& rhs) const {
        return {x + rhs.x, y + rhs.y};
    }

    constexpr Point<T> operator-(const Point<T>& rhs) const {
        return {x - rhs.x, y - rhs.y};
    }

    template<typename U>
    requires std::is_convertible_v<T, U>
    constexpr Point<U> into() const {
        return {static_cast<U>(x), static_cast<U>(y)};
    }
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_POINT_H