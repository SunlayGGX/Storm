#include "GraphicGravity.h"

#include "GravityShader.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GeneralGraphicConfig.h"
#include "SceneSimulationConfig.h"


namespace
{
	struct GravityVertexType
	{
	public:
		using PointType = DirectX::XMVECTOR;

	public:
		GravityVertexType(const Storm::Vector3 &vect) :
			pt{ vect.x(), vect.y(), vect.z(), 0.f }
		{}

	public:
		PointType pt;
	};

	__forceinline float normalizedToViewportCoord(const float normalized, const float viewportDim)
	{
		return (normalized / 2.f + 0.5f) * viewportDim;
	}
}


Storm::GraphicGravity::GraphicGravity(const ComPtr<ID3D11Device> &device, const ComPtr<ID2D1RenderTarget> &hudTarget)
{
	enum { k_vertexCount = 1 };
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	
	const Storm::GeneralGraphicConfig &generalGraphicConfig = configMgr.getGeneralGraphicConfig();
	_visible = generalGraphicConfig._showGravityArrow;
	
	const Storm::SceneSimulationConfig &generalSimulConfig = configMgr.getSceneSimulationConfig();
	
	// Index and vertex CPU buffers
	const GravityVertexType k_coordinateSystemVertexes[k_vertexCount] =
	{
		GravityVertexType{ generalSimulConfig._gravity }
	};

	constexpr int32_t k_coordinateSystemIndexes[k_vertexCount] = { 0 };
	
	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(k_coordinateSystemVertexes);
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = k_coordinateSystemVertexes;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));


	// Create Indexes data
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;

	indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(k_coordinateSystemIndexes);
	indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = k_coordinateSystemIndexes;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));

	// For the HUD
	Storm::throwIfFailed(hudTarget->CreateSolidColorBrush(D2D1::ColorF{ D2D1::ColorF::BlanchedAlmond }, &_direct2DTextSolidBrush));

	_gravityShader = std::make_unique<Storm::GravityShader>(device);
}

void Storm::GraphicGravity::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	if (_visible)
	{
		_gravityShader->setup(device, deviceContext, currentCamera);
		this->setup(deviceContext);
		_gravityShader->draw(deviceContext);
	}
}

void Storm::GraphicGravity::postRenderUI(const ComPtr<ID2D1RenderTarget>& hudTarget, IDWriteTextFormat*const textFormat, const float viewportWidth, const float viewportHeight)
{
	if (_visible)
	{
		Storm::IRenderedElement::postRenderUI(hudTarget, textFormat, viewportWidth, viewportHeight);

		std::pair<float, float> beginRect = _gravityShader->getDrawLocation();
		beginRect.first = normalizedToViewportCoord(beginRect.first, viewportWidth);
		// Minus because the coordinates don't have the same origins. One (the uniform) starts at the top-left of the screen while the other starts at the bottom-left.
		beginRect.second = normalizedToViewportCoord(-beginRect.second, viewportHeight);

		const float rectDim = _gravityShader->getAxisLengthUnit();
		const D2D1_RECT_F writeRectPosition{
			beginRect.first,
			beginRect.second,
			beginRect.first + normalizedToViewportCoord(rectDim, viewportWidth),
			beginRect.second + normalizedToViewportCoord(rectDim, viewportHeight)
		};

		constexpr std::wstring_view gravityTag = STORM_TEXT("G");
		hudTarget->DrawText(
			gravityTag.data(),
			static_cast<UINT32>(gravityTag.size()),
			textFormat,
			writeRectPosition,
			_direct2DTextSolidBrush.Get()
		);
	}
}

void Storm::GraphicGravity::setup(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	if (_visible)
	{
		constexpr UINT stride = sizeof(GravityVertexType);
		constexpr UINT offset = 0;

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

		ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
		deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
	}
}

void Storm::GraphicGravity::switchShow()
{
	_visible = !_visible;
}
