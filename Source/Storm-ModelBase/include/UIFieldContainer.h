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
		
		UIFieldContainer& deleteFieldW(const std::wstring_view &fieldName);

		void push() const;
		void pushFieldW(const std::wstring_view &fieldName) const;

	private:
		static void createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr);
		static void deleteGraphicsField(const std::wstring_view &fieldName);

	private:
		std::vector<std::unique_ptr<Storm::UIFieldBase>> _fields;
	};

	template<class FieldType, class UpdatedFieldType>
	void updateFieldW(Storm::UIFieldContainer &uifields, FieldType &inOutField, UpdatedFieldType &&newFieldValue, const std::wstring_view &fieldName)
	{
		if (inOutField != newFieldValue)
		{
			inOutField = std::forward<UpdatedFieldType>(newFieldValue);
			uifields.pushFieldW(fieldName);
		}
	}

#ifndef bindField
#	define bindField(fieldName, valueRef) bindFieldW(STORM_TEXT(fieldName), valueRef)
#	define deleteField(fieldName) deleteFieldW(STORM_TEXT(fieldName))
#	define pushField(fieldName) pushFieldW(STORM_TEXT(fieldName))
#	define updateField(uiFields, fieldName, valueRef, newValue) Storm::updateFieldW(uiFields, valueRef, newValue, STORM_TEXT(fieldName))
#endif
}
