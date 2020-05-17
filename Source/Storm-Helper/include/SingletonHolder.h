#pragma once


#include "FacetsContainer.h"
#include "Singleton.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
    class SingletonHeldInterfaceBase;

    class SingletonHolder :
        private Storm::Singleton<Storm::SingletonHolder, Storm::DefineDefaultInitAndCleanupImplementation>,
        public Storm::FacetContainer<Storm::SingletonHeldInterfaceBase, Storm::FacetContainerIsNotOwner>
    {
        STORM_DECLARE_SINGLETON(SingletonHolder);
    };
}
