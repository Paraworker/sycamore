#include "sycamore/input/Cursor.h"

#include "sycamore/output/OutputManager.h"
#include <string>

namespace sycamore
{

static wlr_xcursor_manager* createXcursorManager(const char* theme = nullptr, uint32_t size = 24)
{
    if (theme)
    {
        setenv("XCURSOR_THEME", theme, true);
    }

    setenv("XCURSOR_SIZE", std::to_string(size).c_str(), true);

    return wlr_xcursor_manager_create(theme, size);
}

Cursor::Cursor()
    : m_handle{wlr_cursor_create()}
    , m_xcursorManager{createXcursorManager()}
    , m_xcursor{}
{}

void Cursor::init(wlr_output_layout* layout)
{
    wlr_cursor_attach_output_layout(m_handle, layout);
}

Cursor::~Cursor()
{
    wlr_xcursor_manager_destroy(m_xcursorManager);
    wlr_cursor_destroy(m_handle);
}

void Cursor::setXcursor(const char* name)
{
    m_xcursor = name;
    wlr_cursor_set_xcursor(m_handle, m_xcursorManager, name);
}

void Cursor::setSurface(wlr_surface* surface, const Point<int32_t>& hotspot)
{
    m_xcursor = nullptr;
    wlr_cursor_set_surface(m_handle, surface, hotspot.x, hotspot.y);
}

void Cursor::hide()
{
    m_xcursor = nullptr;
    wlr_cursor_unset_image(m_handle);
}

void Cursor::refreshXcursor()
{
    warp(position());

    if (m_xcursor)
    {
        wlr_cursor_set_xcursor(m_handle, m_xcursorManager, m_xcursor);
    }
}

void Cursor::updateXcursorTheme(const char* theme, uint32_t size)
{
    if (m_xcursorManager)
    {
        wlr_xcursor_manager_destroy(m_xcursorManager);
    }

    m_xcursorManager = createXcursorManager(theme, size);

    refreshXcursor();
}

Output* Cursor::atOutput() const
{
    return OutputManager::findOutputAt(position());
}

}