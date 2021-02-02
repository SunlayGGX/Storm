#pragma once

#include "IRenderedElement.h"
#include "GraphicAreaBaseHolder.h"


namespace Storm
{
	struct GraphicParticleData;

	class GraphicKernelEffectArea :
		public Storm::IRenderedElement,
		private Storm::GraphicAreaBaseHolder
	{
	public:
		GraphicKernelEffectArea(const ComPtr<ID3D11Device> &device);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	public:
		void setAreaRadius(const float radius);
		void setAreaPosition(const Storm::GraphicParticleData &selectedParticleData);

		void setHasParticleHook(bool hasHook) noexcept;

		void tweakEnabled() noexcept;

	private:
		float _kernelRadius;
		DirectX::XMVECTOR _currentAreaPosition;

		bool _enabled;
		bool _hasParticleHook;
	};
}
