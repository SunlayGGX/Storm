#pragma once

#include "Singleton.h"
#include "IGraphicsManager.h"


namespace Storm
{
	class DirectXController;
	class Camera;
	class IRenderedElement;

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
		const Storm::Camera& getCamera() const;

	private:
		unsigned char _renderCounter;

		std::unique_ptr<Storm::DirectXController> _directXController;
		std::unique_ptr<Storm::Camera> _camera;

		std::vector<std::unique_ptr<Storm::IRenderedElement>> _renderedElements;

		std::thread _renderThread;
	};
}
