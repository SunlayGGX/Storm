#pragma once

#include "SingletonHeldInterfaceBase.h"
#include "AsyncAction.h"


namespace Storm
{
	enum class ThreadEnumeration;
	enum class ThreadPriority;

	class IThreadManager : public Storm::ISingletonHeldInterface<IThreadManager>
	{
	public:
		virtual ~IThreadManager() = default;

	public:
		virtual void registerCurrentThread(Storm::ThreadEnumeration threadEnum, const std::wstring &newName) = 0;
		virtual void executeOnThread(const std::thread::id &threadId, Storm::AsyncAction &&action) = 0;
		virtual void executeOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action) = 0;
		virtual void executeDefferedOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action) = 0;
		virtual void processCurrentThreadActions() = 0;
		virtual void processActionsOfThread(Storm::ThreadEnumeration threadEnum) = 0; // This is only to be used if you know what you are doing.

		// Clear without processing any actions.
		virtual void clearCurrentThreadActions() = 0;

		virtual bool isExecutingOnThread(Storm::ThreadEnumeration threadEnum) const = 0;

		virtual void setCurrentThreadPriority(const Storm::ThreadPriority priority) const = 0;
	};

#define STORM_REGISTER_THREAD(ThreadEnum) Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().registerCurrentThread(Storm::ThreadEnumeration::ThreadEnum, L#ThreadEnum)
}
