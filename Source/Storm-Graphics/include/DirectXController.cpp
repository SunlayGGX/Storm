#include "DirectXController.h"

#include "IRenderedElement.h"
#include "GraphicRigidBody.h"
#include "GraphicParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "GraphicManager.h"

#include "MemoryHelper.h"
#include "DirectXHardwareInfo.h"

#include "RenderModeState.h"


namespace
{
	template<class Type>
	class MallocUPtr : public std::unique_ptr<Type, decltype(&::free)>
	{
	private:
		using BaseUPtr = std::unique_ptr<Type, decltype(&::free)>;

	public:
		MallocUPtr() : BaseUPtr{ nullptr, ::free } {};
		MallocUPtr(nullptr_t) : BaseUPtr{ nullptr, ::free } {};
		MallocUPtr(Type* ptr) : BaseUPtr{ ptr, ::free } {};
	};

	const std::string_view directXMessageCategoryPrettyPrinted(D3D11_MESSAGE_CATEGORY category)
	{
#define PRETTY_PRINT_CATEGORY_CASE(CategoryCaseLabel) case D3D11_MESSAGE_CATEGORY::D3D11_MESSAGE_CATEGORY_##CategoryCaseLabel: return "Category: " STRINGIFY(CategoryCaseLabel)

		switch (category)
		{
			PRETTY_PRINT_CATEGORY_CASE(APPLICATION_DEFINED);
			PRETTY_PRINT_CATEGORY_CASE(MISCELLANEOUS);
			PRETTY_PRINT_CATEGORY_CASE(INITIALIZATION);
			PRETTY_PRINT_CATEGORY_CASE(CLEANUP);
			PRETTY_PRINT_CATEGORY_CASE(COMPILATION);
			PRETTY_PRINT_CATEGORY_CASE(STATE_CREATION);
			PRETTY_PRINT_CATEGORY_CASE(STATE_SETTING);
			PRETTY_PRINT_CATEGORY_CASE(STATE_GETTING);
			PRETTY_PRINT_CATEGORY_CASE(RESOURCE_MANIPULATION);
			PRETTY_PRINT_CATEGORY_CASE(EXECUTION);
			PRETTY_PRINT_CATEGORY_CASE(SHADER);

		default: return "Unknown Category!";
		}

#undef PRETTY_PRINT_CATEGORY_CASE
	}

	void printParseDeviceMessage(const D3D11_MESSAGE &message)
	{
		const std::string_view prettyPrintedMessageCategory = directXMessageCategoryPrettyPrinted(message.Category);

		std::string finalMsg;
		finalMsg.reserve(message.DescriptionByteLength + prettyPrintedMessageCategory.size() + 38);

		finalMsg += "DirectX Device Message (ID: ";
		finalMsg += std::to_string(message.ID);
		finalMsg += "; ";
		finalMsg += prettyPrintedMessageCategory;
		finalMsg += ") : ";
		finalMsg.append(message.pDescription, message.DescriptionByteLength - 1);

		switch (message.Severity)
		{
		case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_CORRUPTION: LOG_FATAL << finalMsg; break;
		case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_ERROR: LOG_ERROR << finalMsg; break;
		case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_INFO: LOG_COMMENT << finalMsg; break;
		case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_WARNING: LOG_WARNING << finalMsg; break;
		case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_MESSAGE: LOG_DEBUG << finalMsg; break;

		default:
			assert(false && "Unhandled device message, it shouldn't happen unless DirectX API upgraded and this method wasn't modified to reflect the new changes.");
			break;
		}
	}

	class WriterAutoDrawer
	{
	public:
		WriterAutoDrawer(ID2D1RenderTarget* direct2DRT) :
			_direct2DRT{ direct2DRT }
		{
			direct2DRT->BeginDraw();
		}

		~WriterAutoDrawer()
		{
			_direct2DRT->EndDraw();
		}

	private:
		ID2D1RenderTarget* _direct2DRT;
	};
}

void Storm::DirectXController::initialize(HWND hwnd)
{
	this->internalCreateDXDevices(hwnd);
	this->internalCreateRenderView();
	this->internalInitializeDepthBuffer();
	this->internalConfigureStates();
	this->internalInitializeDebugDevice();
	this->internalConfigureImmediateContextToDefault();
	this->internalInitializeViewPort();

	this->internalCreateDirect2DDevices(hwnd);
	this->internalCreateDirectWrite();

	_logDeviceMessage = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getShouldLogGraphicDeviceMessage();
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

	// A last report before destroying the device once and for all.
	this->reportDeviceMessages();

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
	if (_debugDevice != nullptr)
	{
		Storm::throwIfFailed(_debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL));
	}
#endif // _DEBUG
}

