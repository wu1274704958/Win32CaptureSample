#include "pch.h"
#include "SimpleCapture.h"
#include "App.h"

namespace util
{
    using namespace robmikh::common::desktop;
}
int GetWindowColor(HWND hwnd,int x,int y);

int main()
{
    auto hwnd = FindWindow(nullptr, TEXT("Age of Empires IV -dev"));
    if (hwnd == nullptr)
    {
        MessageBox(nullptr, TEXT("Not Find hwnd"), TEXT("WARN"), 0);
        return -1;
    }
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    //auto controller = util::CreateDispatcherQueueControllerForCurrentThread();

    auto app = std::make_unique<App>();
    app->TryStartCaptureFromWindowHandle(hwnd);
    app->GetCapture()->IsBorderRequired(false);
    
    int a = 0;
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (a >= 3 && app->GetColor(0, 0) != 2)
        {
            a = 0;
            auto color = app->GetColor(19, 0);
            auto b = (color >> 24) & 255;
            auto g = (color >> 16) & 255;
            auto r = (color >> 8) & 255;
            printf("r=%d g=%d b=%d\n",r,g,b);
            auto c = getchar();
            if (c == 'x')
                break;
        }
        ++a;
    }
    
    app->StopCapture();

    return 0;//util::ShutdownDispatcherQueueControllerAndWait(controller, static_cast<int>(msg.wParam));
}





int GetWindowColor(HWND hwnd, int x, int y)
{
    

	return 0;
}
