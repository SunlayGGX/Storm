#include "ShaderMacroItem.h"


Storm::ShaderMacroItem::ShaderMacroItem(const std::string_view &macroName, std::string &&macroValue) :
	_macroName{ macroName },
	_macroValue{ std::move(macroValue) }
{
	if (_macroName.empty())
	{
		Storm::throwException<std::exception>("Macro name shouldn't be empty!");
	}
}

const std::string_view& Storm::ShaderMacroItem::getMacroName() const noexcept
{
	return _macroName;
}

const std::string& Storm::ShaderMacroItem::getMacroValue() const noexcept
{
	return _macroValue;
}

void Storm::ShaderMacroItem::appendToCachedName(std::string &inOutCachedNameStr) const
{
	inOutCachedNameStr += '_';
	inOutCachedNameStr += this->getMacroName();
	inOutCachedNameStr += '_';
	inOutCachedNameStr += this->getMacroValue();
}

std::size_t Storm::ShaderMacroItem::getCacheNameStrSize() const noexcept
{
	return _macroName.size() + _macroValue.size() + 2;
}

Storm::ShaderMacroContainer::ShaderMacroContainer()
{
	_shaderMacros.reserve(2);
}

const std::vector<Storm::ShaderMacroItem>& Storm::ShaderMacroContainer::getMacros() const noexcept
{
	return _shaderMacros;
}

std::string Storm::ShaderMacroContainer::toCachedName() const
{
	class CustomShaderMacroCacheParserPolicy
	{
	public:
		static std::string parsePolicyAgnostic(const std::vector<Storm::ShaderMacroItem> &macroItemContainer)
		{
			std::string result;

			if (!macroItemContainer.empty())
			{
				std::size_t finalStrSize = 0;
				for (const Storm::ShaderMacroItem &item : macroItemContainer)
				{
					finalStrSize += item.getCacheNameStrSize();
				}

				result.reserve(finalStrSize);

				for (const Storm::ShaderMacroItem &item : macroItemContainer)
				{
					item.appendToCachedName(result);
				}
			}

			return result;
		}
	};

	return Storm::toStdString<CustomShaderMacroCacheParserPolicy>(this->getMacros());
}
