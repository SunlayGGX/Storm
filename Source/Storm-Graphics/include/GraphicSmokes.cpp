#include "GraphicSmokes.h"

#include "SmokeShader.h"
#include "TextureOMBlendShader.h"

#include "SingletonHolder.h"
#include "IRandomManager.h"
#include "IGraphicsManager.h"

#include "PushedParticleEmitterData.h"

#include "SceneSmokeEmitterConfig.h"

#define STORM_HIJACKED_TYPE Storm::Vector4
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE
#define STORM_HIJACKED_TYPE int
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#include "MemoryHelper.h"
#include "RAII.h"


namespace
{
	enum { k_outputMergerVertexCount = 4 };


	// https://rtouti.github.io/graphics/perlin-noise-algorithm

	std::vector<int> makeShuffled()
	{
		enum : std::size_t
		{
			k_indexSz = 256,
			k_fullSz = k_indexSz * 2,
		};

		std::vector<int> result;
		Storm::setNumUninitialized_safeHijack(result, Storm::VectorHijacker{ k_fullSz });

		for (int iter = 0; iter < k_indexSz; ++iter)
		{
			result[iter] = iter;
		}

		Storm::SingletonHolder::instance().getSingleton<Storm::IRandomManager>().shuffle(result, k_indexSz);
		const auto midVectIter = std::begin(result) + k_indexSz;
		std::copy(std::begin(result), midVectIter, midVectIter);

		return result;
	}

	constexpr float fade(const float val)
	{
		return ((6.f * val - 15.f) * val + 10.f) * val * val * val;
	}

	Storm::Vector2 getConstantVector(const int shuffledVal)
	{
		const auto index3bits = shuffledVal & 3;
		switch (index3bits)
		{
		case 0: return Storm::Vector2{ 1.f, 1.f };
		case 1: return Storm::Vector2{ -1.f, 1.f };
		case 2: return Storm::Vector2{ -1.f, -1.f };
		case 3: return Storm::Vector2{ 1.f, -1.f };
		default: __assume(false);
		}
	}

	float makePerlinNoise(const std::vector<int> &shuffled, float x, float y, const float frequency)
	{
		x *= frequency;
		y *= frequency;

		const int x_floor = static_cast<int>(x) & 255;
		const int y_floor = static_cast<int>(y) & 255;

		const Storm::Vector2 bottomLeft{ x - std::floor(x), y - std::floor(y) };
		const Storm::Vector2 topRight{ bottomLeft.x() - 1.f, bottomLeft.y() - 1.f};
		const Storm::Vector2 topLeft{ bottomLeft.x(), bottomLeft.y() - 1.0 };
		const Storm::Vector2 bottomRight{ bottomLeft.x() - 1.0, bottomLeft.y() };

		//Select a value in the array for each of the 4 corners
		const auto valueTopRight = shuffled[shuffled[x_floor + 1] + y_floor + 1];
		const auto valueTopLeft = shuffled[shuffled[x_floor] + y_floor + 1];
		const auto valueBottomRight = shuffled[shuffled[x_floor + 1] + y_floor];
		const auto valueBottomLeft = shuffled[shuffled[x_floor] + y_floor];

		const auto dotTopRight = topRight.dot(getConstantVector(valueTopRight));
		const auto dotTopLeft = topLeft.dot(getConstantVector(valueTopLeft));
		const auto dotBottomRight = bottomRight.dot(getConstantVector(valueBottomRight));
		const auto dotBottomLeft = bottomLeft.dot(getConstantVector(valueBottomLeft));

		const float u = fade(bottomLeft.x());
		const float v = fade(bottomLeft.y());

		return std::lerp(std::lerp(dotBottomLeft, dotTopLeft, v), std::lerp(dotBottomRight, dotTopRight, v), u);
	}

