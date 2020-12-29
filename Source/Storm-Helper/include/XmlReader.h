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
		template<class OverridenAutoType, class ConverterFunc, class TreeItem, class ValueType>
		static auto callConversion(const ConverterFunc &converter, const TreeItem &item, ValueType &val, int, int)
			-> decltype(converter(item, val), void())
		{
			converter(item, val);
		}

		template<class OverridenAutoType, class ConverterFunc, class TreeItem, class ValueType>
		static auto callConversion(const ConverterFunc &converter, const TreeItem &item, ValueType &val, int, void*)
			-> decltype(val = converter(item), void())
		{
			val = converter(item);
		}

		template<class OverridenAutoType, class ConverterFunc, class TreeItem, class ValueType>
		static auto callConversion(const ConverterFunc &converter, const TreeItem &item, ValueType &val, void*, int)
			-> decltype(val = converter(item.template get_value<OverridenAutoType>()), void())
		{
			val = converter(item.get_value<OverridenAutoType>());
		}

		template<class OverridenAutoType, class ConverterFunc, class TreeItem, class ValueType>
		constexpr static void callConversion(const ConverterFunc &, const TreeItem &, ValueType &, ...)
		{
			STORM_COMPILE_ERROR("Converter not handled!");
		}

	public:
		template<class ValueType>
		static auto defaultValidator(ValueType&& val)
		{
			return val;
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
				callConversion<OverridenAutoType>(converter, treePair.second, val, 0, 0);
				return true;
			}

			return false;
		}

		template<class ResultType, class TreeItem>
		static void readXmlAttribute(const TreeItem &treeItem, ResultType &value, const std::string_view &tag)
		{
			constexpr std::string_view xmlAttrIdentificator{ "<xmlattr>." };

			std::string finalTag;
			finalTag.reserve(xmlAttrIdentificator.size() + tag.size());

			finalTag += xmlAttrIdentificator;
			finalTag += tag;

			if (auto maybeAttr = treeItem.get_optional<ResultType>(finalTag); maybeAttr.has_value())
			{
				value = maybeAttr.value();
			}
		}

		template<class ResultType, class TreeItem>
		static void sureReadXmlAttribute(const TreeItem &treeItem, ResultType &value, const std::string_view &tag)
		{
			constexpr std::string_view xmlAttrIdentificator{ "<xmlattr>." };

			std::string finalTag;
			finalTag.reserve(xmlAttrIdentificator.size() + tag.size());

			finalTag += xmlAttrIdentificator;
			finalTag += tag;

			value = treeItem.get<ResultType>(finalTag);
		}

	public:
		template<class XmlTreeType, class DataContainerType, class XmlHandlerFunc, class PostLoadFunc>
		static bool readDataInList(const XmlTreeType &srcTree, const std::string &listTag, const std::string_view &elementTag, DataContainerType &inOutDataArray, const XmlHandlerFunc &xmlHandler, const PostLoadFunc &postLoad)
		{
			const auto &treeOptValue = srcTree.get_child_optional(listTag);
			if (treeOptValue.has_value())
			{
				const auto &treeValue = treeOptValue.value();

				inOutDataArray.reserve(inOutDataArray.size() + treeValue.size());

				for (const auto &dataXmlElement : treeValue)
				{
					if (dataXmlElement.first == elementTag)
					{
						auto &data = inOutDataArray.emplace_back();
						for (const auto &insideDataXml : dataXmlElement.second)
						{
							if (!xmlHandler(insideDataXml, data))
							{
								LOG_ERROR << "tag '" << insideDataXml.first << "' (inside Scene." << listTag << '.' << elementTag << ") is unknown, therefore it cannot be handled";
							}
						}

						postLoad(data);
					}
					else
					{
						LOG_ERROR << dataXmlElement.first + " is not a valid tag inside '" << listTag << "'. Only '" << elementTag << "' is accepted!";
					}
				}

				return true;
			}

			return false;
		}
	};
}
