#pragma once

#include "SceneRigidBodyConfig.h"


struct aiScene;

namespace Storm
{
	struct SceneRigidBodyConfig;
	struct SystemSimulationStateObject;

	class AssetCacheData
	{
	public:
		struct MeshData
		{
			std::vector<Storm::Vector3> _vertices;
			std::vector<Storm::Vector3> _normals;
		};

	public:
		AssetCacheData(const Storm::SceneRigidBodyConfig &rbConfig, const aiScene* meshScene, const float layerDistance);
		AssetCacheData(const Storm::SceneRigidBodyConfig &rbConfig, const Storm::AssetCacheData &srcCachedData, const float layerDistance);

	public:
		bool isEquivalentWith(const Storm::SceneRigidBodyConfig &rbConfig, bool considerFinal) const;
		bool isInsideFinalBoundingBox(const Storm::Vector3 &pos) const;

		const Storm::Vector3& getFinalBoundingBoxMin() const noexcept;
		const Storm::Vector3& getFinalBoundingBoxMax() const noexcept;

	public:
		void removeInsiderParticle(std::vector<Storm::Vector3> &inOutParticles, Storm::SystemSimulationStateObject* inOutSimulStateObjectPtr) const;

	private:
		void removeInsiderParticleWithNormalsMethod(std::vector<Storm::Vector3> &inOutParticles, Storm::SystemSimulationStateObject* inOutSimulStateObjectPtr) const;

	public:
		const std::vector<Storm::Vector3>& getSrcVertices() const noexcept;
		const std::vector<uint32_t>& getSrcIndices() const noexcept;
		const std::vector<Storm::Vector3>& getSrcNormals() const noexcept;
		const std::vector<Storm::Vector3>& getScaledVertices() const noexcept;
		const std::vector<Storm::Vector3>& getScaledNormals() const noexcept;
		const std::vector<Storm::Vector3>& getFinalVertices() const noexcept;
		const std::vector<Storm::Vector3>& getFinalNormals() const noexcept;
		const std::vector<uint32_t>& getIndices() const noexcept;
		const Storm::SceneRigidBodyConfig& getAssociatedRbConfig() const noexcept;

	private:
		void generateCurrentData(const float layerDistance);
		void buildSrc(const aiScene* meshScene);
		void generateDissociatedTriangleLayers(const float layerDistance);

		void generateCurrentDataForOneParticle(const float particleRadius);

	private:
		const Storm::SceneRigidBodyConfig _rbConfig;
		std::shared_ptr<Storm::AssetCacheData::MeshData> _src;
		Storm::AssetCacheData::MeshData _scaledCurrent;
		Storm::AssetCacheData::MeshData _finalCurrent;
		std::shared_ptr<std::vector<uint32_t>> _indices;
		std::vector<uint32_t> _overrideIndices;

		Storm::Vector3 _finalBoundingBoxMin;
		Storm::Vector3 _finalBoundingBoxMax;
	};
}
