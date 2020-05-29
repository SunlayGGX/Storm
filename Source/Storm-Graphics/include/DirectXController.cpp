#include "DirectXController.h"
#include "MemoryHelper.h"



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
	Storm::throwIfFailed(D3D11CreateDevice(
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

	Storm::throwIfFailed(d3dDevice.As(&_device));
	Storm::throwIfFailed(d3dDeviceContext.As(&_deviceContext));

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
	Storm::throwIfFailed(_swapChain->Present(1, 0));
	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

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
	Storm::throwIfFailed(_device.As(&dxgiDevice));

	Storm::throwIfFailed(dxgiDevice->SetMaximumFrameLatency(1));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	Storm::throwIfFailed(
		dxgiDevice->GetAdapter(&dxgiAdapter)
	);

	ComPtr<IDXGIFactory2> dxgiFactory;
	Storm::throwIfFailed(
		dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
	);

	// Finally, create the swap chain.
	Storm::throwIfFailed(dxgiFactory->CreateSwapChainForHwnd(_device.Get(), hwnd, &swapChainDesc, &fullScreenSwapChainDesc, nullptr, &_swapChain));

	// Once the swap chain is created, create a render target view. This will allow Direct3D to render graphics to the window.
	ComPtr<ID3D11Texture2D> backBuffer;
	Storm::throwIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	Storm::throwIfFailed(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &_renderTarget));


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

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	Storm::ZeroMemories(depthBufferDesc);

	depthBufferDesc.Width = static_cast<UINT>(viewport.Width);
	depthBufferDesc.Height = static_cast<UINT>(viewport.Height);
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	Storm::throwIfFailed(_device->CreateTexture2D(&depthBufferDesc, nullptr, &_depthStencilBuffer));

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	Storm::ZeroMemories(depthStencilDesc);

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;

	Storm::throwIfFailed(_device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilState));
	_deviceContext->OMSetDepthStencilState(_depthStencilState.Get(), 1);


	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	Storm::ZeroMemories(depthStencilViewDesc);

	depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	Storm::throwIfFailed(_device->CreateDepthStencilView(_depthStencilBuffer.Get(), &depthStencilViewDesc, &_depthStencilView));


	D3D11_RASTERIZER_DESC rasterStateDesc;
	Storm::ZeroMemories(rasterStateDesc);

	rasterStateDesc.AntialiasedLineEnable = false;
	rasterStateDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rasterStateDesc.DepthBias = 0;
	rasterStateDesc.DepthBiasClamp = 0.0f;
	rasterStateDesc.DepthClipEnable = true;
	rasterStateDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterStateDesc.FrontCounterClockwise = false;
	rasterStateDesc.MultisampleEnable = false;
	rasterStateDesc.ScissorEnable = false;
	rasterStateDesc.SlopeScaledDepthBias = 0.0f;

	Storm::throwIfFailed(_device->CreateRasterizerState(&rasterStateDesc, &_rasterState));

	_deviceContext->RSSetState(_rasterState.Get());

	_deviceContext->RSSetViewports(1, &viewport);
}
