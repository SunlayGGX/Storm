#pragma once


namespace Storm
{
	class ParticleSystem;
	using ParticleSystemContainer = std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>>;
}
