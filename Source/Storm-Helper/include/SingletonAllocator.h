#pragma once


#include "SizeCounter.h"
#include "StaticAssertionsMacros.h"
#include "Facets.h"
#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	template<class SingletonType>
	struct SingletonAllocatorHelper
	{
	private:
		template<class RealSingletonType>
		static auto registerSingletonInterfaceIntoHolder(FacetBase<Storm::SingletonHeldInterfaceBase>* singletonInterfacePtr) -> decltype(Storm::SingletonHolder::instance().registerFacet<RealSingletonType>(singletonInterfacePtr), void())
		{
			if (Storm::SingletonHolder::isAlive())
			{
				Storm::SingletonHolder::instance().registerFacet<RealSingletonType>(singletonInterfacePtr);
			}
		}

		// This one is when there is no SingletonInterfaceHolder defined in the project... (The feature doesn't exist)
		template<class RealSingletonType>
		static constexpr void registerSingletonInterfaceIntoHolder(void*)
		{

		}

		template<class RealSingletonType>
		static auto unregisterSingletonInterfaceIntoHolder(int) -> decltype(Storm::SingletonHolder::instance().template unregisterFacet<RealSingletonType>(), void())
		{
			if (Storm::SingletonHolder::isAlive())
			{
				Storm::SingletonHolder::instance().unregisterFacet<RealSingletonType>();
			}
		}

		// This one is when there is no SingletonInterfaceHolder defined in the project... (The feature doesn't exist)
		template<class RealSingletonType>
		static constexpr void unregisterSingletonInterfaceIntoHolder(void*)
		{

		}


		template<class RealSingletonType>
		static auto registerScripting(RealSingletonType* singletonPtr)
			-> decltype(singletonPtr->registerCurrentOnScript(std::declval<Storm::ScriptManager::UsedScriptWrapper>()), void())
		{
			if (Storm::ScriptManager::isAlive())
			{
				Storm::ScriptManager &scriptMgr = Storm::ScriptManager::instance();
				auto &scriptWrapper = scriptMgr.getScriptWrapper();
				singletonPtr->registerCurrentOnScript(scriptWrapper);
			}
		}

		// This one is when there is no Scripting defined in the project... (The feature doesn't exist)
		template<class RealSingletonType>
		static constexpr void registerScripting(void*)
		{

		}

	public:
		static void allocate(uint8_t* &memoryLocation)
		{
			assert(!SingletonType::isAlive() && "We mustn't allocate again a singleton.");
			SingletonType::s_instance = new(static_cast<void*>(memoryLocation))SingletonType;
			SingletonAllocatorHelper<SingletonType>::registerSingletonInterfaceIntoHolder<SingletonType>(SingletonType::s_instance);
			SingletonAllocatorHelper<SingletonType>::registerScripting<SingletonType>(SingletonType::s_instance);

			memoryLocation += Storm::SizeCounter<SingletonType>::value;
		}

		static void deallocate()
		{
			assert(SingletonType::isAlive() && "We cannot deallocate a non existing singleton.");
			SingletonType::s_instance->~SingletonType();
			SingletonType::s_instance = nullptr;
			SingletonAllocatorHelper<SingletonType>::unregisterSingletonInterfaceIntoHolder<SingletonType>(0);
		}
	};


	struct SingletonAllocatorBase
	{
		static inline std::mutex s_globalSingletonAllocMutex;
	};

	template<class ... Singletons>
	class SingletonAllocator : private SingletonAllocatorBase
	{
	private:
		STORM_STATIC_ASSERT(sizeof...(Singletons) > 0, "We should have at least one or more singletons.");

		enum : std::size_t
		{
			k_internalBufferSize = Storm::SizeCounter<Singletons...>::value
		};

	private:
		template<class CurrentSingleton, class ... RemainingSingletons>
		struct Internal
		{
			static void allocate(uint8_t* &memoryLocation)
			{
				Internal<CurrentSingleton>::allocate(memoryLocation);
				try
				{
					Internal<RemainingSingletons...>::allocate(memoryLocation);
				}
				catch (...)
				{
					Internal<CurrentSingleton>::deallocate();
					throw;
				}
			}

			static void deallocate()
			{
				Internal<RemainingSingletons...>::deallocate();
				Internal<CurrentSingleton>::deallocate();
			}
		};

		template<class CurrentSingleton>
		struct Internal<CurrentSingleton>
		{
			static void allocate(uint8_t* &memoryLocation)
			{
				SingletonAllocatorHelper<CurrentSingleton>::allocate(memoryLocation);
			}

			static void deallocate()
			{
				SingletonAllocatorHelper<CurrentSingleton>::deallocate();
			}
		};

	public:
		SingletonAllocator()
		{
			uint8_t* current = _internalBuffer;

			std::lock_guard<std::mutex> lock{ s_globalSingletonAllocMutex };
			SingletonAllocator::Internal<Singletons...>::allocate(current);

			assert(current == std::end(_internalBuffer));
		}

		~SingletonAllocator()
		{
			std::lock_guard<std::mutex> lock{ s_globalSingletonAllocMutex };
			SingletonAllocator::Internal<Singletons...>::deallocate();
		}

	private:
		uint8_t _internalBuffer[k_internalBufferSize];
	};
}
