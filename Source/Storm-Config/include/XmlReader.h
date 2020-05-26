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
				val = converter(treePair.second.get_value<OverridenAutoType>());
				return true;
			}

			return false;
		}
	};
}
