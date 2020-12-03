#include "AsyncActionExecutor.h"


void Storm::AsyncActionExecutor::execute()
{
	if (_internalCurrentFrameAsyncActions.empty())
	{
		return;
	}

	Storm::prettyCallMultiCallback(_internalCurrentFrameAsyncActions);
	this->clear();

	if (!_internalNextFrameAsyncActions.empty())
	{
		_internalNextFrameAsyncActions.transfertTo(_internalCurrentFrameAsyncActions);
	}
}

void Storm::AsyncActionExecutor::clear()
{
	_internalCurrentFrameAsyncActions.clear();
}

void Storm::AsyncActionExecutor::bind(Storm::AsyncAction &&action)
{
	_internalCurrentFrameAsyncActions.add(std::move(action));
}

void Storm::AsyncActionExecutor::bindDeffered(Storm::AsyncAction &&action)
{
	_internalNextFrameAsyncActions.add(std::move(action));
}
