#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class XmlReader : private Storm::NonInstanciable
	{
	private:
		template<class Type, class ... Args>
		struct TypeSelector
		{
			using Selected = Type;
		};

		template<class Type, class Type2>
		struct TypeSelector<Type, Type2>
		{
			using Selected = Type2;
		};

		template<class Ty, class ... Args>
		struct FirstArgSelector
		{
			using Type = std::remove_reference_t<Ty>;
		};

		template<class Fallback, class Type1> struct FuncArgAnalyzer
		{
			using FirstArg = Fallback;
		};

		template<class Fallback, class RetType, class ... Args>
		struct FuncArgAnalyzer<Fallback, RetType(Args...)>
		{
			using FirstArg = typename FirstArgSelector<Args...>::Type;
		};

		template<class Fallback, class RetType, class ... Args>
		struct FuncArgAnalyzer<Fallback, RetType(*)(Args...)>
		{
			using FirstArg = typename FirstArgSelector<Args...>::Type;
		};

		template<class Fallback, class RetType, class ObjectType, class ... Args>
		struct FuncArgAnalyzer<Fallback, RetType(ObjectType::*)(Args...)>
		{
			using FirstArg = typename FirstArgSelector<Args...>::Type;
		};

	private:
		template<class OverridenAutoType, class ConverterFunc, class TreeItem>
		static auto callConversion(const ConverterFunc &converter, const TreeItem &item, int) -> decltype(converter(item))
		{
			return converter(item);
		}

		template<class OverridenAutoType, class ConverterFunc, class TreeItem>
		static auto callConversion(const ConverterFunc &converter, const TreeItem &item, void*) -> decltype(converter(item.template get_value<OverridenAutoType>()))
		{
			return converter(item.get_value<OverridenAutoType>());
		}

		template<class OverridenAutoType, class ConverterFunc, class TreeItem>
		constexpr static void callConversion(const ConverterFunc &, const TreeItem &, ...)
		{
			STORM_COMPILE_ERROR("Converter not handled!");
		}

	public:
		template<class ValueType>
		static ValueType&& defaultValidator(ValueType&& val)
		{
			return std::forward<ValueType>(val);
		}

	public:
		template<class ... ValueTypeOverride, class XmlTreePair, class ValueType, class SelectedType = TypeSelector<ValueType, ValueTypeOverride...>::Selected, class ValidatorFunc = decltype(&XmlReader::defaultValidator<SelectedType>)>
		static bool handleXml(const XmlTreePair &treePair, const std::string_view &tag, ValueType &val, ValidatorFunc converter = &XmlReader::defaultValidator<SelectedType>)
		{
			// Could be cleaner, but with the time I had in hand, fast (therefore ugly) metaP is the only thing I could do.
			using OverridenAutoType = FuncArgAnalyzer<SelectedType, ValidatorFunc>::template FirstArg;

			if (treePair.first == tag)
			{
				LOG_DEBUG << "Parsing xml tag " << tag;
				val = callConversion<OverridenAutoType>(converter, treePair.second, 0);
				return true;
			}

			return false;
		}

		template<class ResultType, class TreeItem>
		static void sureReadXmlAttribute(const TreeItem &treeItem, ResultType &value, const std::string_view &tag)
		{
			const std::string_view xmlAttrIdentificator{ "<xmlattr>." };

			std::string finalTag;
			finalTag.reserve(xmlAttrIdentificator.size() + tag.size());

			finalTag += xmlAttrIdentificator;
			finalTag += tag;

			value = treeItem.get<ResultType>(finalTag);
		}
	};
}
