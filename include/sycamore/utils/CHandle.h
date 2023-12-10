#ifndef SYCAMORE_C_HANDLE_H
#define SYCAMORE_C_HANDLE_H

#include <memory>

namespace sycamore
{

// Destroy resource by a function
template<auto fn>
struct DeleterFromFn
{
    template<typename T>
    constexpr void operator()(T* handle) const noexcept
    {
        fn(handle);
    }
};

// Manage C resource in a RAII way
template<typename T, auto fn>
using CHandle = std::unique_ptr<T, DeleterFromFn<fn>>;

}

#endif //SYCAMORE_C_HANDLE_H