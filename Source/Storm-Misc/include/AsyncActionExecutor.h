#pragma once

#include "MultiCallback.h"
#include "AsyncAction.h"


namespace Storm
{
	class AsyncActionExecutor
	{
	public:
		void execute();
		void clear();
		void bind(Storm::AsyncAction &&action);
		void prepare();

	public:
		static void executeFastNoBind(Storm::AsyncAction &&action);

	private:
		mutable std::mutex _mutex;
		Storm::MultiCallback<Storm::AsyncAction> _internalNextFrameAsyncActions;
		Storm::MultiCallback<Storm::AsyncAction> _internalCurrentFrameAsyncActions;
	};
}
