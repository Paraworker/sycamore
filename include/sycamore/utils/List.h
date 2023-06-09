#ifndef SYCAMORE_LIST_H
#define SYCAMORE_LIST_H

#include "sycamore/defines.h"
#include <wayland-util.h>

NAMESPACE_SYCAMORE_BEGIN

// A wrapper around wl_list
class List {
public:
    List() { wl_list_init(&m_handle); }

    void add(wl_list& link) {
        wl_list_insert(&m_handle, &link);
        ++m_size;
    }

    void remove(wl_list& link) {
        wl_list_remove(&link);
        --m_size;
    }

    void reinsert(wl_list& link) {
        wl_list_remove(&link);
        wl_list_insert(&m_handle, &link);
    }

    auto& getHandle() const { return m_handle; }

    size_t size() const { return m_size; }

private:
    wl_list m_handle{};
    size_t  m_size{0};
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_LIST_H