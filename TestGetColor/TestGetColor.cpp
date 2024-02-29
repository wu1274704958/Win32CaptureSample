#include "pch.h"
#include "CaptureSnapshot.h"

namespace winrt
{
	using namespace Windows;
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
	using namespace robmikh::common::desktop;
	using namespace robmikh::common::uwp;
}


unsigned int GetWindowColor(winrt::IDirect3DDevice const& device,HWND hwnd, int x, int y);

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

	auto d3dDevice = util::CreateD3DDevice();
	auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
	auto device = CreateDirect3DDevice(dxgiDevice.get());

	std::atomic<bool> isRun = true;
	auto th = std::thread([&device, &isRun,&hwnd]() {
		while (isRun)
		{
			auto start = GetTickCount();
			auto color = GetWindowColor(device,hwnd,3, 33);
			auto b = (color >> 24) & 255;
			auto g = (color >> 16) & 255;
			auto r = (color >> 8) & 255;
			auto tick = GetTickCount() - start;
			printf("r=%d g=%d b=%d tick = %d\n", r, g, b,tick);
			Sleep(100);
		}
		});

	getchar();
	isRun = false;
	th.join();

	return 0;//util::ShutdownDispatcherQueueControllerAndWait(controller, static_cast<int>(msg.wParam));
}





unsigned int GetWindowColor(winrt::IDirect3DDevice const& device,HWND hwnd, int x, int y)
{
	winrt::GraphicsCaptureItem item = {nullptr};
	try {
		item = util::CreateCaptureItemForWindow(hwnd);
	}
	catch (winrt::hresult_error const& error)
	{
		return 1;
	}
	auto size = item.Size();
	if (x >= size.Width || x >= size.Height)
		return 2;
	
	auto textureFuture = CaptureSnapshot::TakeAsync(device, item);

	while (!textureFuture._Is_ready()){}

	auto texture = textureFuture.get();

	auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(device);
	winrt::com_ptr<ID3D11DeviceContext> d3dContext;
	d3dDevice->GetImmediateContext(d3dContext.put());

	auto stagingTexture = util::PrepareStagingTexture(d3dDevice, texture);

	D3D11_TEXTURE2D_DESC desc = {};
	stagingTexture->GetDesc(&desc);
	auto bytesPerPixel = util::GetBytesPerPixel(desc.Format);

	// Copy the bits
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	winrt::check_hresult(d3dContext->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mapped));

	auto bytesStride = static_cast<size_t>(desc.Width) * bytesPerPixel;

	unsigned int res = 0;
	auto source = reinterpret_cast<byte*>(mapped.pData);

	auto target = (y * mapped.RowPitch) + (x * bytesPerPixel);
	res |= (source[target] << 24);
	res |= (source[target + 1] << 16);
	res |= (source[target + 2] << 8);

	d3dContext->Unmap(stagingTexture.get(), 0);
	return res;
}
