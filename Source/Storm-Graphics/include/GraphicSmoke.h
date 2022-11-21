#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	struct PushedParticleEmitterData;
	class SmokeShader;

	class GraphicSmoke : public Storm::IRenderedElement
	{
	public:
		GraphicSmoke(const ComPtr<ID3D11Device> &device);
		~GraphicSmoke();

	public:
		void updateData(const ComPtr<ID3D11Device> &device, const Storm::PushedParticleEmitterData &data);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext);

	private:
		ComPtr<ID3D11Texture2D> _perlinNoiseTexture;

		std::size_t _count;
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		std::unique_ptr<Storm::SmokeShader> _shader;
	};
}
