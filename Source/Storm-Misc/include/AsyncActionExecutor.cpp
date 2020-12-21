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
	_internalNextFrameAsyncActions.add(std::move(action));
}

void Storm::AsyncActionExecutor::prepare()
{
	if (!_internalNextFrameAsyncActions.empty())
	{
		_internalNextFrameAsyncActions.transfertTo(_internalCurrentFrameAsyncActions);
	}
}

void Storm::AsyncActionExecutor::executeFastNoBind(Storm::AsyncAction &&action)
{
	Storm::MultiCallback<Storm::AsyncAction> tmp;
	tmp.add(std::move(action));
	Storm::prettyCallMultiCallback(tmp);
}
