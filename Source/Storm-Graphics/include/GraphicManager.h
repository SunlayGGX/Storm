#pragma once

#include "Singleton.h"
#include "IGraphicsManager.h"


namespace Storm
{
	class DirectXController;
	class Camera;
	class IRenderedElement;
	class GraphicRigidBody;

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
		const Storm::Camera& getCamera() const;

	private:
		unsigned char _renderCounter;

		std::unique_ptr<Storm::DirectXController> _directXController;
		std::unique_ptr<Storm::Camera> _camera;

		std::vector<std::unique_ptr<Storm::IRenderedElement>> _renderedElements;
		std::map<unsigned int, std::unique_ptr<Storm::GraphicRigidBody>> _meshesMap;

		std::mutex _actionMutex;
		std::vector<Storm::GraphicsAction> _actionsToBeExecuted;

		std::thread _renderThread;
	};
}
