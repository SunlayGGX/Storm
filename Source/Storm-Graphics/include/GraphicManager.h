#pragma once

#include "Singleton.h"
#include "IGraphicsManager.h"


namespace Storm
{
	class DirectXController;
	class Camera;
	class IRenderedElement;
	class GraphicRigidBody;
	class GraphicParticleSystem;
	class GraphicBlower;
	class GraphicConstraintSystem;

	class GraphicManager :
		private Storm::Singleton<GraphicManager>,
		public Storm::IGraphicsManager
	{
		STORM_DECLARE_SINGLETON(GraphicManager);

	private:
		bool initialize_Implementation();
		void initialize_Implementation(void* hwnd);
		void cleanUp_Implementation();

	public:
		void update() final override;

	public:
		void addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes) final override;
		void bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const final override;

	public:
		void loadBlower(const Storm::BlowerData &blowerData, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes) final override;

	public:
		void pushParticlesData(unsigned int particleSystemId, const std::vector<Storm::Vector3> &particlePosData, const std::vector<Storm::Vector3> &particlevelocityData, bool isFluids, bool isWall) final override;
		void pushConstraintData(const std::vector<Storm::Vector3> &constraintsVisuData) final override;

	public:
		void createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr) final override;
		void updateGraphicsField(std::vector<std::pair<std::wstring_view, std::wstring>> &&rawFields) final override;
		void updateGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValue) final override;

	public:
		void convertScreenPositionToRay(const Storm::Vector2 &screenPos, Storm::Vector3 &outRayOrigin, Storm::Vector3 &outRayDirection) const final override;
		void getClippingPlaneValues(float &outZNear, float &outZFar) const final override;

		Storm::Vector3 get3DPosOfScreenPixel(const Storm::Vector2 &screenPos) const final override;

	public:
		void safeSetSelectedParticle(unsigned int particleSystemId, std::size_t particleIndex) final override;
		void safeClearSelectedParticle() final override;

	public:
		const Storm::Camera& getCamera() const;
		const Storm::DirectXController& getController() const;

		std::size_t getFieldCount() const;

	public:
		void changeBlowerState(const std::size_t blowerId, const Storm::BlowerState newState) final override;

	public:
		bool hasSelectedParticle() const;

	private:
		unsigned char _renderCounter;

		std::unique_ptr<Storm::DirectXController> _directXController;
		std::unique_ptr<Storm::Camera> _camera;

		std::vector<std::unique_ptr<Storm::IRenderedElement>> _renderedElements;
		std::map<unsigned int, std::unique_ptr<Storm::GraphicRigidBody>> _meshesMap;
		std::unique_ptr<Storm::GraphicParticleSystem> _graphicParticlesSystem;
		std::map<std::size_t, std::unique_ptr<Storm::GraphicBlower>> _blowersMap;
		std::unique_ptr<Storm::GraphicConstraintSystem> _graphicConstraintsSystem;

		std::map<std::wstring_view, std::wstring> _fieldsMap;

		std::pair<unsigned int, std::size_t> _selectedParticle;

		std::thread _renderThread;
	};
}
