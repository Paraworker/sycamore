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
     * @throw std::runtime_error on failure
     * @lifetime Be destroyed by listener
     */
    static OutputLayout* create(wl_display* display);

    bool addAuto(Output* output);

    void remove(Output* output);

    Output* findOutputAt(const Point<double>& coords) const;

    auto getHandle() const
    {
        return m_handle;
    }

    size_t getOutputCount() const
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
    explicit OutputLayout(wl_display* display);

    /**
     * @brief Destructor
     */
    ~OutputLayout();

private:
    wlr_output_layout* m_handle;
    size_t             m_outputCount;

private:
    Listener m_destroy;
};

}

#endif //SYCAMORE_OUTPUT_LAYOUT_H