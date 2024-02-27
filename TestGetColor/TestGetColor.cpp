#include "pch.h"
#include "SimpleCapture.h"

int GetWindowColor(HWND hwnd,int x,int y);

int main()
{
    auto hwnd = FindWindow(nullptr, TEXT("Age of Empires IV "));
    if (hwnd == nullptr)
    {
        MessageBox(nullptr, TEXT("Not Find hwnd"), TEXT("WARN"), 0);
        return -1;
    }

	return 0;
}

namespace util
{
    using namespace robmikh::common::desktop;
    using namespace robmikh::common::uwp;
}



int GetWindowColor(HWND hwnd, int x, int y)
{
    

	return 0;
}
