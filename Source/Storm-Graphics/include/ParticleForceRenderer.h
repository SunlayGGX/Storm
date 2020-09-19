#pragma once


namespace Storm
{
	struct GraphicParticleData;


	class ParticleForceRenderer
	{
	public:
		ParticleForceRenderer(const ComPtr<ID3D11Device> &device);
		~ParticleForceRenderer();

	public:
		bool prepareData(unsigned int particleSystemId, std::vector<Storm::GraphicParticleData> &particlePosition, const std::pair<unsigned int, std::size_t> &selectedParticle);

		void refreshForceData(const ComPtr<ID3D11Device> &device, const Storm::Vector3 &selectedParticlePosition, const Storm::Vector3 &selectedParticleForce);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		Storm::Vector3 _lastParticlePositionCached;
		Storm::Vector3 _lastParticleForceCached;
	};
}
