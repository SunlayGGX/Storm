#pragma once


#include "FacetsContainer.h"
#include "Singleton.h"
#include "SingletonDefaultImplementation.h"


#if STORM_SINGLETON_HOLDER_STORAGE_TYPE_MODE == 0
#	define STORM_SINGLETON_HOLDER_STORAGE_TYPE Storm::FacetContainerUseMapStorage
#elif STORM_SINGLETON_HOLDER_STORAGE_TYPE_MODE == 1
#	define STORM_SINGLETON_HOLDER_STORAGE_TYPE Storm::FacetContainerUseContiguousDynamicStorage
#elif STORM_SINGLETON_HOLDER_STORAGE_TYPE_MODE == 2
#	define STORM_SINGLETON_HOLDER_STORAGE_TYPE Storm::FacetContainerUseContiguousStaticStorage<64> // This singleton count should be enough, I think... If not, we can increase it.
#else
#	error STORM_SINGLETON_HOLDER_STORAGE_TYPE_MODE value is unknown !
#endif

namespace Storm
{
	class SingletonHeldInterfaceBase;

	class SingletonHolder :
		private Storm::Singleton<Storm::SingletonHolder, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::FacetContainer<Storm::SingletonHeldInterfaceBase, Storm::FacetContainerIsNotOwner, STORM_SINGLETON_HOLDER_STORAGE_TYPE>
	{
		STORM_DECLARE_SINGLETON(SingletonHolder);

	public:
		template<class SingletonType>
		SingletonType& getSingleton() const
		{
			return *this->getFacet<SingletonType>();
		}
	};
}

#undef STORM_SINGLETON_HOLDER_STORAGE_TYPE
