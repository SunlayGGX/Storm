#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	struct PushedParticleEmitterData;
	struct SceneSmokeEmitterConfig;
	class TextureOMBlendShader;
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

		struct SmokeRTTargets
		{
		public:
			SmokeRTTargets(const ComPtr<ID3D11Device> &device);

		public:
			ComPtr<ID3D11RenderTargetView> _renderTargetView;
			ComPtr<ID3D11ShaderResourceView> _renderTargetTextureView;
			ComPtr<ID3D11Texture2D> _renderTargetTexture;
			ComPtr<ID3D11ShaderResourceView> _saveTextureView;
			ComPtr<ID3D11Texture2D> _saveTexture;
		};

	public:
		GraphicSmokes(const ComPtr<ID3D11Device> &device, const std::vector<Storm::SceneSmokeEmitterConfig> &smokeCfgs);
		~GraphicSmokes();

	private:
		void initPerlinNoiseTexture(const ComPtr<ID3D11Device> &device);
		void initSmokeShader(const ComPtr<ID3D11Device> &device);
		void initOutputMergerShader(const ComPtr<ID3D11Device> &device);

		void prepareUpdate();
		void handlePushedData(const ComPtr<ID3D11Device> &device, const Storm::PushedParticleEmitterData &pushedData);

	public:
		void updateData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::PushedParticleEmitterData> &data);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void renderFirstPass(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);
		void renderSecondPass(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		void setupForFirstPassRender(const ComPtr<ID3D11DeviceContext> &deviceContext);
		void setupForSecondPassRender(const ComPtr<ID3D11DeviceContext> &deviceContext, const InternalOneSmokeEmit &graphicSmokeEmitterElem);

	private:
		ComPtr<ID3D11Texture2D> _perlinNoiseTexture;
		SmokeRTTargets _frameBefore;

		std::map<unsigned int, InternalOneSmokeEmit> _emitObjects;
		std::unique_ptr<Storm::SmokeShader> _smokeShader;

		ComPtr<ID3D11Buffer> _outputMergerVertexBuffer;
		ComPtr<ID3D11Buffer> _outputMergerIndexBuffer;
		std::unique_ptr<Storm::TextureOMBlendShader> _outputMergerShader;
	};
}
