/***************************************************************************
    Author: Ben Stone
    Project: Kauai

    Cross-platform window handle class

***************************************************************************/

#ifndef KAUAI_KWND_H
#define KAUAI_KWND_H

#ifdef KAUAI_SDL
#include <SDL.h>
#include <SDL_syswm.h>
#endif

#define kwndNil nullptr

// Window handle class
class KWND
{
  public:
    KWND() = default;

#ifdef WIN
    KWND(std::nullptr_t) : _hwnd(nullptr)
    {
    }

    KWND(HWND hwnd) : _hwnd(hwnd)
    {
    }
#endif // WIN

#ifdef KAUAI_SDL
    KWND(std::nullptr_t) : _wnd(nullptr)
    {
    }

    // Create a KWND from an SDL Window handle
    explicit KWND(SDL_Window *wnd)
    {
        _wnd = wnd;
#ifdef WIN
        _hwnd = HwndFromSDLWindow(_wnd);
#endif
    }

    // Get SDL Window handle
    operator SDL_Window *() const
    {
        return _wnd;
    }

    // Set the KWND to an SDL window handle
    KWND &operator=(SDL_Window *wnd)
    {
        _wnd = wnd;
#ifdef WIN
        _hwnd = HwndFromSDLWindow(_wnd);
#endif
        return *this;
    }

    bool operator==(std::nullptr_t) const
    {
        return _wnd == nullptr;
    }

    bool operator!=(std::nullptr_t) const
    {
        return _wnd != nullptr;
    }
#endif // KAUAI_SDL

#ifdef WIN
    // Get Win32 HWND
    operator HWND() const
    {
        return _hwnd;
    }

    KWND &operator=(HWND hwnd)
    {
        _hwnd = hwnd;
        return *this;
    }

    KWND &operator=(std::nullptr_t)
    {
        _hwnd = nullptr;
        return *this;
    }

    bool operator==(std::nullptr_t) const
    {
        return _hwnd == nullptr;
    }

    bool operator!=(std::nullptr_t) const
    {
        return _hwnd != nullptr;
    }
#endif

  private:
#ifdef WIN
    HWND _hwnd = nullptr;
#endif // WIN

#ifdef KAUAI_SDL
    SDL_Window *_wnd = nullptr;

#ifdef WIN
    static HWND HwndFromSDLWindow(SDL_Window *wnd)
    {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(wnd, &wmInfo))
        {
            return wmInfo.info.win.window;
        }
        else
        {
            return nullptr;
        }
    }
#endif // WIN

#endif // KAUAI_SDL
};

#endif // KAUAI_KWND_H
