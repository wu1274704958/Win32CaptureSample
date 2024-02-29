#include "pch.h"
#include "SimpleCapture.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::DirectX::Direct3D11;
    using namespace Windows::System;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Storage;
    using namespace Windows::Foundation;
    using namespace Windows::Storage::Pickers;
    using namespace Windows::System;
    using namespace Windows::UI::Popups;
}

namespace util
{
    using namespace robmikh::common::uwp;
}

SimpleCapture::SimpleCapture(winrt::IDirect3DDevice const& device, winrt::GraphicsCaptureItem const& item, winrt::DirectXPixelFormat pixelFormat)
{
    m_item = item;
    m_device = device;
    m_pixelFormat = pixelFormat;



    // Creating our frame pool with 'Create' instead of 'CreateFreeThreaded'
    // means that the frame pool's FrameArrived event is called on the thread
    // the frame pool was created on. This also means that the creating thread
    // must have a DispatcherQueue. If you use this method, it's best not to do
    // it on the UI thread. 
    m_framePool = winrt::Direct3D11CaptureFramePool::Create(m_device, m_pixelFormat, 2, m_item.Size());
    m_session = m_framePool.CreateCaptureSession(m_item);
    m_lastSize = m_item.Size();
    m_framePool.FrameArrived({ this, &SimpleCapture::OnFrameArrived });
}

void SimpleCapture::StartCapture()
{
    CheckClosed();
    m_session.StartCapture();
}


unsigned int SimpleCapture::GetColorFromBackBuffer(int x, int y, uint32_t subresource)
{
    if (x >= m_lastSize.Width || y >= m_lastSize.Height)
        return 3;
    std::lock_guard<std::mutex> guard(m_textureMutex);
    if (m_lastTexture != nullptr)
    {
        auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
        winrt::com_ptr<ID3D11DeviceContext> d3dContext;
        d3dDevice->GetImmediateContext(d3dContext.put());

        auto stagingTexture = util::PrepareStagingTexture(d3dDevice, m_lastTexture);

        D3D11_TEXTURE2D_DESC desc = {};
        stagingTexture->GetDesc(&desc);
        auto bytesPerPixel = util::GetBytesPerPixel(desc.Format);

        // Copy the bits
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        winrt::check_hresult(d3dContext->Map(stagingTexture.get(), subresource, D3D11_MAP_READ, 0, &mapped));

        auto bytesStride = static_cast<size_t>(desc.Width) * bytesPerPixel;

        unsigned int res = 0;
        auto source = reinterpret_cast<byte*>(mapped.pData);
        
        auto target = (y * bytesStride) + (x * bytesPerPixel);
        res |= (source[target] << 24);
        res |= (source[target + 1] << 16);
        res |= (source[target + 2] << 8);
        res |= (source[target + 3]);
        
        d3dContext->Unmap(stagingTexture.get(), 0);
        return res;
    }
    return 2;
}


winrt::IAsyncAction SimpleCapture::SaveSnapshoot()
{
   
    if (m_lastTexture != nullptr)
    {
        m_textureMutex.lock();
        auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
        winrt::com_ptr<ID3D11DeviceContext> d3dContext;
        d3dDevice->GetImmediateContext(d3dContext.put());

        //auto stagingTexture = util::PrepareStagingTexture(d3dDevice, m_lastTexture);

        D3D11_TEXTURE2D_DESC desc = {};
        m_lastTexture->GetDesc(&desc);

        auto bytesPerPixel = util::GetBytesPerPixel(desc.Format);
        auto arr = util::CopyBytesFromTexture(m_lastTexture);
        m_textureMutex.unlock();
        // Copy the bits
        auto file = co_await winrt::StorageFile::GetFileFromPathAsync(winrt::hstring(TEXT("D:\\a.png")));
        if (file != nullptr)
        {
            auto stream = co_await file.OpenAsync(winrt::FileAccessMode::ReadWrite);
            auto encoder = co_await winrt::BitmapEncoder::CreateAsync(winrt::BitmapEncoder::PngEncoderId(), stream);
            encoder.SetPixelData(
                winrt::BitmapPixelFormat::Bgra8,
                winrt::BitmapAlphaMode::Premultiplied,
                desc.Width,
                desc.Height,
                1.0,
                1.0,
                winrt::array_view<uint8_t>(arr.data(), arr.size()));
            co_await encoder.FlushAsync();
        }
    }
}

void SimpleCapture::Close()
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {
        m_session.Close();
        m_framePool.Close();

        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }
}


void SimpleCapture::OnFrameArrived(winrt::Direct3D11CaptureFramePool const& sender, winrt::IInspectable const&)
{
    {
        std::lock_guard<std::mutex> guard(m_textureMutex);
        auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
        winrt::com_ptr<ID3D11DeviceContext> d3dContext;
        d3dDevice->GetImmediateContext(d3dContext.put());

        auto frame = sender.TryGetNextFrame();
        m_lastSize = frame.ContentSize();

        auto texture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
        m_lastTexture = util::CopyD3DTexture(d3dDevice, texture, true);
    }
}
