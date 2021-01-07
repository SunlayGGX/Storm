#pragma once

#include "Singleton.h"
#include "IBibliographyManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class BibliographyManager final :
		private Storm::Singleton<Storm::BibliographyManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IBibliographyManager
	{
		STORM_DECLARE_SINGLETON(BibliographyManager);

	public:
		void generateBibTexLibrary() const final override;
	};
}
