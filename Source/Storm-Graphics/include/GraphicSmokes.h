#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	struct PushedParticleEmitterData;
	struct SceneSmokeEmitterConfig;
	class SmokeShader;

	class GraphicSmokes : public Storm::IRenderedElement
	{
	private:
		struct InternalOneSmokeEmit
		{
		public:
			InternalOneSmokeEmit(const Storm::SceneSmokeEmitterConfig &smokeCfg);

		public:
			bool _updated;
			std::size_t _count;
			ComPtr<ID3D11Buffer> _vertexBuffer;
			ComPtr<ID3D11Buffer> _indexBuffer;

			DirectX::XMVECTOR _color;
		};

	public:
		GraphicSmokes(const ComPtr<ID3D11Device> &device, const std::vector<Storm::SceneSmokeEmitterConfig> &smokeCfgs);
		~GraphicSmokes();

	private:
		void initPerlinNoiseTexture(const ComPtr<ID3D11Device> &device);
		void initSmokeShader(const ComPtr<ID3D11Device> &device);

		void prepareUpdate();
		void handlePushedData(const ComPtr<ID3D11Device> &device, const Storm::PushedParticleEmitterData &pushedData);

	public:
		void updateData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::PushedParticleEmitterData> &data);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext, const InternalOneSmokeEmit &graphicSmokeEmitterElem);

	private:
		ComPtr<ID3D11Texture2D> _perlinNoiseTexture;
		std::map<unsigned int, InternalOneSmokeEmit> _emitObjects;

		std::unique_ptr<Storm::SmokeShader> _shader;
	};
}
