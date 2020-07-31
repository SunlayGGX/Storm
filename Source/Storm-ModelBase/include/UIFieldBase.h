#pragma once


namespace Storm
{
	class UIFieldBase
	{
	public:
		UIFieldBase(const std::wstring_view &fieldName);

	public:
		const std::wstring_view& getFieldName() const noexcept;

		virtual std::wstring getFieldValue() const = 0;

	protected:
		std::wstring_view _fieldName;
	};
}