void Storm::DirectXController::reportDeviceMessages()
{
	if (_logDeviceMessage)
	{
		ComPtr<ID3D11InfoQueue> deviceMessageInfoQueue;
		_device->QueryInterface(__uuidof(ID3D11InfoQueue), &deviceMessageInfoQueue);

		if (deviceMessageInfoQueue)
		{
			const UINT64 numMessage = deviceMessageInfoQueue->GetNumStoredMessages();
			if (numMessage > 0)
			{
				unsigned int unprintedMsgCount = 0;
				for (UINT64 iter = 0; iter < numMessage; ++iter)
				{
					SIZE_T messageSize = 0;
					deviceMessageInfoQueue->GetMessage(iter, nullptr, &messageSize);
					if (messageSize > 0)
					{
						MallocUPtr<D3D11_MESSAGE> currentMessage{ static_cast<D3D11_MESSAGE*>(malloc(messageSize)) };
						if (SUCCEEDED(deviceMessageInfoQueue->GetMessage(iter, currentMessage.get(), &messageSize)))
						{
							printParseDeviceMessage(*currentMessage);
						}
						else
						{
							++unprintedMsgCount;
						}
					}
					else
					{
						++unprintedMsgCount;
					}
				}

				if (unprintedMsgCount > 0)
				{
					LOG_WARNING << "There was " << unprintedMsgCount << " unprinted message(s) (maybe due to an unsuccessful message grabbing or a message of size 0.";
				}

				deviceMessageInfoQueue->ClearStoredMessages();
			}
		}
	}
}

const ComPtr<ID3D11Device>& Storm::DirectXController::getDirectXDevice() const noexcept
{
	return _device;
}

const ComPtr<ID3D11DeviceContext>& Storm::DirectXController::getImmediateContext() const noexcept
{
	return _immediateContext;
}

void Storm::DirectXController::renderElements(const Storm::Camera &currentCamera, const std::vector<std::unique_ptr<Storm::IRenderedElement>> &renderedElementArrays, const std::map<unsigned int, std::unique_ptr<Storm::GraphicRigidBody>> &rbElementArrays, Storm::GraphicParticleSystem &particleSystem) const
{
	for (const auto &renderedElement : renderedElementArrays)
	{
		renderedElement->render(_device, _immediateContext, currentCamera);
	}

	switch (_currentRenderModeState)
	{
	case Storm::RenderModeState::Solid:
	case Storm::RenderModeState::SolidCullNone:
	case Storm::RenderModeState::Wireframe:
	case Storm::RenderModeState::NoWallSolid:
		for (const auto &rbElementPair : rbElementArrays)
		{
			rbElementPair.second->render(_device, _immediateContext, currentCamera);
		}
		break;

	case Storm::RenderModeState::NoWallParticles:
	case Storm::RenderModeState::AllParticle:
	default:
		break;
	}

	particleSystem.render(_device, _immediateContext, currentCamera, _currentRenderModeState);
}

float Storm::DirectXController::getViewportWidth() const noexcept
{
	return _viewportWidth;
}

float Storm::DirectXController::getViewportHeight() const noexcept
{
	return _viewportHeight;
}

void Storm::DirectXController::setWireFrameState()
{
	_immediateContext->RSSetState(_wireframeCullNoneRS.Get());
	_currentRenderModeState = Storm::RenderModeState::Wireframe;
}

void Storm::DirectXController::setSolidCullNoneState()
{
	_immediateContext->RSSetState(_solidCullNoneRS.Get());
	_currentRenderModeState = Storm::RenderModeState::SolidCullNone;
}

void Storm::DirectXController::setSolidCullBackState()
{
	_immediateContext->RSSetState(_solidCullBackRS.Get());
	_currentRenderModeState = Storm::RenderModeState::Solid;
}

void Storm::DirectXController::setAllParticleState()
{
	_currentRenderModeState = Storm::RenderModeState::AllParticle;
}

void Storm::DirectXController::setRenderNoWallParticle()
{
	_currentRenderModeState = Storm::RenderModeState::NoWallParticles;
}