	std::vector<Storm::Vector4> makePerlinTextureData(const std::size_t width, const std::size_t height)
	{
		std::vector<Storm::Vector4> result;
		Storm::setNumUninitialized_safeHijack(result, Storm::VectorHijacker{ width * height });

		enum
		{
			k_minFrequencyDivider = 10,
			k_maxFrequencyDivider = 40,
			k_frequencyIncrement = 10,
			k_diffFrequencyDivider = (k_maxFrequencyDivider - k_minFrequencyDivider) / k_frequencyIncrement,
			k_normalizerFrequencyDivider = k_diffFrequencyDivider * 2,
		};

		STORM_STATIC_ASSERT(
			((k_maxFrequencyDivider - k_minFrequencyDivider) % k_frequencyIncrement) == 0,
			"the difference in frequency divider should be a multiple of the step!"
		);

		const auto shuffled = makeShuffled();

		constexpr float k_interpolCoeff = ((1.f / static_cast<float>(k_diffFrequencyDivider)) - 1.f) / static_cast<float>(k_diffFrequencyDivider);

		for (std::size_t y = 0; y < height; ++y)
		{
			const std::size_t yoffset = y * width;
			for (std::size_t x = 0; x < width; ++x)
			{
				float noise = 0.f;
				float coeffSum = 0.f;
				for (int divider = k_minFrequencyDivider; divider < k_maxFrequencyDivider; divider += k_frequencyIncrement)
				{
					const float dividerFl = static_cast<float>(divider);
					const float coeff = (static_cast<float>(divider - k_minFrequencyDivider) * k_interpolCoeff) + 1.f;
					coeffSum += coeff;
					noise += makePerlinNoise(shuffled, static_cast<float>(x), static_cast<float>(y), 1.f / dividerFl) * coeff;
				}
				noise = (noise + coeffSum) / (2.f * coeffSum);

				auto &color = result[yoffset + x];
				color.x() = noise;
				color.y() = noise;
				color.z() = noise;
				color.w() = 1.f;
			}
		}

		return result;
	}

	void setRenderTarget(const ComPtr<ID3D11DeviceContext> &immediateCtx, const ComPtr<ID3D11RenderTargetView> &rt, const ComPtr<ID3D11DepthStencilView> &stencilV)
	{
		ComPtr<ID3D11RenderTargetView> voidTargetView;
		immediateCtx->OMSetRenderTargets(1, &voidTargetView, nullptr);

		ID3D11RenderTargetView*const tmpRTV = rt.Get();
		immediateCtx->OMSetRenderTargets(1, &tmpRTV, stencilV.Get());
	}

	struct SmokeGraphicData
	{
	public:
		using PointType = DirectX::XMFLOAT3;

	public:
		PointType _position;
		float _alphaCoeff;
	};

	struct OutputMergeGraphicData
	{
	public:
		DirectX::XMFLOAT4 _position;
	};
}

Storm::GraphicSmokes::InternalOneSmokeEmit::InternalOneSmokeEmit(const Storm::SceneSmokeEmitterConfig &smokeCfg) :
	_color{ smokeCfg._color.x(), smokeCfg._color.y(), smokeCfg._color.z(), smokeCfg._color.w() },
	_count{ 0 },
	_updated{ false }
{

}

Storm::GraphicSmokes::SmokeRTTargets::SmokeRTTargets(const ComPtr<ID3D11Device> &device)
{
	UINT width;
	UINT height;
	{
		float widthFl;
		float heightFl;
		SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>().getViewportTextureResolution(widthFl, heightFl);
		width = static_cast<decltype(width)>(widthFl);
		height = static_cast<decltype(height)>(heightFl);
	}

	CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, width, height };
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;

	std::vector<Storm::Vector4> initialTextureData;
	Storm::setNumUninitialized_safeHijack(initialTextureData, Storm::VectorHijacker{ static_cast<std::size_t>(width * height) });
	memset(initialTextureData.data(), 0, initialTextureData.size() * sizeof(Storm::Vector4));

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemories(initialData);
	initialData.pSysMem = initialTextureData.data();
	initialData.SysMemPitch = desc.Width * sizeof(decltype(*initialTextureData.data()));

	Storm::throwIfFailed(device->CreateTexture2D(&desc, &initialData, &_renderTargetTexture));
	Storm::throwIfFailed(device->CreateRenderTargetView(_renderTargetTexture.Get(), nullptr, &_renderTargetView));


	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	Storm::throwIfFailed(device->CreateTexture2D(&desc, &initialData, &_saveTexture));
	Storm::throwIfFailed(device->CreateShaderResourceView(_saveTexture.Get(), nullptr, &_saveTextureView));

	const CD3D11_SHADER_RESOURCE_VIEW_DESC resourceView{ D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D };
	Storm::throwIfFailed(device->CreateShaderResourceView(_renderTargetTexture.Get(), &resourceView, &_renderTargetTextureView));
}

