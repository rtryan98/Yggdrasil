// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <cstdint>
#include <string>

typedef struct HINSTANCE__* HINSTANCE;
typedef struct HWND__* HWND;

namespace ygg
{
    /**
     * @brief Data-store for all Win32-window related data except the native handles.
    */
    struct Window_win32_data
    {
        uint32_t width;
        uint32_t height;
        std::string name;
        uint64_t style;
        bool is_closed;
    };

    /**
     * @brief Win32 window.
    */
    class Window_win32
    {
    public:
        /**
         * @brief Constructs a `Window_win32` instance with the given initial parameters.
        */
        Window_win32(uint32_t width, uint32_t height, const char* name);

        /**
         * @brief Runs the Message loop. Must be called from the thread that created the window.
        */
        void update();

        /**
         * @brief Returns the data-store for this window.
        */
        const Window_win32_data& window_data() const { return m_data; }

        /**
         * @brief Native Win32 window handle.
        */
        HWND hwnd() const { return m_hwnd; };

        /**
         * @brief Native Win32 instance handle.
        */
        HINSTANCE hinstance() const { return m_hinstance; };

    private:
        Window_win32_data m_data;
        HWND m_hwnd;
        HINSTANCE m_hinstance;
    };
}
