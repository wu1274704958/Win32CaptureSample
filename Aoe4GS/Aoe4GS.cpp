#include "pch.h"
#include "SimpleCapture.h"
#include "App.h"
#include "Aoe4GS.h"

namespace util
{
    using namespace robmikh::common::desktop;
}

std::unique_ptr<App> APP;

int Init(HWND hwnd)
{
    if (hwnd == NULL)
        return -1;
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    APP = std::make_unique<App>();
    APP->TryStartCaptureFromWindowHandle(hwnd);
    APP->GetCapture()->IsBorderRequired(false);
    return 0;
}

unsigned int GetColor(int x, int y)
{
    if (APP == nullptr)
        return 3;
    int a = 0;
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (a >= 3 && APP->GetColor(0, 0) != 2)
        {
            a = 0;
            auto color = APP->GetColor(x, y);
            return color;
        }
        ++a;
    }
}

void Stop()
{
    if (APP)
        APP->StopCapture();
}


extern "C" {
    int AGS_Init(HWND hwnd)
    {
        return Init(hwnd);
    }
    unsigned int AGS_GetColor(int x, int y)
    {
        return GetColor(x, y);
    }
    void AGS_Stop()
    {
        Stop();
    }
}