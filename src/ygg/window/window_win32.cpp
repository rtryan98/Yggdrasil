// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/window/Window_win32.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace ygg
{
    LRESULT CALLBACK Wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        Window_win32_data& data = *reinterpret_cast<Window_win32_data*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        switch (msg)
        {
        default: break;
        case WM_ACTIVATE:
            break;
        case WM_CLOSE:
            data.is_closed = true;
            break;
        case WM_KEYDOWN:
            break;
        case WM_KEYUP:
            break;
        case WM_EXITSIZEMOVE:
            break;
        case WM_HSCROLL:
            break;
        case WM_VSCROLL:
            break;
        case WM_QUIT:
            break;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    Window_win32::Window_win32(uint32_t width, uint32_t height, const char* name)
        : m_data{ width, height, name, false }, m_hwnd(0), m_hinstance(GetModuleHandle(nullptr))
    {
        DWORD style = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        RECT wr = {
            .left = 0,
            .top = 0,
            .right = LONG(m_data.width),
            .bottom = LONG(m_data.height)
        };
        AdjustWindowRect(&wr, style, false);

        WNDCLASSEX wc = {
            .cbSize = sizeof(WNDCLASSEX),
            .style = 0,
            .lpfnWndProc = Wnd_proc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = GetModuleHandle(nullptr),
            .hIcon = LoadIcon(nullptr, IDI_WINLOGO),
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH)),
            .lpszMenuName = nullptr,
            .lpszClassName = m_data.name.c_str(),
            .hIconSm = wc.hIcon,
        };
        RegisterClassEx(&wc);

        m_hwnd = CreateWindowEx(0, wc.lpszClassName, wc.lpszClassName, style, 0, 0, wr.right - wr.left, wr.bottom - wr.top,
            nullptr, nullptr, m_hinstance, 0);
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&m_data));

        ShowWindow(m_hwnd, SW_SHOWDEFAULT);
        SetForegroundWindow(m_hwnd);
        SetFocus(m_hwnd);
    }

    void Window_win32::update()
    {
        MSG msg = {};
        ZeroMemory(&msg, sizeof(MSG));
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
#endif
