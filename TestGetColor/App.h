#pragma once
#include "SimpleCapture.h"

class App
{
public:
    App();
    ~App() {}

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem TryStartCaptureFromWindowHandle(HWND hwnd);
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem TryStartCaptureFromMonitorHandle(HMONITOR hmon);
    unsigned int GetColor(int x,int y);
    unsigned int ErrorCode(unsigned char e);
    winrt::Windows::Graphics::DirectX::DirectXPixelFormat PixelFormat() { return m_pixelFormat; }
    void PixelFormat(winrt::Windows::Graphics::DirectX::DirectXPixelFormat pixelFormat);

    bool IsCursorEnabled();
    void IsCursorEnabled(bool value);
    bool IsBorderRequired();
    winrt::fire_and_forget IsBorderRequired(bool value);

    void StopCapture();

private:
    void StartCaptureFromItem(winrt::Windows::Graphics::Capture::GraphicsCaptureItem item);

private:
    
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
    std::unique_ptr<SimpleCapture> m_capture{ nullptr };
    winrt::Windows::Graphics::DirectX::DirectXPixelFormat m_pixelFormat = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;
};