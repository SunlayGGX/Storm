#pragma once


#include "Singleton.h"
#include "IEmitterManager.h"


namespace Storm
{
	class EmitterObject;

	class EmitterManager final :
		private Storm::Singleton<Storm::EmitterManager>,
		public Storm::IEmitterManager
	{
		STORM_DECLARE_SINGLETON(EmitterManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void update(float deltaTime) final override;

	private:
		std::vector<Storm::EmitterObject> _emitters;
	};
}
