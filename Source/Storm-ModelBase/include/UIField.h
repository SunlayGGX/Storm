#pragma once

#include "UIFieldBase.h"


namespace Storm
{
	template<class FieldType>
	class UIField : public UIFieldBase
	{
	public:
		UIField(const std::wstring_view &fieldName, const FieldType &fieldValueRef) :
			UIFieldBase{ fieldName },
			_fieldValueRef{ fieldValueRef }
		{}

	private:
		template<class Field>
		static auto parseToWString(const Field &val, int, int) -> decltype(CustomFieldParser::parseToWString(val))
		{
			return CustomFieldParser::parseToWString(val);
		}

		template<class Field>
		static auto parseToWString(Field* val, int, int) -> decltype(Storm::UIField::parseToWString(*val, 0, 0))
		{
			if (val)
			{
				return Storm::UIField::parseToWString(*val, 0, 0);
			}

			return L"{ null }";
		}

		template<class Field>
		static auto parseToWString(Field val, int, void*) -> decltype(std::enable_if_t<!std::is_same_v<Field, bool>, std::true_type>::value, std::to_wstring(val))
		{
			return std::to_wstring(val);
		}

		template<class Field>
		static auto parseToWString(bool val, int, void*) -> decltype(std::enable_if_t<std::is_same_v<Field, bool>, std::true_type>::value, std::wstring())
		{
			return val ? L"true" : L"false";
		}

		template<class Field>
		static auto parseToWString(const std::atomic<Field> &val, int, int) -> decltype(UIField::parseToWString(val.load(), 0, 0))
		{
			return UIField::parseToWString(val.load(), 0, 0);
		}

		template<class Field>
		static auto parseToWString(const Field &val, void*, int) -> decltype(std::filesystem::path{ val }.wstring())
		{
			return std::filesystem::path{ val }.wstring();
		}

		template<class Field>
		static auto parseToWString(const Field &val, void*, void*) -> decltype(std::filesystem::path{ Storm::toStdString(val) }.wstring())
		{
			return std::filesystem::path{ Storm::toStdString(val) }.wstring();
		}

	public:
		virtual std::wstring getFieldValue() const final override
		{
			return UIField::parseToWString(_fieldValueRef, 0, 0);
		}

	private:
		const FieldType &_fieldValueRef;
	};
}
