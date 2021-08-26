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
