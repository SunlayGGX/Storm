#pragma once


struct aiScene;

namespace Storm
{
	struct RigidBodySceneData;

	class AssetCacheData
	{
	private:
		struct MeshData
		{
			std::vector<Storm::Vector3> _vertices;
			std::vector<Storm::Vector3> _normals;
		};

	public:
		AssetCacheData(const Storm::RigidBodySceneData &rbConfig, const aiScene* meshScene);
		AssetCacheData(const Storm::RigidBodySceneData &rbConfig, const Storm::AssetCacheData &srcCachedData);

	public:
		bool isEquivalentWith(const Storm::RigidBodySceneData &rbConfig, bool considerFinal) const;

	public:
		const std::vector<Storm::Vector3>& getSrcVertices() const noexcept;
		const std::vector<Storm::Vector3>& getSrcNormals() const noexcept;
		const std::vector<Storm::Vector3>& getScaledVertices() const noexcept;
		const std::vector<Storm::Vector3>& getScaledNormals() const noexcept;
		const std::vector<Storm::Vector3>& getFinalVertices() const noexcept;
		const std::vector<Storm::Vector3>& getFinalNormals() const noexcept;
		const std::vector<uint32_t>& getIndices() const noexcept;

	private:
		void generateCurrentData();
		void buildSrc(const aiScene* meshScene);

	private:
		const Storm::RigidBodySceneData &_rbConfig;
		std::shared_ptr<Storm::AssetCacheData::MeshData> _src;
		Storm::AssetCacheData::MeshData _scaledCurrent;
		Storm::AssetCacheData::MeshData _finalCurrent;
		std::shared_ptr<std::vector<uint32_t>> _indices;
	};
}
