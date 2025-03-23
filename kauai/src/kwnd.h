/***************************************************************************
    Author: Ben Stone
    Project: Kauai

    Cross-platform window handle class

***************************************************************************/

#ifndef KAUAI_KWND_H
#define KAUAI_KWND_H

#define kwndNil nullptr

// Window handle class
class KWND
{
  public:
    KWND() = default;

    KWND(std::nullptr_t) : _hwnd(nullptr)
    {
    }

    KWND(HWND hwnd) : _hwnd(hwnd)
    {
    }

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

  private:
    HWND _hwnd = nullptr;
};

#endif // KAUAI_KWND_H
