#include "DirectXController.h"

#include "IRenderedElement.h"
#include "GraphicRigidBody.h"

#include "GraphicsAction.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "MemoryHelper.h"
#include "DirectXHardwareInfo.h"



void Storm::DirectXController::initialize(HWND hwnd)
{
	this->internalCreateDXDevices(hwnd);
	this->internalCreateRenderView();
	this->internalInitializeDepthBuffer();
	this->internalConfigureStates();
	this->internalInitializeDebugDevice();
	this->internalConfigureImmediateContextToDefault();
	this->internalInitializeViewPort();
}

void Storm::DirectXController::cleanUp()
{
	_alphaBlendEnable.Reset();
	_alphaBlendDisable.Reset();

	_zBufferEnabled.Reset();
	_zBufferDisabled.Reset();

	_rasterAlphaBlendEnabled.Reset();
	_rasterAlphaBlendDisabled.Reset();

	_solidCullBackRS.Reset();
	_solidCullNoneRS.Reset();
	_wireframeCullNoneRS.Reset();

	_zBufferEnable.Reset();
	_zBufferDisable.Reset();

	_depthTexture.Reset();
	_depthStencilView.Reset();
	
	_renderTargetView.Reset();

	_swapChain.Reset();
	_immediateContext.Reset();
	_device.Reset();

#if defined(DEBUG) || defined(_DEBUG)
	if (_debugDevice != nullptr)
	{
		Storm::throwIfFailed(_debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_IGNORE_INTERNAL));
		_debugDevice.Reset();
	}
#endif
}

void Storm::DirectXController::clearView(const float(&clearColor)[4])
{
	_immediateContext->ClearRenderTargetView(_renderTargetView.Get(), clearColor);
	_immediateContext->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.f, 0);
}

void Storm::DirectXController::unbindTargetView()
{
	ComPtr<ID3D11RenderTargetView> voidTargetView;
	_immediateContext->OMSetRenderTargets(1, &voidTargetView, nullptr);
}

void Storm::DirectXController::initView()
{
	ID3D11RenderTargetView*const tmpRenderTargetView = _renderTargetView.Get();
	_immediateContext->OMSetRenderTargets(1, &tmpRenderTargetView, _depthStencilView.Get());
}

void Storm::DirectXController::presentToDisplay()
{
	_swapChain->Present(0, 0);
}

void Storm::DirectXController::reportLiveObject()
{
#ifdef _DEBUG_GRAPHIC
	if (m_DXDebugDevice != nullptr)
	{
		DXTry(m_DXDebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL));
	}
#endif // _DEBUG
}

const ComPtr<ID3D11Device>& Storm::DirectXController::getDirectXDevice() const noexcept
{
	return _device;
}

const ComPtr<ID3D11DeviceContext>& Storm::DirectXController::getImmediateContext() const noexcept
{
	return _immediateContext;
}

void Storm::DirectXController::renderElements(const Storm::Camera &currentCamera, const std::vector<std::unique_ptr<Storm::IRenderedElement>> &renderedElementArrays, const std::map<unsigned int, std::unique_ptr<Storm::GraphicRigidBody>> &rbElementArrays) const
{
	for (const auto &renderedElement : renderedElementArrays)
	{
		renderedElement->render(_device, _immediateContext, currentCamera);
	}

	for (const auto &rbElementPair : rbElementArrays)
	{
		rbElementPair.second->render(_device, _immediateContext, currentCamera);
	}
}

float Storm::DirectXController::getViewportWidth() const noexcept
{
	return _viewportWidth;
}

float Storm::DirectXController::getViewportHeight() const noexcept
{
	return _viewportHeight;
}

void Storm::DirectXController::executeAction(Storm::GraphicsAction action)
{
	switch (action)
	{
	case Storm::GraphicsAction::ShowWireframe:
		this->setWireFrameState();
		break;

	case Storm::GraphicsAction::ShowSolidFrameWithCulling:
		this->setSolidCullBackState();
		break;
	
	case Storm::GraphicsAction::ShowSolidFrameNoCulling:
		this->setSolidCullNoneState();
		break;

	case Storm::GraphicsAction::EnableZBuffer:
		this->setEnableZBuffer(true);
		break;

	case Storm::GraphicsAction::DisableZBuffer:
		this->setEnableZBuffer(false);
		break;

	case Storm::GraphicsAction::EnableBlendAlpha:
		this->setEnableBlendAlpha(true);
		break;

	case Storm::GraphicsAction::DisableBlendAlpha:
		this->setEnableBlendAlpha(false);
		break;

	default:
		LOG_ERROR << "Graphics action " << std::to_string(static_cast<int>(action)) << " is unknown!";
	}
}

void Storm::DirectXController::setWireFrameState()
{
	_immediateContext->RSSetState(_wireframeCullNoneRS.Get());
}

void Storm::DirectXController::setSolidCullNoneState()
{
	_immediateContext->RSSetState(_solidCullNoneRS.Get());
}

void Storm::DirectXController::setSolidCullBackState()
{
	_immediateContext->RSSetState(_solidCullBackRS.Get());
}

void Storm::DirectXController::setEnableZBuffer(bool enable)
{
	_immediateContext->OMSetDepthStencilState(enable ? _zBufferEnable.Get() : _zBufferDisable.Get(), 0);
}

void Storm::DirectXController::setEnableBlendAlpha(bool enable)
{
	constexpr float k_blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
	_immediateContext->OMSetBlendState(enable ? _alphaBlendEnable.Get() : _alphaBlendDisable.Get(), k_blendFactor, 0xFFFFFFFF);
}

