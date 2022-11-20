#include "GraphicSmoke.h"
#include "MemoryHelper.h"

#define STORM_HIJACKED_TYPE Storm::Vector4
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

namespace
{
	// https://rtouti.github.io/graphics/perlin-noise-algorithm


	float makePerlinNoise(const float x, const float y)
	{
		// TODO
		return 0.f;
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
