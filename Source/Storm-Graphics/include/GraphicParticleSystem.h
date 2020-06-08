#pragma once


namespace Storm
{
	class Camera;
	class ParticleShader;
	struct GraphicParticleData;

	// No need to lock. GraphicParticleSystem should and must only be exposed to the graphics thread...
	// The moment it refresh its data is controlled and the data is already part of the graphic thread.
	class GraphicParticleSystem
	{
	private:
		struct InternalParticleSystemBuffer
		{
		public:
			std::size_t _vertexCount;
			ComPtr<ID3D11Buffer> _vertexBuffer;
			ComPtr<ID3D11Buffer> _indexBuffer;
		};

	public:
		GraphicParticleSystem(const ComPtr<ID3D11Device> &device);
		~GraphicParticleSystem();

	public:
		void refreshParticleSystemData(unsigned int particleSystemId, std::vector<Storm::GraphicParticleData> &&particlePosition);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext, const InternalParticleSystemBuffer &sysBufferToRender);

	private:
		std::map<unsigned int, InternalParticleSystemBuffer> _particleSystemVBuffer;
		std::unique_ptr<Storm::ParticleShader> _shader;
	};
}
