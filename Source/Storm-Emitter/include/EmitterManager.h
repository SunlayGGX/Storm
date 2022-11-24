#pragma once


#include "Singleton.h"
#include "IEmitterManager.h"
#include "DeclareScriptableItem.h"


namespace Storm
{
	class EmitterObject;

	class EmitterManager final :
		private Storm::Singleton<Storm::EmitterManager>,
		public Storm::IEmitterManager
	{
		STORM_DECLARE_SINGLETON(EmitterManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void update(float deltaTime) final override;

	public:
		void setEmitterEnabled(unsigned int emitterID, bool enable);
		void setEmitterPauseEmission(unsigned int emitterID, bool pause);

	private:
		std::vector<Storm::EmitterObject> _emitters;
	};
}
