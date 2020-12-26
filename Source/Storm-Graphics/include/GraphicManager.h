#pragma once

#include "Singleton.h"
#include "IGraphicsManager.h"
#include "DeclareScriptableItem.h"


namespace Storm
{
	class DirectXController;
	class Camera;
	class IRenderedElement;
	class GraphicRigidBody;
	class GraphicParticleSystem;
	class GraphicBlower;
	class GraphicConstraintSystem;
	class ParticleForceRenderer;
	class GraphicPipe;

	struct WithUI;
	struct NoUI;

	class GraphicManager final :
		private Storm::Singleton<GraphicManager>,
		public Storm::IGraphicsManager
	{
		STORM_DECLARE_SINGLETON(GraphicManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		bool initialize_Implementation(const Storm::WithUI &);
		void initialize_Implementation(const Storm::NoUI &);
		void initialize_Implementation(void* hwnd);
		void cleanUp_Implementation(const Storm::WithUI &);
		void cleanUp_Implementation(const Storm::NoUI &);

	public:
		void update() final override;

	public:
		bool isActive() const noexcept;

	public:
		void addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes) final override;
		void bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const final override;

	public:
		void loadBlower(const Storm::BlowerData &blowerData, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes) final override;

	public:
		void pushParticlesData(const Storm::PushedParticleSystemDataParameter &param) final override;
		void pushConstraintData(const std::vector<Storm::Vector3> &constraintsVisuData) final override;
		void pushParticleSelectionForceData(const Storm::Vector3 &selectedParticlePos, const Storm::Vector3 &selectedParticleForce) final override;

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
		void setTargetPositionTo(const Storm::Vector3 &newTargetPosition) final override;

	public:
		void changeBlowerState(const std::size_t blowerId, const Storm::BlowerState newState) final override;

	public:
		bool hasSelectedParticle() const;

	public:
		void notifyViewportRescaled(int newWidth, int newHeight);

	public:
		void cycleColoredSetting() final override;
		void setColorSettingMinMaxValue(float minValue, float maxValue) final override;

	private:
		unsigned char _renderCounter;

		bool _hasUI;

		std::unique_ptr<Storm::DirectXController> _directXController;
		std::unique_ptr<Storm::Camera> _camera;

		std::vector<std::unique_ptr<Storm::IRenderedElement>> _renderedElements;
		std::map<unsigned int, std::unique_ptr<Storm::GraphicRigidBody>> _meshesMap;
		std::unique_ptr<Storm::GraphicParticleSystem> _graphicParticlesSystem;
		std::map<std::size_t, std::unique_ptr<Storm::GraphicBlower>> _blowersMap;
		std::unique_ptr<Storm::GraphicConstraintSystem> _graphicConstraintsSystem;
		std::unique_ptr<Storm::ParticleForceRenderer> _forceRenderer;

		std::map<std::wstring_view, std::wstring> _fieldsMap;

		std::unique_ptr<Storm::GraphicPipe> _pipe;
		std::pair<unsigned int, std::size_t> _selectedParticle;

		std::thread _renderThread;

		unsigned short _windowsResizedCallbackId;
	};
}