void Storm::DirectXController::internalCreateDXDevices(HWND hwnd)
{
	enum
	{
		k_wantedFPS = 60
	};

	UINT creationFlags = 0;

#if defined(_DEBUG) || defined(DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

	constexpr const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0
	};

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	Storm::ZeroMemories(swapChainDesc);

	{
		Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		DXGI_MODE_DESC descModeWanted;
		descModeWanted.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		descModeWanted.Height = configMgr.getWantedScreenHeight();
		descModeWanted.Width = configMgr.getWantedScreenWidth();
		descModeWanted.RefreshRate.Numerator = k_wantedFPS;
		descModeWanted.RefreshRate.Denominator = 1;
		descModeWanted.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		descModeWanted.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;

		Storm::DirectXHardwareInfo infos{ descModeWanted };

		swapChainDesc.BufferDesc.Format = infos._mode.Format;

		swapChainDesc.BufferDesc.Width = infos._mode.Width;
		swapChainDesc.BufferDesc.Height = infos._mode.Height;

		_viewportWidth = static_cast<float>(swapChainDesc.BufferDesc.Width);
		_viewportHeight = static_cast<float>(swapChainDesc.BufferDesc.Height);
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags = 0;

	swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferCount = 1;

	swapChainDesc.BufferDesc.RefreshRate.Numerator = k_wantedFPS;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = hwnd;

	swapChainDesc.Windowed = true;

	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	{
		RECT rcClient;
		RECT rcWindow;
		GetClientRect(hwnd, &rcClient);
		GetWindowRect(hwnd, &rcWindow);

		POINT ptDiff{ (rcWindow.right - rcWindow.left) - rcClient.right, (rcWindow.bottom - rcWindow.top) - rcClient.bottom };

		MoveWindow(
			hwnd,
			rcWindow.left,
			rcWindow.top,
			swapChainDesc.BufferDesc.Width + ptDiff.x,
			swapChainDesc.BufferDesc.Height + ptDiff.y,
			TRUE
		);
	}

	Storm::throwIfFailed(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&_swapChain,
		&_device,
		nullptr,
		&_immediateContext
	));
}

void Storm::DirectXController::internalCreateRenderView()
{
	ComPtr<ID3D11Texture2D> backBuffer;

	Storm::throwIfFailed(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
	Storm::throwIfFailed(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &_renderTargetView));
}

void Storm::DirectXController::internalInitializeDepthBuffer()
{
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC descStencilView;
	Storm::ZeroMemories(depthTextureDesc, descStencilView);

	depthTextureDesc.Width = static_cast<UINT>(_viewportWidth);
	depthTextureDesc.Height = static_cast<UINT>(_viewportHeight);
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	depthTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.CPUAccessFlags = 0;

	Storm::throwIfFailed(_device->CreateTexture2D(&depthTextureDesc, nullptr, &_depthTexture));

	descStencilView.Format = depthTextureDesc.Format;
	descStencilView.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
	descStencilView.Texture2D.MipSlice = 0;

	Storm::throwIfFailed(_device->CreateDepthStencilView(_depthTexture.Get(), &descStencilView, &_depthStencilView));
}

void Storm::DirectXController::internalConfigureStates()
{
	{ 
		//Blend State creation
		D3D11_BLEND_DESC blendDesc;
		Storm::ZeroMemories(blendDesc);

		blendDesc.IndependentBlendEnable = FALSE;

		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		Storm::throwIfFailed(_device->CreateBlendState(&blendDesc, &_alphaBlendEnable));

		blendDesc.RenderTarget[0].BlendEnable = FALSE;

		Storm::throwIfFailed(_device->CreateBlendState(&blendDesc, &_alphaBlendDisable));
	}

	this->setEnableBlendAlpha(true);

	{ 
		//Rasterizer state creation
		D3D11_RASTERIZER_DESC rsDesc;
		Storm::ZeroMemories(rsDesc);

		rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
		rsDesc.FrontCounterClockwise = FALSE;
		rsDesc.DepthClipEnable = TRUE;
		Storm::throwIfFailed(_device->CreateRasterizerState(&rsDesc, &_solidCullBackRS));

		rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;

		Storm::throwIfFailed(_device->CreateRasterizerState(&rsDesc, &_solidCullNoneRS));

		rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
		rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

		Storm::throwIfFailed(_device->CreateRasterizerState(&rsDesc, &_wireframeCullNoneRS));
	}

	this->setSolidCullBackState();

	{
		// Z-Buffer State creation
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		Storm::ZeroMemories(depthStencilDesc);

		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = FALSE;

		Storm::throwIfFailed(_device->CreateDepthStencilState(&depthStencilDesc, &_zBufferDisable));

		depthStencilDesc.DepthEnable = TRUE;

		Storm::throwIfFailed(_device->CreateDepthStencilState(&depthStencilDesc, &_zBufferEnable));
	}

	this->setEnableZBuffer(true);
}

void Storm::DirectXController::internalInitializeDebugDevice()
{
#if defined(_DEBUG) || defined(DEBUG)
	try
	{
		ID3D11Debug* debugDeviceTmp = nullptr;
		Storm::throwIfFailed(_device->QueryInterface(&debugDeviceTmp));
		_debugDevice.Attach(debugDeviceTmp);
	}
	catch (const std::exception &resultException)
	{
		LOG_DEBUG_ERROR << "Error : Failed to initialize debug device. Error : " << resultException.what();
	}
#endif
}

void Storm::DirectXController::internalConfigureImmediateContextToDefault()
{
	_immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Storm::DirectXController::internalInitializeViewPort()
{
	D3D11_VIEWPORT viewPort;

	viewPort.Width = _viewportWidth;
	viewPort.Height = _viewportHeight;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	_immediateContext->RSSetViewports(1, &viewPort);
}
