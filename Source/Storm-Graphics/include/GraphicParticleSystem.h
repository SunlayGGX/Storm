#pragma once


namespace Storm
{
	// No need to lock. GraphicParticleSystem should and must only be exposed to the graphics thread...
	// The moment it refresh its data is controlled and the data is already part of the graphic thread.
	class GraphicParticleSystem
	{
	public:
		GraphicParticleSystem();

	public:
		void refreshParticleSystemData(unsigned int particleSystemId, std::vector<DirectX::XMFLOAT3> &&particlePosition);

	private:
		std::map<unsigned int, std::vector<DirectX::XMFLOAT3>> _particleSystemPositions;
	};
}
