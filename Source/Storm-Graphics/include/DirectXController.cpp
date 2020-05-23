#include "DirectXController.h"
#include "MemoryHelper.h"
#include "ThrowException.h"

#include <filesystem>
#include <comdef.h>

namespace
{
	void throwIfFailedImpl(HRESULT res, const std::string &callExpression)
	{
		if (!SUCCEEDED(res))
		{
			Storm::throwException<std::exception>(callExpression + " failed! Error was " + std::filesystem::path{ _com_error{ res }.ErrorMessage() }.string());
		}
	}

#define throwIfFailed(expression) throwIfFailedImpl(expression, #expression)
}

void Storm::DirectXController::initialize(HWND hwnd)
{
	UINT creationFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG) || defined(DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

	constexpr const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1
	};

	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	throwIfFailed(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&d3dDevice,
		nullptr,
		&d3dDeviceContext
	));

	throwIfFailed(d3dDevice.As(&_device));
	throwIfFailed(d3dDeviceContext.As(&_deviceContext));

	this->setupSwapChain(hwnd);
}

void Storm::DirectXController::cleanUp()
{

}

void Storm::DirectXController::clearRenderTarget(const float(&clearColor)[4])
{
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
}

void Storm::DirectXController::drawRenderTarget()
{
	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

	throwIfFailed(_swapChain->Present(1, 0));
}

void Storm::DirectXController::setupSwapChain(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenSwapChainDesc;
	Storm::ZeroMemories(swapChainDesc, fullScreenSwapChainDesc);

	swapChainDesc.Stereo = false;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_NONE;
	swapChainDesc.Flags = 0;

	swapChainDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferCount = 2;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	fullScreenSwapChainDesc.RefreshRate.Numerator = 60;
	fullScreenSwapChainDesc.RefreshRate.Denominator = 1;

	fullScreenSwapChainDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	fullScreenSwapChainDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullScreenSwapChainDesc.Windowed = true;

	ComPtr<IDXGIDevice2> dxgiDevice;
	throwIfFailed(_device.As(&dxgiDevice));

	throwIfFailed(dxgiDevice->SetMaximumFrameLatency(1));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	throwIfFailed(
		dxgiDevice->GetAdapter(&dxgiAdapter)
	);

	ComPtr<IDXGIFactory2> dxgiFactory;
	throwIfFailed(
		dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
	);

	// Finally, create the swap chain.
	throwIfFailed(dxgiFactory->CreateSwapChainForHwnd(_device.Get(), hwnd, &swapChainDesc, &fullScreenSwapChainDesc, nullptr, &_swapChain));

	// Once the swap chain is created, create a render target view. This will allow Direct3D to render graphics to the window.
	ComPtr<ID3D11Texture2D> backBuffer;
	throwIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	throwIfFailed(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &_renderTarget));


	// After the render target view is created, specify that the viewport,
	// which describes what portion of the window to draw to, should cover
	// the entire window.

	D3D11_TEXTURE2D_DESC backBufferDesc;
	Storm::ZeroMemories(swapChainDesc, fullScreenSwapChainDesc);

	backBuffer->GetDesc(&backBufferDesc);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(backBufferDesc.Width);
	viewport.Height = static_cast<float>(backBufferDesc.Height);
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;

	_deviceContext->RSSetViewports(1, &viewport);
}

