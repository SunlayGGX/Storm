#include "DynamicMenuBuilder.h"

#include "MFCHelper.h"


Storm::DynamicMenuBuilder::~DynamicMenuBuilder() = default;

void Storm::DynamicMenuBuilder::appendMenu(const HMENU parent, const std::wstring &menuContentName, Storm::DynamicMenuBuilder::MenuFunctionCallback &&callback)
{
	UINT newMenuID;
	if (Storm::MFCHelper::appendNewStringMenu(parent, menuContentName, newMenuID))
	{
		const auto endCallbackRangesIter = std::end(_menuCallbacks);
		if (auto found = std::find_if(std::begin(_menuCallbacks), endCallbackRangesIter, [newMenuID](const auto &rangesCallbackPair)
		{
			return newMenuID == rangesCallbackPair.first + static_cast<UINT>(rangesCallbackPair.second.size()) + 1;
		}); found != endCallbackRangesIter)
		{
			auto &rangesCallbackPair = *found;
			rangesCallbackPair.second.emplace_back(std::move(callback));

			// Maybe the next array was made contiguous to this one. If this is the case, then we need to merge the 2 callback containers.
			// This will prevent fragmentation.
			auto next = found;
			++next;

			if (next != endCallbackRangesIter && next->first == (newMenuID + 1))
			{
				std::vector<Storm::DynamicMenuBuilder::MenuFunctionCallback> &nextCallbacks = next->second;
				std::vector<Storm::DynamicMenuBuilder::MenuFunctionCallback> &currentRangeCallbacks = rangesCallbackPair.second;
				currentRangeCallbacks.reserve(currentRangeCallbacks.size() + nextCallbacks.size());

				std::copy(std::make_move_iterator(std::begin(nextCallbacks)), std::make_move_iterator(std::end(nextCallbacks)), std::back_inserter(currentRangeCallbacks));
				_menuCallbacks.erase(next);
			}
		}
		else
		{
			_menuCallbacks.emplace_back(newMenuID, std::initializer_list<Storm::DynamicMenuBuilder::MenuFunctionCallback>{ callback });
		}
	}
}

bool Storm::DynamicMenuBuilder::operator()(const UINT commandID)
{
	const auto endCallbackRangesIter = std::end(_menuCallbacks);
	if (auto found = std::find_if(std::begin(_menuCallbacks), endCallbackRangesIter, [commandID](const auto &rangesCallbackPair)
	{
		return commandID >= rangesCallbackPair.first && commandID < (rangesCallbackPair.first + static_cast<UINT>(rangesCallbackPair.second.size()));
	}); found != endCallbackRangesIter)
	{
		auto &rangesCallbackPair = *found;
		rangesCallbackPair.second[commandID - rangesCallbackPair.first]();
		return true;
	}

	return false;
}

