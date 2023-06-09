#ifndef SYCAMORE_OUTPUT_LAYOUT_H
#define SYCAMORE_OUTPUT_LAYOUT_H

#include "sycamore/defines.h"
#include "sycamore/utils/List.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <memory>

NAMESPACE_SYCAMORE_BEGIN

class Output;

class OutputLayout {
public:
    using Ptr = std::unique_ptr<OutputLayout>;

public:
    /**
     * @brief Create OutputLayout
     *
     * Return nullptr on failed
     */
    static OutputLayout::Ptr create();

public:
    ~OutputLayout();

    OutputLayout(const OutputLayout&) = delete;
    OutputLayout(OutputLayout&&) = delete;
    OutputLayout& operator=(const OutputLayout&) = delete;
    OutputLayout& operator=(OutputLayout&&) = delete;

    auto getHandle() const { return m_handle; }

    size_t getOutputNum() const { return m_outputList.size(); }

    bool add(Output* output);

    void remove(Output* output);

    Output* findOutputAt(const Point<double>& coords) const;

private:
    explicit OutputLayout(wlr_output_layout* handle);

private:
    wlr_output_layout* m_handle;
    List               m_outputList;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_OUTPUT_LAYOUT_H