#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct SceneBlowerConfig;

	class BlowerCubeMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};

	class BlowerSphereMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};

	class BlowerCylinderMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};

	class BlowerConeMeshMaker : private Storm::NonInstanciable
	{
	public:
		static void generate(const Storm::SceneBlowerConfig &blowerConfig, std::vector<Storm::Vector3> &outVertexes, std::vector<uint32_t> &outIndexes);
	};
}
