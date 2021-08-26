#include "UIFieldContainer.h"
#include "UIFieldBase.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"
#include "IThreadManager.h"

#include "FuncMovePass.h"

#include "ThreadEnumeration.h"


Storm::UIFieldBase::UIFieldBase(const std::wstring_view &fieldName) :
	_fieldName{ fieldName }
{

}

const std::wstring_view& Storm::UIFieldBase::getFieldName() const noexcept
{
	return _fieldName;
}

Storm::UIFieldContainer& Storm::UIFieldContainer::deleteFieldW(const std::wstring_view& fieldName)
{
	for (auto &fieldPtr : _fields)
	{
		if (fieldPtr->getFieldName() == fieldName)
		{
			if (&fieldPtr != &_fields.back())
			{
				std::swap(fieldPtr, _fields.back());
			}

			_fields.pop_back();
			UIFieldContainer::deleteGraphicsField(fieldName);

			return *this;
		}
	}

	assert(false && "We cannot remove a field that does not exist!");
	LOG_DEBUG_ERROR << "We cannot remove a field that does not exist!";
	return *this;
}

void Storm::UIFieldContainer::push() const
{
	std::vector<std::pair<std::wstring_view, std::wstring>> tmp;
	tmp.reserve(_fields.size());

	for (const std::unique_ptr<Storm::UIFieldBase> &field : _fields)
	{
		tmp.emplace_back(field->getFieldName(), field->getFieldValue());
	}

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicMgr, fieldRawBuf = Storm::FuncMovePass<decltype(tmp)>{ std::move(tmp) }]() mutable
	{
		graphicMgr.updateGraphicsField(std::move(fieldRawBuf._object));
	});
}

void Storm::UIFieldContainer::pushFieldW(const std::wstring_view &fieldName) const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	for (const std::unique_ptr<Storm::UIFieldBase> &field : _fields)
	{
		if (field->getFieldName() == fieldName)
		{
			Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
			singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicMgr, fieldName, fieldVal = Storm::FuncMovePass<std::wstring>{ std::move(field->getFieldValue()) }]() mutable
			{
				graphicMgr.updateGraphicsField(fieldName, std::move(fieldVal._object));
			});
			return;
		}
	}
}

void Storm::UIFieldContainer::createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>().createGraphicsField(fieldName, std::move(fieldValueStr));
}

void Storm::UIFieldContainer::deleteGraphicsField(const std::wstring_view &fieldName)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>().removeGraphicsField(fieldName);
}
