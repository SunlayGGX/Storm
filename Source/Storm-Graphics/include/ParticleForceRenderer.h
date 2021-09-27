#pragma once


namespace Storm
{
	struct GraphicParticleData;
	class ParticleForceShader;
	class Camera;
	class GraphicParameters;

	class ParticleForceRenderer
	{
	public:
		ParticleForceRenderer(const ComPtr<ID3D11Device> &device);
		~ParticleForceRenderer();

	public:
		bool prepareData(unsigned int particleSystemId, std::vector<Storm::GraphicParticleData> &particlePosition, const std::pair<unsigned int, std::size_t> &selectedParticle);

		void refreshForceData(const ComPtr<ID3D11Device> &device, const Storm::GraphicParameters &params);
		void updateForceData(const ComPtr<ID3D11Device> &device, const Storm::GraphicParameters &params, const Storm::Vector3 &selectedPosition, const Storm::Vector3 &selectedForce);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext);

	public:
		void tweekAlwaysOnTop();

	private:
		Storm::Vector3 _lastPositionCached;
		Storm::Vector3 _lastForceCached;

		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		std::unique_ptr<Storm::ParticleForceShader> _shader;
	};
}
