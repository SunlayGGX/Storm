#include "GraphicSmokes.h"

#include "SmokeShader.h"

#include "SingletonHolder.h"
#include "IRandomManager.h"

#include "PushedParticleEmitterData.h"

#include "SceneSmokeEmitterConfig.h"

#define STORM_HIJACKED_TYPE Storm::Vector4
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE
#define STORM_HIJACKED_TYPE int
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#include "MemoryHelper.h"


namespace
{
	// https://rtouti.github.io/graphics/perlin-noise-algorithm

	std::vector<int>& getShuffled()
	{
		static std::vector<int> shuffled = []
		{
			static std::vector<int> result;
			Storm::setNumUninitialized_safeHijack(result, Storm::VectorHijacker{ 512 });

			for (int iter = 0; iter < 256; ++iter)
			{
				result[iter] = iter;
			}

			Storm::SingletonHolder::instance().getSingleton<Storm::IRandomManager>().shuffle(result, 256);
			std::copy(std::begin(result), std::begin(result) + 256, std::begin(result) + 256);

			return result;
		}();

		return shuffled;
	}

	// destroy shuffle once and for all to save memory. Operation cannot be undoed once done. Unsafe to call getShuffled afterward.
	void purgeShuffled()
	{
		auto &shuffled = getShuffled();
		shuffled.clear();
		shuffled.shrink_to_fit();
	}

	float fade(float val)
	{
		return ((6.f * val - 15.f) * val + 10.f) * val * val * val;
	}

	Storm::Vector2 getConstantVector(int shuffledVal)
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

	float makePerlinNoise(float x, float y, const float frequency)
	{
		x *= frequency;
		y *= frequency;

		const int x_floor = static_cast<int>(x) & 255;
		const int y_floor = static_cast<int>(y) & 255;

		const Storm::Vector2 bottomLeft{ x - std::floor(x), y - std::floor(y) };
		const Storm::Vector2 topRight{ bottomLeft.x() - 1.f, bottomLeft.y() - 1.f};
		const Storm::Vector2 topLeft{ bottomLeft.x(), bottomLeft.y() - 1.0 };
		const Storm::Vector2 bottomRight{ bottomLeft.x() - 1.0, bottomLeft.y() };

		const auto &shuffled = getShuffled();

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
					noise += makePerlinNoise(static_cast<float>(x), static_cast<float>(y), 1.f / dividerFl) * coeff;
				}
				noise = (noise + coeffSum) / (2.f * coeffSum);

				auto &color = result[yoffset + x];
				color.x() = noise;
				color.y() = noise;
				color.z() = noise;
				color.w() = 1.f;
			}
		}

		purgeShuffled();

		return result;
	}

	struct GraphicData
	{
	public:
		using PointType = DirectX::XMFLOAT3;

	public:
		PointType _position;
	};
}

Storm::GraphicSmokes::InternalOneSmokeEmit::InternalOneSmokeEmit(const Storm::SceneSmokeEmitterConfig &smokeCfg) :
	_color{ smokeCfg._color.x(), smokeCfg._color.y(), smokeCfg._color.z(), smokeCfg._color.w() },
	_count{ 0 },
	_updated{ false }
{

}

Storm::GraphicSmokes::GraphicSmokes(const ComPtr<ID3D11Device> &device, const std::vector<Storm::SceneSmokeEmitterConfig> &smokeCfgs)
{
	assert(!smokeCfgs.empty() && "Smoke emitter configs should posess at least one emitter to spawn.");

	this->initPerlinNoiseTexture(device);
	this->initSmokeShader(device);

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
	CD3D11_SHADER_RESOURCE_VIEW_DESC resourceView{ D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D };

	ComPtr<ID3D11ShaderResourceView> perlinNoiseTextureSRV = nullptr;
	Storm::throwIfFailed(device->CreateShaderResourceView(_perlinNoiseTexture.Get(), &resourceView, &perlinNoiseTextureSRV));

	_shader = std::make_unique<Storm::SmokeShader>(device, std::move(perlinNoiseTextureSRV));
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

		const auto smokeCount = pushedData._positions.size();
		assert(smokeCount > 0 && "We should have emitted smokes to render.");

		const bool shouldRegenIndexBuffer = graphicSmokeEmitterElem._indexBuffer == nullptr || graphicSmokeEmitterElem._count != smokeCount;

		// In case it has a vertex buffer set (most of the time)
		graphicSmokeEmitterElem._vertexBuffer = nullptr;

		// Create Vertex data
		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData;
		ZeroMemories(vertexBufferDesc, vertexData);

		vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(GraphicData) * static_cast<UINT>(smokeCount);
		vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

		vertexData.pSysMem = pushedData._positions.data();

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
	for (const auto &graphicSmokeEmitterPair : _emitObjects)
	{
		auto &graphicSmokeEmitterElem = graphicSmokeEmitterPair.second;
		if (graphicSmokeEmitterElem._updated)
		{
			_shader->setup(deviceContext, currentCamera, graphicSmokeEmitterElem._color);
			this->setupForRender(deviceContext, graphicSmokeEmitterElem);
			_shader->draw(static_cast<unsigned int>(graphicSmokeEmitterElem._count), deviceContext);
		}
	}
}

void Storm::GraphicSmokes::setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext, const InternalOneSmokeEmit &graphicSmokeEmitterElem)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	constexpr UINT stride = sizeof(GraphicData);
	constexpr UINT offset = 0;

	deviceContext->IASetIndexBuffer(graphicSmokeEmitterElem._indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = graphicSmokeEmitterElem._vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}
