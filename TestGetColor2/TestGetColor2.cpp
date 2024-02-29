#include "pch.h"
#include "App.h"

#include <iostream>

int main()
{
	auto hwnd = FindWindow(nullptr, TEXT("Age of Empires IV -dev"));
	//auto hwnd = FindWindow(nullptr, TEXT("无标题 - 画图"));
	if (hwnd == nullptr)
	{
		MessageBox(nullptr, TEXT("Not Find hwnd"), TEXT("WARN"), 0);
		return -1;
	}
	winrt::init_apartment(winrt::apartment_type::single_threaded);

	auto app = std::make_unique<App>();
	app->TryStartCaptureFromWindowHandle(hwnd);
	app->GetCapture()->IsCursorEnabled(false);
	app->GetCapture()->IsBorderRequired(false);

	std::atomic<bool> isRun = true;
	auto th = std::thread([&isRun, &app]() {
		while (isRun)
		{
			auto start = ::GetTickCount();
			auto color = app->GetCapture()->GetColorFromBackBuffer(3, 33);
			if (color == 2) continue;
			auto b = (color >> 24) & 255;
			auto g = (color >> 16) & 255;
			auto r = (color >> 8) & 255;
			auto tick = ::GetTickCount() - start;
			printf("r=%d g=%d b=%d tick = %d\n", r, g, b, tick);
			Sleep(50);
			auto res = app->GetCapture()->SaveSnapshoot();
			res.get();
			isRun = false;
			break;
		}
	});
	
	MSG msg = {};
	while (isRun && GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	app->StopCapture();
	return 0;
}