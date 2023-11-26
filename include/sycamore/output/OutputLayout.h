#ifndef SYCAMORE_OUTPUT_LAYOUT_H
#define SYCAMORE_OUTPUT_LAYOUT_H

#include "sycamore/defines.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <memory>

NAMESPACE_SYCAMORE_BEGIN

class Output;

class OutputLayout
{
public:
    using UPtr = std::unique_ptr<OutputLayout>;

public:
    /**
     * @brief Create OutputLayout
     * @return nullptr on failure
     */
    static OutputLayout::UPtr create(wl_display* display);

    ~OutputLayout();

    auto getHandle() const
    {
        return m_handle;
    }

    size_t getOutputCount() const
    {
        return m_outputCount;
    }

    bool addAuto(Output* output);

    void remove(Output* output);

    Output* findOutputAt(const Point<double>& coords) const;

    OutputLayout(const OutputLayout&) = delete;
    OutputLayout(OutputLayout&&) = delete;
    OutputLayout& operator=(const OutputLayout&) = delete;
    OutputLayout& operator=(OutputLayout&&) = delete;

private:
    explicit OutputLayout(wlr_output_layout* handle);

private:
    wlr_output_layout* m_handle;
    size_t             m_outputCount;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_OUTPUT_LAYOUT_H