void Storm::DirectXController::setRenderNoWallSolid()
{
	_immediateContext->RSSetState(_solidCullNoneRS.Get());
	_currentRenderModeState = Storm::RenderModeState::NoWallSolid;
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

void Storm::DirectXController::drawUI(const std::map<std::wstring_view, std::wstring> &texts)
{
	D2D1_RECT_F writeRectPosition{
		_writeRectLeft,
		0.f, 
		_writeRectRight,
		_writeRectHeight
	};

	WriterAutoDrawer autoDrawerRAII{ _direct2DRenderTarget.Get() };

	this->drawTextBackground(writeRectPosition);

	std::wstring tmp;

	unsigned int iter = 0;
	for (const auto fieldText : texts)
	{
		writeRectPosition.top = _textHeightCoeff * static_cast<float>(iter);
		writeRectPosition.bottom = _textHeightCoeff * static_cast<float>(iter + 1);

		tmp.clear();
		tmp.reserve(fieldText.first.size() + fieldText.second.size() + 2);

		tmp += fieldText.first;
		tmp += STORM_TEXT(": ");
		tmp += fieldText.second;

		this->drawText(tmp, writeRectPosition);

		++iter;
	}
}

void Storm::DirectXController::notifyFieldCount(std::size_t fieldCount)
{
	_writeRectHeight = (static_cast<float>(fieldCount) * _textHeightCoeff) + 1.f;
	if (_writeRectHeight > _viewportHeight)
	{
		LOG_WARNING << 
			"Beware, some field would be rendered outside the screen since either the font is too big, or the viewport is too small...\n"
			"Currently, we ask to render " << fieldCount << " field taking each " << _textHeightCoeff << " pixels.";
	}
}

void Storm::DirectXController::setTextHeightCoeff(float textHeightCoeff)
{
	_textHeightCoeff = textHeightCoeff;
	this->notifyFieldCount(Storm::GraphicManager::instance().getFieldCount());
}

void Storm::DirectXController::drawTextBackground(const D2D1_RECT_F &rectPosition)
{
	_direct2DRenderTarget->FillRectangle(rectPosition, _direct2DRectSolidBrush.Get());
}

void Storm::DirectXController::drawText(const std::wstring &text, const D2D1_RECT_F &rectPosition)
{
	_direct2DRenderTarget->DrawText(
		text.c_str(),
		static_cast<UINT32>(text.size()),
		_textFormat.Get(),
		rectPosition,
		_direct2DTextSolidBrush.Get()
	);
}

void Storm::DirectXController::internalCreateDXDevices(HWND hwnd)
{
	enum
	{
		k_wantedFPS = 60
	};

	UINT creationFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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
		descModeWanted.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
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
	//swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;

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

	ComPtr<ID3D11InfoQueue> deviceMessageInfoQueue;
	_device->QueryInterface(__uuidof(ID3D11InfoQueue), &deviceMessageInfoQueue);

	if (deviceMessageInfoQueue)
	{
		deviceMessageInfoQueue->PushEmptyStorageFilter();
	}
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

		auto &currentRTSetup = blendDesc.RenderTarget[0];
		currentRTSetup.BlendEnable = TRUE;
		currentRTSetup.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		currentRTSetup.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		currentRTSetup.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		currentRTSetup.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		currentRTSetup.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		currentRTSetup.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		currentRTSetup.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		Storm::throwIfFailed(_device->CreateBlendState(&blendDesc, &_alphaBlendEnable));

		currentRTSetup.BlendEnable = FALSE;

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

void Storm::DirectXController::internalCreateDirect2DDevices(HWND hwnd)
{
	Storm::throwIfFailed(D2D1CreateFactory<ID2D1Factory>(D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED, &_direct2DFactory));

	// For interoperability between DX11 and D2D
	ComPtr<IDXGISurface> sharedSurface;
	Storm::throwIfFailed(_swapChain->GetBuffer(0, __uuidof(IDXGISurface), &sharedSurface));

	auto dcReleaser = [hwnd](HDC* dc)
	{
		if (dc != nullptr)
		{
			ReleaseDC(hwnd, *dc);
		}
	};

	HDC currentDC = GetDC(hwnd);
	std::unique_ptr<HDC, decltype(dcReleaser)> dcRAIIHolder{ &currentDC, dcReleaser };

	float dpiX = static_cast<float>(GetDeviceCaps(currentDC, LOGPIXELSX));
	float dpiY = static_cast<float>(GetDeviceCaps(currentDC, LOGPIXELSY));

	const D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE::D2D1_RENDER_TARGET_TYPE_DEFAULT, 
		D2D1::PixelFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE::D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX, dpiY
	);
	const D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProperties = D2D1::HwndRenderTargetProperties(hwnd, D2D1::Size(static_cast<UINT>(_viewportWidth), static_cast<UINT>(_viewportHeight)));

	Storm::throwIfFailed(_direct2DFactory->CreateDxgiSurfaceRenderTarget(sharedSurface.Get(), renderTargetProperties, &_direct2DRenderTarget));

	Storm::throwIfFailed(_direct2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF{ D2D1::ColorF::Aquamarine, 0.6f }, &_direct2DRectSolidBrush));
	Storm::throwIfFailed(_direct2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF{ D2D1::ColorF::Black }, &_direct2DTextSolidBrush));

	_writeRectLeft = _viewportWidth * 0.8f;
	_writeRectRight = _viewportWidth;
}

void Storm::DirectXController::internalCreateDirectWrite()
{
	Storm::throwIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE::DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory) , &_writeFactory));

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	_textHeightCoeff = configMgr.getFontSize();

	Storm::throwIfFailed(_writeFactory->CreateTextFormat(
		L"Arial",
		nullptr,
		DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
		_textHeightCoeff,
		L"en-us",
		&_textFormat
	));

	this->setTextHeightCoeff(_textHeightCoeff + 1.f);
}
