#ifndef SYCAMORE_POINTER_H
#define SYCAMORE_POINTER_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class Pointer final : public InputDevice {
public:
    static void onCreate(wlr_input_device* deviceHandle);

public:
    Pointer(const Pointer&) = delete;
    Pointer(Pointer&&) = delete;
    Pointer& operator=(const Pointer&) = delete;
    Pointer& operator=(Pointer&&) = delete;

private:
    Pointer(wlr_input_device* deviceHandle, wlr_pointer* pointerHandle);
    ~Pointer();

private:
    wlr_pointer* m_pointerHandle;

private:
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_POINTER_H