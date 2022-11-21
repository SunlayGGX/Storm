#pragma once


namespace Storm
{
	class IRenderedElement;
	class GraphicRigidBody;
	class GraphicBlower;
	class GraphicParticleSystem;
	class GraphicKernelEffectArea;
	class ParticleForceRenderer;
	class GraphicConstraintSystem;
	class GraphicNormals;
	class GraphicSmokes;

	// Pointers are facultative elements. They are nullptr if we shouldn't render them.
	// References are mandatory elements.
	struct RenderedElementProxy
	{
	public:
		const std::vector<std::unique_ptr<Storm::IRenderedElement>> &_renderedElementArrays;
		const std::map<unsigned int, std::unique_ptr<Storm::GraphicRigidBody>> &_rbElementArrays;
		Storm::GraphicParticleSystem &_particleSystem;
		const std::map<std::size_t, std::unique_ptr<Storm::GraphicBlower>> &_blowersMap;
		Storm::GraphicConstraintSystem &_constraintSystem;
		Storm::ParticleForceRenderer &_selectedParticleForce;
		Storm::GraphicKernelEffectArea &_kernelEffectArea;
		Storm::GraphicNormals*const _graphicNormals;
		Storm::GraphicSmokes*const _graphicSmokesOptional;

		bool _multiPass;
	};
}
