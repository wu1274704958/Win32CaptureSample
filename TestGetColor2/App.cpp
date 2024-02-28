#include "pch.h"
#include "App.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Storage;
    using namespace Windows::Storage::Pickers;
    using namespace Windows::System;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::UI::Popups;
}

namespace util
{
    using namespace robmikh::common::desktop;
    using namespace robmikh::common::uwp;
}

App::App()
{
    auto d3dDevice = util::CreateD3DDevice();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    m_device = CreateDirect3DDevice(dxgiDevice.get());
}

winrt::GraphicsCaptureItem App::TryStartCaptureFromWindowHandle(HWND hwnd)
{
    winrt::GraphicsCaptureItem item{ nullptr };
    try
    {
        item = util::CreateCaptureItemForWindow(hwnd);
        StartCaptureFromItem(item);
    }
    catch (winrt::hresult_error const& error)
    {
        MessageBoxW(nullptr,
            error.message().c_str(),
            L"Win32CaptureSample",
            MB_OK | MB_ICONERROR);
    }
    return item;
}

winrt::GraphicsCaptureItem App::TryStartCaptureFromMonitorHandle(HMONITOR hmon)
{
    winrt::GraphicsCaptureItem item{ nullptr };
    try
    {
        item = util::CreateCaptureItemForMonitor(hmon);
        StartCaptureFromItem(item);
    }
    catch (winrt::hresult_error const& error)
    {
        MessageBoxW(nullptr,
            error.message().c_str(),
            L"Win32CaptureSample",
            MB_OK | MB_ICONERROR);
    }
    return item;
}

unsigned int App::GetColor(int x, int y)
{
    if (m_capture == nullptr)
        return ErrorCode(1);
    return m_capture->GetColorFromBackBuffer(x,y);
}

unsigned int App::ErrorCode(unsigned char e)
{
    return e;
}





void App::StartCaptureFromItem(winrt::GraphicsCaptureItem item)
{
    m_capture = std::make_unique<SimpleCapture>(m_device, item, m_pixelFormat);

    m_capture->StartCapture();
}

void App::StopCapture()
{
    if (m_capture)
    {
        m_capture->Close();
        m_capture = nullptr;
    }
}

bool App::IsCursorEnabled()
{
    if (m_capture != nullptr)
    {
        return m_capture->IsCursorEnabled();
    }
    return false;
}

void App::IsCursorEnabled(bool value)
{
    if (m_capture != nullptr)
    {
        m_capture->IsCursorEnabled(value);
    }
}

void App::PixelFormat(winrt::DirectXPixelFormat pixelFormat)
{
    m_pixelFormat = pixelFormat;
    if (m_capture)
    {
        m_capture->SetPixelFormat(pixelFormat);
    }
}


bool App::IsBorderRequired()
{
    if (m_capture != nullptr)
    {
        return m_capture->IsBorderRequired();
    }
    return false;
}

winrt::fire_and_forget App::IsBorderRequired(bool value)
{
    if (m_capture != nullptr)
    {
        // Even if the user or system policy denies access, it's
        // still safe to set the IsBorderRequired property. In the
        // event that the policy changes, the property will be honored.
        auto ignored = co_await winrt::GraphicsCaptureAccess::RequestAccessAsync(winrt::GraphicsCaptureAccessKind::Borderless);
        m_capture->IsBorderRequired(value);
    }
}