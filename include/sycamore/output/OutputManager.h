#ifndef SYCAMORE_OUTPUT_MANAGER_H
#define SYCAMORE_OUTPUT_MANAGER_H

#include "sycamore/output/Output.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <list>

namespace sycamore
{

class OutputManager
{
public:
    /**
     * @brief Constructor
     */
    OutputManager() = default;

    /**
     * @brief Destructor
     */
    ~OutputManager() = default;

    /**
     * @brief Add a new output
     */
    void addOutput(wlr_output* handle);

    /**
     * @brief Remove an output
     */
    void removeOutput(Output* output);

    /**
     * @brief Get the number all outputs
     */
    size_t outputCount() const;

    /**
     * @brief Find output at the given position
     */
    static Output* findOutputAt(const Point<double>& coords);

private:
    std::list<Output> m_outputs;
};

inline OutputManager outputManager{};

}

#endif //SYCAMORE_OUTPUT_MANAGER_H