Storm::GraphicSmokes::GraphicSmokes(const ComPtr<ID3D11Device> &device, const std::vector<Storm::SceneSmokeEmitterConfig> &smokeCfgs) :
	_frameBefore{ device },
	_outputMergerShader{ std::make_unique<std::remove_cvref_t<decltype(*_outputMergerShader)>>(device) }
{
	assert(!smokeCfgs.empty() && "Smoke emitter configs should possess at least one emitter to spawn.");

	this->initPerlinNoiseTexture(device);
	this->initSmokeShader(device);
	this->initOutputMergerShader(device);

	for (const auto &smokeCfg : smokeCfgs)
	{
		if (auto emplaced = _emitObjects.try_emplace(smokeCfg._emitterId, smokeCfg);
			!emplaced.second)
		{
			Storm::throwException<Storm::Exception>("Cannot emplace graphic smoke particle with id " + Storm::toStdString(smokeCfg._emitterId));
		}
	}
}

Storm::GraphicSmokes::~GraphicSmokes() = default;

void Storm::GraphicSmokes::initPerlinNoiseTexture(const ComPtr<ID3D11Device> &device)
{
	CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 128, 128 };
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;

	const auto perlinData = makePerlinTextureData(desc.Width, desc.Height);

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemories(initialData);
	initialData.pSysMem = perlinData.data();
	initialData.SysMemPitch = desc.Width * sizeof(decltype(*perlinData.data()));

	Storm::throwIfFailed(device->CreateTexture2D(&desc, &initialData, &_perlinNoiseTexture));
}

void Storm::GraphicSmokes::initSmokeShader(const ComPtr<ID3D11Device> &device)
{
	const CD3D11_SHADER_RESOURCE_VIEW_DESC resourceView{ D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D };

	ComPtr<ID3D11ShaderResourceView> perlinNoiseTextureSRV = nullptr;
	Storm::throwIfFailed(device->CreateShaderResourceView(_perlinNoiseTexture.Get(), &resourceView, &perlinNoiseTextureSRV));

	_smokeShader = std::make_unique<Storm::SmokeShader>(device, std::move(perlinNoiseTextureSRV));
}

void Storm::GraphicSmokes::initOutputMergerShader(const ComPtr<ID3D11Device> &device)
{
	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemories(vertexBufferDesc, vertexData);

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(OutputMergeGraphicData) * k_outputMergerVertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	constexpr const OutputMergeGraphicData k_screenSpaceVertexData[k_outputMergerVertexCount] =
	{
		{ ._position = { 1.f, 0.f, 0.f, 0.f } },
		{ ._position = { 0.f, 1.f, 0.f, 0.f } },
		{ ._position = { 0.f, 0.f, 1.f, 0.f } },
		{ ._position = { 0.f, 0.f, 0.f, 0.f } },
	};

	vertexData.pSysMem = k_screenSpaceVertexData;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_outputMergerVertexBuffer));

	
	// Create Indexes data
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemories(indexBufferDesc, indexData);

	indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * k_outputMergerVertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

	constexpr uint32_t k_indexData[k_outputMergerVertexCount] = { 0, 1, 2, 3 };
	indexData.pSysMem = k_indexData;

	Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_outputMergerIndexBuffer));
}

void Storm::GraphicSmokes::prepareUpdate()
{
	for (auto &graphicSmokeEmitterElemPair : _emitObjects)
	{
		graphicSmokeEmitterElemPair.second._updated = false;
	}
}

void Storm::GraphicSmokes::handlePushedData(const ComPtr<ID3D11Device> &device, const Storm::PushedParticleEmitterData &pushedData)
{
	if (auto found = _emitObjects.find(pushedData._id);
		found != std::end(_emitObjects))
	{
		auto &graphicSmokeEmitterElem = found->second;
		assert(graphicSmokeEmitterElem._updated == false && "We should prepare to update before coming in this method!");

		const auto smokeCount = pushedData._data.size();
		assert(smokeCount > 0 && "We should have emitted smokes to render.");

		const bool shouldRegenIndexBuffer = graphicSmokeEmitterElem._indexBuffer == nullptr || graphicSmokeEmitterElem._count != smokeCount;

		// In case it has a vertex buffer set (most of the time)
		graphicSmokeEmitterElem._vertexBuffer = nullptr;

		// Create Vertex data
		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData;
		ZeroMemories(vertexBufferDesc, vertexData);

		vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(SmokeGraphicData) * static_cast<UINT>(smokeCount);
		vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

		vertexData.pSysMem = pushedData._data.data();

		Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &graphicSmokeEmitterElem._vertexBuffer));

		if (shouldRegenIndexBuffer)
		{
			std::unique_ptr<uint32_t[]> indexes = std::make_unique<uint32_t[]>(smokeCount);
			for (uint32_t iter = 0; iter < smokeCount; ++iter)
			{
				indexes[iter] = iter;
			}

			// Create Indexes data
			D3D11_BUFFER_DESC indexBufferDesc;
			D3D11_SUBRESOURCE_DATA indexData;
			ZeroMemories(indexBufferDesc, indexData);

			indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * smokeCount);
			indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

			indexData.pSysMem = indexes.get();

			Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &graphicSmokeEmitterElem._indexBuffer));
		}

		graphicSmokeEmitterElem._count = smokeCount;
		graphicSmokeEmitterElem._updated = true;
	}
	else
	{
		Storm::throwException<Storm::Exception>("No graphic smoke emitter with id " + Storm::toStdString(pushedData._id) + " exists!");
	}
}

