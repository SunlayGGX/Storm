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
		void bindDeffered(Storm::AsyncAction &&action);

	private:
		Storm::MultiCallback<Storm::AsyncAction> _internalCurrentFrameAsyncActions;
		Storm::MultiCallback<Storm::AsyncAction> _internalNextFrameAsyncActions;
	};
}
