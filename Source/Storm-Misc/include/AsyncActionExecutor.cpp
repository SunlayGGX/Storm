#include "AsyncActionExecutor.h"


void Storm::AsyncActionExecutor::execute()
{
	if (_internalCurrentFrameAsyncActions.empty())
	{
		return;
	}

	Storm::prettyCallMultiCallback(_internalCurrentFrameAsyncActions);
	this->clear();
}

void Storm::AsyncActionExecutor::clear()
{
	_internalCurrentFrameAsyncActions.clear();
}

void Storm::AsyncActionExecutor::bind(Storm::AsyncAction &&action)
{
	_internalCurrentFrameAsyncActions.add(std::move(action));
}
