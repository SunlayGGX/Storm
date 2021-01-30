#pragma once


namespace Storm
{
	class ShaderMacroItem
	{
	private:
		ShaderMacroItem(const std::string_view &macroName, std::string &&macroValue);

	public:
		template<class ValueType>
		ShaderMacroItem(const std::string_view &macroName, ValueType &&value) :
			Storm::ShaderMacroItem{ macroName, Storm::toStdString(std::forward<ValueType>(value)) }
		{}

	public:
		const std::string_view& getMacroName() const noexcept;
		const std::string& getMacroValue() const noexcept;

	public:
		// Make the cached name portion used for shader cache file name.
		void appendToCachedName(std::string &inOutCachedNameStr) const;

		std::size_t getCacheNameStrSize() const noexcept;

	private:
		std::string_view _macroName;
		std::string _macroValue;
	};

	class ShaderMacroContainer
	{
	public:
		ShaderMacroContainer();

	public:
		template<class ValueType>
		ShaderMacroContainer& addMacro(const std::string_view &macroName, ValueType &&value)
		{
			_shaderMacros.emplace_back(macroName, std::forward<ValueType>(value));
			return *this;
		}

	public:
		const std::vector<Storm::ShaderMacroItem>& getMacros() const noexcept;
		std::string toCachedName() const;
		void reserve(const std::size_t count);

	private:
		std::vector<Storm::ShaderMacroItem> _shaderMacros;
	};
}
