#pragma once


namespace Storm
{
	class UIFieldBase;

	class UIFieldContainer
	{
	public:
		template<class FieldType>
		UIFieldContainer& bindFieldW(const std::wstring_view &fieldName, const FieldType &valueRef)
		{
			std::unique_ptr<Storm::UIField<FieldType>> fieldItem = std::make_unique<Storm::UIField<FieldType>>(fieldName, valueRef);
			std::wstring initialValueStr = fieldItem->getFieldValue();

			_fields.emplace_back(std::move(fieldItem));
			UIFieldContainer::createGraphicsField(fieldName, std::move(initialValueStr));
			return *this;
		}

		void push() const;
		void pushFieldW(const std::wstring_view &fieldName) const;

	private:
		static void createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr);

	private:
		std::vector<std::unique_ptr<Storm::UIFieldBase>> _fields;
	};

#ifndef bindField
#	define bindField(fieldName, valueRef) bindFieldW(STORM_TEXT(fieldName), valueRef)
#	define pushField(fieldName) pushFieldW(STORM_TEXT(fieldName))
#endif
}
