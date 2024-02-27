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

    auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
    d3dDevice->GetImmediateContext(m_d3dContext.put());

    m_swapChain = util::CreateDXGISwapChain(d3dDevice, static_cast<uint32_t>(m_item.Size().Width), static_cast<uint32_t>(m_item.Size().Height),
        static_cast<DXGI_FORMAT>(m_pixelFormat), 2);

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

winrt::ICompositionSurface SimpleCapture::CreateSurface(winrt::Compositor const& compositor)
{
    CheckClosed();
    return util::CreateCompositionSurfaceForSwapChain(compositor, m_swapChain.get());
}

unsigned int SimpleCapture::GetColorFromBackBuffer(int x, int y, uint32_t subresource)
{
    if (m_backBuffer != nullptr)
    {
        auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
        winrt::com_ptr<ID3D11DeviceContext> d3dContext;
        d3dDevice->GetImmediateContext(d3dContext.put());

        auto texture = util::CopyD3DTexture(d3dDevice, m_backBuffer, true);

        auto stagingTexture = util::PrepareStagingTexture(d3dDevice, texture);

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
        
        d3dContext->Unmap(stagingTexture.get(), 0);
        return res;
    }
    return 2;
}

void SimpleCapture::Close()
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {
        m_session.Close();
        m_framePool.Close();

        m_swapChain = nullptr;
        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }
}

void SimpleCapture::ResizeSwapChain()
{
    winrt::check_hresult(m_swapChain->ResizeBuffers(2, static_cast<uint32_t>(m_lastSize.Width), static_cast<uint32_t>(m_lastSize.Height),
        static_cast<DXGI_FORMAT>(m_pixelFormat), 0));
}

bool SimpleCapture::TryResizeSwapChain(winrt::Direct3D11CaptureFrame const& frame)
{
    auto const contentSize = frame.ContentSize();
    if ((contentSize.Width != m_lastSize.Width) ||
        (contentSize.Height != m_lastSize.Height))
    {
        // The thing we have been capturing has changed size, resize the swap chain to match.
        m_lastSize = contentSize;
        ResizeSwapChain();
        return true;
    }
    return false;
}

bool SimpleCapture::TryUpdatePixelFormat()
{
    auto newFormat = m_pixelFormatUpdate.exchange(std::nullopt);
    if (newFormat.has_value())
    {
        auto pixelFormat = newFormat.value();
        if (pixelFormat != m_pixelFormat)
        {
            m_pixelFormat = pixelFormat;
            ResizeSwapChain();
            return true;
        }
    }
    return false;
}

void SimpleCapture::OnFrameArrived(winrt::Direct3D11CaptureFramePool const& sender, winrt::IInspectable const&)
{
    auto swapChainResizedToFrame = false;

    {
        auto frame = sender.TryGetNextFrame();
        swapChainResizedToFrame = TryResizeSwapChain(frame);

       
        winrt::check_hresult(m_swapChain->GetBuffer(0, winrt::guid_of<ID3D11Texture2D>(), m_backBuffer.put_void()));
        auto surfaceTexture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
        // copy surfaceTexture to backBuffer
        m_d3dContext->CopyResource(m_backBuffer.get(), surfaceTexture.get());
    }
    
    //DXGI_PRESENT_PARAMETERS presentParameters{};
    //m_swapChain->Present1(1, 0, &presentParameters);

    swapChainResizedToFrame = swapChainResizedToFrame || TryUpdatePixelFormat();

    if (swapChainResizedToFrame)
    {
        m_framePool.Recreate(m_device, m_pixelFormat, 2, m_lastSize);
    }
}
