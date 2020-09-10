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
		static auto parseToWString(const Field &val, int) -> decltype(CustomFieldParser::parseToWString(val))
		{
			return CustomFieldParser::parseToWString(val);
		}

		template<class Field>
		static auto parseToWString(Field val, int) -> decltype(std::to_wstring(val))
		{
			return std::to_wstring(val);
		}

		template<class Field>
		static auto parseToWString(const std::atomic<Field> &val, int) -> decltype(UIField::parseToWString(val.load(), 0))
		{
			return UIField::parseToWString(val.load(), 0);
		}

		static std::wstring parseToWString(bool val, int)
		{
			return val ? L"true" : L"false";
		}

		template<class Field>
		static auto parseToWString(const Field &val, void*) -> decltype(std::filesystem::path{ val }.wstring())
		{
			return std::filesystem::path{ val }.wstring();
		}

		template<class Field>
		static auto parseToWString(const Field &val, ...) -> decltype(std::filesystem::path{ val }.wstring())
		{
			return std::filesystem::path{ Storm::toStdString(val) }.wstring();
		}

	public:
		virtual std::wstring getFieldValue() const final override
		{
			return UIField::parseToWString(_fieldValueRef, 0);
		}

	private:
		const FieldType &_fieldValueRef;
	};
}