void Storm::GraphicSmokes::updateData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::PushedParticleEmitterData> &data)
{
	this->prepareUpdate();

	for (const auto &pushedData : data)
	{
		this->handlePushedData(device, pushedData);
	}
}

void Storm::GraphicSmokes::render(const ComPtr<ID3D11Device> &/*device*/, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	if (!_emitObjects.empty())
	{
		this->saveHiddenFrameBeforeBeforeClearing(deviceContext);

		{
			// Change Render target to a separate, hidden frame that is not the one directly bound to the viewport.
			ComPtr<ID3D11RenderTargetView> oldRT;
			ComPtr<ID3D11DepthStencilView> oldStencilV;
			deviceContext->OMGetRenderTargets(1, &oldRT, &oldStencilV);
			auto rtResetToOld = Storm::makeLazyRAIIObject([&deviceContext, &oldRT, &oldStencilV]()
			{
				setRenderTarget(deviceContext, oldRT, oldStencilV);
			});
			setRenderTarget(deviceContext, _frameBefore._renderTargetView, oldStencilV);

			// gather/blend old (reduced by a certain amout) and new into an hidden frame to make trailing, persistent effects.
			this->renderOldPersistentFrameReduced(deviceContext, currentCamera);
			this->renderNewSmokeEmission(deviceContext, currentCamera);
		}

		this->renderHiddenFrameToFinalOutput(deviceContext, currentCamera);
	}
}

void Storm::GraphicSmokes::renderOldPersistentFrameReduced(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	// Lose 5% each frame
	_outputMergerShader->setup(deviceContext, currentCamera, _frameBefore._saveTextureView.Get(), 0.95f);
	this->setupForMerger(deviceContext);
	_outputMergerShader->draw(k_outputMergerVertexCount, deviceContext);
}

void Storm::GraphicSmokes::renderHiddenFrameToFinalOutput(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	_outputMergerShader->setup(deviceContext, currentCamera, _frameBefore._saveTextureView.Get(), 1.f);
	this->setupForMerger(deviceContext);
	_outputMergerShader->draw(k_outputMergerVertexCount, deviceContext);
}

void Storm::GraphicSmokes::renderNewSmokeEmission(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	for (const auto &graphicSmokeEmitterPair : _emitObjects)
	{
		auto &graphicSmokeEmitterElem = graphicSmokeEmitterPair.second;
		if (graphicSmokeEmitterElem._updated)
		{
			_smokeShader->setup(deviceContext, currentCamera, graphicSmokeEmitterElem._color);
			this->setupForSmoke(deviceContext, graphicSmokeEmitterElem);
			_smokeShader->draw(static_cast<unsigned int>(graphicSmokeEmitterElem._count), deviceContext);
		}
	}
}

void Storm::GraphicSmokes::saveHiddenFrameBeforeBeforeClearing(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	deviceContext->CopyResource(_frameBefore._saveTexture.Get(), _frameBefore._renderTargetTexture.Get());
	constexpr float k_noColor[4] = { 0.f, 0.f, 0.f, 0.f };
	deviceContext->ClearRenderTargetView(_frameBefore._renderTargetView.Get(), k_noColor);
}

void Storm::GraphicSmokes::setupForMerger(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	constexpr UINT stride = sizeof(OutputMergeGraphicData);
	constexpr UINT offset = 0;

	deviceContext->IASetIndexBuffer(_outputMergerIndexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _outputMergerVertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}

void Storm::GraphicSmokes::setupForSmoke(const ComPtr<ID3D11DeviceContext> &deviceContext, const InternalOneSmokeEmit &graphicSmokeEmitterElem)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	constexpr UINT stride = sizeof(SmokeGraphicData);
	constexpr UINT offset = 0;

	deviceContext->IASetIndexBuffer(graphicSmokeEmitterElem._indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = graphicSmokeEmitterElem._vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}
