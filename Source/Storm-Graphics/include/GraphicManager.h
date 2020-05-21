#pragma once

#include "Singleton.h"
#include "IGraphicsManager.h"


namespace Storm
{
	class GraphicManager :
		private Storm::Singleton<GraphicManager>,
		public Storm::IGraphicsManager
	{
		STORM_DECLARE_SINGLETON(GraphicManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();
	};
}
