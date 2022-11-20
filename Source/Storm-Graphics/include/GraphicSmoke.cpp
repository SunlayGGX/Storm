#include "GraphicSmoke.h"
#include "MemoryHelper.h"

#include "SingletonHolder.h"
#include "IRandomManager.h"

#define STORM_HIJACKED_TYPE Storm::Vector4
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE
#define STORM_HIJACKED_TYPE int
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


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

	float makePerlinNoise(const float x, const float y)
	{
		const int x_floor = static_cast<int>(std::floor(x)) & 255;
		const int y_floor = static_cast<int>(std::floor(y)) & 255;

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

		for (std::size_t y = 0; y < height; ++y)
		{
			const std::size_t yoffset = y * width;
			for (std::size_t x = 0; x < width; ++x)
			{
				float noise = makePerlinNoise(static_cast<float>(x) * 0.01f, static_cast<float>(y) * 0.01f);
				noise = (noise + 1.f) / 2.f;

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
}

Storm::GraphicSmoke::GraphicSmoke(const ComPtr<ID3D11Device> &device) :
	_perlinNoiseTexture{ nullptr }
{
	CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 500, 500 };
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;

	const auto perlinData = makePerlinTextureData(desc.Width, desc.Height);

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemories(initialData);
	initialData.pSysMem = perlinData.data();
	initialData.SysMemPitch = desc.Width * sizeof(decltype(*perlinData.data()));

	Storm::throwIfFailed(device->CreateTexture2D(&desc, &initialData, &_perlinNoiseTexture));
}
