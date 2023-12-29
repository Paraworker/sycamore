#ifndef SYCAMORE_OUTPUT_LAYOUT_H
#define SYCAMORE_OUTPUT_LAYOUT_H

#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class Output;

class OutputLayout
{
public:
    /**
     * @brief Create OutputLayout
     * @return nullptr on failure
     */
    static OutputLayout* create(wl_display* display);

    bool addAuto(Output* output);

    void remove(Output* output);

    Output* findOutputAt(const Point<double>& coords) const;

    /**
     * @brief Get wlr_output_layout
     */
    auto getHandle() noexcept
    {
        return m_handle;
    }

    size_t getOutputCount() const noexcept
    {
        return m_outputCount;
    }

    OutputLayout(const OutputLayout&) = delete;
    OutputLayout(OutputLayout&&) = delete;
    OutputLayout& operator=(const OutputLayout&) = delete;
    OutputLayout& operator=(OutputLayout&&) = delete;

private:
    /**
     * @brief Constructor
     */
    explicit OutputLayout(wlr_output_layout* handle);

    /**
     * @brief Destructor
     */
    ~OutputLayout();

private:
    wlr_output_layout* m_handle;
    size_t             m_outputCount;

    Listener           m_destroy;
};

}

#endif //SYCAMORE_OUTPUT_LAYOUT_H