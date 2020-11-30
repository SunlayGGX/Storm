#pragma once
#include <type_traits>


namespace Storm
{
	// Built-in Policy

	struct DefaultPolicy {};
	struct DebugPolicy {};
	struct NumericPolicy {};


	namespace details
	{
		template<class Policy, class ValueType> decltype(auto) toStdString(ValueType &&val);

		template<class ParserType, class ValueType>
		struct ParserValidator
		{
		private:
			template<class Type, class ValType>
			constexpr static auto shouldUseSFINAE(int) -> decltype(Type::parse(std::declval<ValType>()), bool()) { return true; }

			template<class Type, class ValType>
			constexpr static bool shouldUseSFINAE(void*) { return false; }

		public:
			enum
			{
				shouldUse = shouldUseSFINAE<ParserType, ValueType>(0)
			};
		};

		// 3 ways to parse :
		// - parse<Policy> : 1st priority. Parse the value type, used Policy is re injected, therefore not lost
		// (useful when there is Policy inheritage involved (to forward the real Policy, and not the Policy we came into that could be a Parent)).
		// This is the default that should be used. Advantage is that Policy is really forwarded. Disadvantage is that because it is a template declaration, the Parser cannot be declared inside a function.
		// - parsePolicyAgnostic : 2nd priority. This is the parsing that does not receive a Policy template. It is useful for Parser on the fly declared inside methods for a local purpose.
		// - parseAppending : Last priority. Instead of returning a new string that is the result of the parsing, this append the parsing result chunks to the resulting final string. Preventing on case we parse a container, uselesses allocation/reallocation/deallocation that could be costly.
		template<class Policy, class ValueType>
		struct CustomPolicyParser
		{
		private:
			template<class ValType>
			static auto parseImpl(ValType &&val, int, int) -> decltype(Policy::template parse<Policy>(std::forward<ValType>(val)))
			{
				return Policy::template parse<Policy>(std::forward<ValType>(val));
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, void*, int) -> decltype(Policy::parsePolicyAgnostic(std::forward<ValType>(val)))
			{
				return Policy::parsePolicyAgnostic(std::forward<ValType>(val));
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, void*, void*) -> decltype(Policy::template parseAppending<Policy>(std::declval<std::string>(), std::forward<ValType>(val)), std::string{})
			{
				std::string result;
				Policy::template parseAppending<Policy>(result, std::forward<ValType>(val));
				return result;
			}

		public:

			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0, 0))
			{
				return parseImpl(std::forward<ValType>(val), 0, 0);
			}
		};

		template<class Policy, class ValueType>
		struct NativeParser
		{
		private:
			template<class BoolType>
			static auto parseImpl(BoolType val, int) -> decltype(std::enable_if_t<std::is_same_v<BoolType, bool>, std::true_type>::value, std::string{})
			{
				if constexpr (std::is_same_v<Policy, Storm::NumericPolicy>)
				{
					return val ? "1" : "0";
				}
				else
				{
					return val ? "true" : "false";
				}
			}

			template<class RawPtrType>
			static auto parseImpl(RawPtrType val, int) -> decltype(val != nullptr, Storm::details::toStdString<Policy>(*val), std::string{})
			{
				if (val != nullptr)
				{
					return Storm::details::toStdString<Policy>(*val);
				}
				else
				{
					return Storm::details::toStdString<Policy>(nullptr);
				}
			}

			template<class NumericType>
			static auto parseImpl(NumericType val, void*) -> decltype(std::to_string(val))
			{
				return std::to_string(val);
			}

		public:
			template<class ValType>
			static auto parse(ValType val) -> decltype(parseImpl(val, 0))
			{
				return parseImpl(val, 0);
			}
		};

		template<class Policy, class ValueType>
		struct StandardParser
		{
		private:
			template<class SmartPtrType>
			static auto parseImpl(const SmartPtrType &val, int) -> decltype(Storm::details::toStdString<Policy>(val.get()))
			{
				return Storm::details::toStdString<Policy>(val.get());
			}

			template<class PairType>
			static auto parseImpl(PairType &&val, int) -> decltype(val.first, val.second, std::string{})
			{
				if constexpr (std::is_rvalue_reference_v<PairType>)
				{
					if constexpr (std::is_convertible_v<Policy, Storm::DebugPolicy>)
					{
						return "{ " + Storm::details::toStdString<Policy>(std::move(val.first)) + ',' + Storm::details::toStdString<Policy>(std::move(val.second)) + " }";
					}
					else
					{
						return Storm::details::toStdString<Policy>(std::move(val.second));
					}
				}
				else
				{
					if constexpr (std::is_convertible_v<Policy, Storm::DebugPolicy>)
					{
						return "{ " + Storm::details::toStdString<Policy>(val.first) + ',' + Storm::details::toStdString<Policy>(val.second) + " }";
					}
					else
					{
						return Storm::details::toStdString<Policy>(val.second);
					}
				}
			}

			// To convert std::wstring, std::u16string, std::u32string, std::u8string and their reduced buffer.
			template<class StringType>
			static auto parseImpl(StringType &&val, int) -> decltype(std::filesystem::path{ std::forward<StringType>(val) }.string())
			{
				return std::filesystem::path{ std::forward<StringType>(val) }.string();
			}

			template<class IteratorType>
			static auto parseImpl(const IteratorType &val, void*)
				-> decltype(std::enable_if_t<!std::is_pointer_v<IteratorType>, std::true_type>::value, Storm::details::toStdString<Policy>(*val))
			{
				return Storm::details::toStdString<Policy>(*val);
			}

			template<class ExceptionType>
			static auto parseImpl(const ExceptionType &val, void*) -> decltype(Storm::details::toStdString<Policy>(val.what()))
			{
				return Storm::details::toStdString<Policy>(val.what());
			}

			template<class ErrorType>
			static auto parseImpl(const ErrorType &val, void*) -> decltype(Storm::details::toStdString<Policy>(val.ErrorMessage()))
			{
				return Storm::details::toStdString<Policy>(val.ErrorMessage());
			}

			template<class ChronoType>
			static auto parseImpl(const ChronoType &val, void*) -> decltype(val.count(), std::declval<ChronoType::rep>(), std::declval<ChronoType::period>(), std::string())
			{
				std::string result;

				result.reserve(32);
				
				std::chrono::microseconds remaining = std::chrono::duration_cast<std::chrono::microseconds>(val);

				const std::chrono::hours hourPart = std::chrono::duration_cast<std::chrono::hours>(remaining);
				const auto hourPartInt = hourPart.count();
				if (hourPartInt != 0)
				{
					result += std::to_string(hourPartInt);
					result += "h ";
					remaining -= std::chrono::duration_cast<std::chrono::microseconds>(hourPart);
				}

				const std::chrono::minutes minPart = std::chrono::duration_cast<std::chrono::minutes>(remaining);
				const auto minPartInt = minPart.count();
				if (minPartInt != 0)
				{
					result += std::to_string(minPartInt);
					result += "min ";
					remaining -= std::chrono::duration_cast<std::chrono::microseconds>(minPart);
				}

				const std::chrono::seconds secPart = std::chrono::duration_cast<std::chrono::seconds>(remaining);
				const auto secPartInt = secPart.count();
				if (secPartInt != 0)
				{
					result += std::to_string(secPartInt);
					result += "s ";
					remaining -= std::chrono::duration_cast<std::chrono::microseconds>(secPart);
				}

				const std::chrono::milliseconds millisecPart = std::chrono::duration_cast<std::chrono::milliseconds>(remaining);
				const auto millisecPartInt = millisecPart.count();
				if (millisecPartInt != 0)
				{
					result += std::to_string(millisecPartInt);
					result += "ms ";
					remaining -= std::chrono::duration_cast<std::chrono::microseconds>(millisecPart);
				}

				const auto microsecPartInt = remaining.count();
				if (microsecPartInt != 0)
				{
					result += std::to_string(microsecPartInt);
					result += "us";
				}

				if (result.back() == ' ')
				{
					result.pop_back();
				}

				return result;
			}

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0))
			{
				return parseImpl(std::forward<ValType>(val), 0);
			}
		};

		template<class Policy, class ValueType>
		struct ContainerParser
		{
		private:
			template<class Policy>
			static auto separator(int) -> decltype(Policy::separator())
			{
				return Policy::separator();
			}

			template<class Policy>
			constexpr static const char separator(void*)
			{
				return '\n';
			}

			template<class Policy>
			constexpr static auto printIndex(int) -> decltype(Policy::k_printIndex)
			{
				return Policy::k_printIndex;
			}

			template<class Policy>
			constexpr static bool printIndex(void*)
			{
				return false;
			}

			template<class Type>
			static auto extractSize(const Type &val, int) -> decltype(val.size())
			{
				return val.size();
			}

			template<class Type>
			static constexpr std::size_t extractSize(const Type &val, void*)
			{
				return 1;
			}

			template<class Policy>
			static constexpr auto defaultItemSize(int) -> decltype(static_cast<std::size_t>(Policy::k_expectedItemSize))
			{
				return static_cast<std::size_t>(Policy::k_expectedItemSize);
			}

			template<class Type>
			static constexpr std::size_t defaultItemSize(void*)
			{
				return 16;
			}

			// In case the Policy have its own heuristic formula
			template<class Policy>
			static auto computeHeuristicSize(const std::size_t extractedContainerSize, const std::size_t separatorSize, int) -> decltype(Policy::heuristicSize(extractedContainerSize, separatorSize))
			{
				return Policy::heuristicSize(extractedContainerSize, separatorSize);
			}

			template<class Policy>
			static constexpr std::size_t computeHeuristicSize(const std::size_t extractedContainerSize, const std::size_t separatorSize, void*)
			{
				return extractedContainerSize * (Storm::details::ContainerParser<Policy, ValueType>::defaultItemSize<Policy>(0) + separatorSize);
			}

			// Special Policy type that are made for container iteration.
			// Reduce the allocation/deallocation by appending to the final string instead of returning a new string that is pushed to the final string.
			// But those Policy should define a method named parseAppending
			template<class Policy, class ItemType>
			static auto appendParsedItem(std::string &inOutFinalStr, ItemType &&item, int) -> decltype(Policy::template parseAppending<Policy>(inOutFinalStr, std::forward<ItemType>(item)))
			{
				Policy::template parseAppending<Policy>(inOutFinalStr, std::forward<ItemType>(item));
			}

			template<class Policy, class ItemType>
			static void appendParsedItem(std::string &inOutFinalStr, ItemType &&item, void*)
			{
				inOutFinalStr += Storm::details::toStdString<Policy>(std::forward<ItemType>(item));
			}

		private:
			template<class StdContainerType>
			static auto parseImpl(StdContainerType &&array, int) -> decltype(std::begin(array), std::end(array), std::string{})
			{
				std::string result;
				const auto currentSeparator = Storm::details::ContainerParser<Policy, ValueType>::separator<Policy>(0);
				const std::size_t currentSeparatorSize = Storm::details::ContainerParser<Policy, ValueType>::extractSize(currentSeparator, 0);

				constexpr bool shouldPrintIndex = Storm::details::ContainerParser<Policy, ValueType>::printIndex<Policy>(0);

				// Heuristic
				const std::size_t containerExtractedSize = Storm::details::ContainerParser<Policy, ValueType>::extractSize(array, 0);
				result.reserve(Storm::details::ContainerParser<Policy, ValueType>::computeHeuristicSize<Policy>(containerExtractedSize, currentSeparatorSize, 0));

				for (auto &&item : array)
				{
					if constexpr (shouldPrintIndex)
					{
						result += "Iter=";
						result += std::to_string(&item - &*std::begin(array));
						result += "; ";
					}

					if constexpr (std::is_rvalue_reference_v<decltype(item)>)
					{
						Storm::details::ContainerParser<Policy, ValueType>::appendParsedItem<Policy>(result, std::move(item), 0);
						result += currentSeparator;
					}
					else
					{
						Storm::details::ContainerParser<Policy, ValueType>::appendParsedItem<Policy>(result, item, 0);
						result += currentSeparator;
					}
				}

				const std::size_t resultSize = result.size();
				if (resultSize > 0)
				{
					result.erase(resultSize - currentSeparatorSize);
				}

				return result;
			}

			// TODO : tuples

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0))
			{
				return parseImpl(std::forward<ValType>(val), 0);
			}
		};

		template<class Policy, class ValueType>
		struct ForwardParser
		{
		public:
			template<std::size_t valSize>
			static std::string parseImpl(const char (&val)[valSize], int)
			{
				return std::string{ val };
			}

			static std::string parseImpl(nullptr_t, int)
			{
				if constexpr (std::is_convertible_v<Policy, Storm::DebugPolicy>)
				{
					return "null";
				}
				else if constexpr (std::is_convertible_v<Policy, Storm::NumericPolicy>)
				{
					return "0x00000000";
				}
				else
				{
					return "";
				}
			}

			static std::string parseImpl(const char* val, int)
			{
				return std::string{ val };
			}

			template<class StringType>
			static auto parseImpl(StringType &&val, void*) 
				-> decltype(std::enable_if_t<std::is_same_v<std::remove_cv_t<std::remove_reference_t<StringType>>, std::string>, std::true_type>::value, std::forward<StringType>(val))
			{
				return std::forward<StringType>(val);
			}

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0))
			{
				return parseImpl(std::forward<ValType>(val), 0);
			}
		};

		template<class Policy, class ValueType>
		struct ConvertableParser
		{
		private:
			template<class PolicyType> using PriorityTagOnDebug = std::conditional_t<std::is_convertible_v<PolicyType, Storm::DebugPolicy>, int, void*>;
			template<class PolicyType> using PriorityTagOnNonDebug = std::conditional_t<!std::is_convertible_v<PolicyType, Storm::DebugPolicy>, int, void*>;

		private:
			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).toStdString())
			{
				return std::forward<ValType>(val).toStdString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).toString())
			{
				return std::forward<ValType>(val).toString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).ToStdString())
			{
				return std::forward<ValType>(val).ToStdString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).ToString())
			{
				return std::forward<ValType>(val).ToString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).string())
			{
				return std::forward<ValType>(val).string();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).str())
			{
				return std::forward<ValType>(val).str();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).String())
			{
				return std::forward<ValType>(val).String();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnNonDebug<Policy>) -> decltype(std::forward<ValType>(val).Str())
			{
				return std::forward<ValType>(val).Str();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnDebug<Policy>) -> decltype(std::forward<ValType>(val).toDebugString())
			{
				return std::forward<ValType>(val).toDebugString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnDebug<Policy>) -> decltype(std::forward<ValType>(val).debugString())
			{
				return std::forward<ValType>(val).debugString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnDebug<Policy>) -> decltype(std::forward<ValType>(val).ToDebugString())
			{
				return std::forward<ValType>(val).ToDebugString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, PriorityTagOnDebug<Policy>) -> decltype(std::forward<ValType>(val).DebugString())
			{
				return std::forward<ValType>(val).DebugString();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, ...) -> decltype(std::forward<ValType>(val).template as<std::string>())
			{
				return std::forward<ValType>(val).template as<std::string>();
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, ...) -> decltype(std::forward<ValType>(val).template As<std::string>())
			{
				return std::forward<ValType>(val).template As<std::string>();
			}

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0))
			{
				return parseImpl(std::forward<ValType>(val), 0);
			}
		};

		template<class Policy, class ValueType>
		struct CastableParser
		{
		private:
			template<class ValType>
			static auto parseImpl(ValType &&val, int) -> decltype(static_cast<std::string>(std::forward<ValType>(val)))
			{
				return static_cast<std::string>(std::forward<ValType>(val));
			}

			template<class ValType>
			static auto parseImpl(ValType &&val, void*) -> decltype(std::string(std::forward<ValType>(val)))
			{
				return std::string(std::forward<ValType>(val));
			}

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0))
			{
				return parseImpl(std::forward<ValType>(val), 0);
			}
		};

		template<class Policy, class ValueType>
		struct MiscParser
		{
		private:
			template<class ValType>
			static auto parseImpl(ValType &&val, int) -> decltype((std::stringstream{} << std::forward<ValType>(val)).str())
			{
				return (std::stringstream{} << std::forward<ValType>(val)).str();
			}

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val), 0))
			{
				return parseImpl(std::forward<ValType>(val), 0);
			}
		};

		// Custom policy parser for type that will be declared somewhere else in the application (maybe inside a child module), but where we want a default policy for them...
		template<class Policy, class ValueType>
		struct CustomLaterImplementedTypeParser
		{
		private:
			// Default policy for Storm::Vector3 that will be defined inside Storm-ModelBase library
			template<class ValType>
			static auto parseImpl(ValType &&val)
				-> decltype(std::enable_if_t<std::is_same_v<std::remove_cv_t<std::remove_reference_t<ValType>>, Storm::Vector3>, std::true_type>::value, std::string{})
			{
				std::string result;
				
				const std::string xStr = Storm::details::toStdString<Policy>(val.x());
				const std::string yStr = Storm::details::toStdString<Policy>(val.y());
				const std::string zStr = Storm::details::toStdString<Policy>(val.z());

				result.reserve(static_cast<std::size_t>(8) + xStr.size() + yStr.size() + zStr.size());

				result += "{ ";
				result += xStr;
				result += ", ";
				result += yStr;
				result += ", ";
				result += zStr;
				result += " }";

				return result;
			}

			// Default policy for Storm::Vector4 that will be defined inside Storm-ModelBase library
			template<class ValType>
			static auto parseImpl(ValType &&val)
				-> decltype(std::enable_if_t<std::is_same_v<std::remove_cv_t<std::remove_reference_t<ValType>>, Storm::Vector4>, std::true_type>::value, std::string{})
			{
				std::string result;

				const std::string xStr = Storm::details::toStdString<Policy>(val.x());
				const std::string yStr = Storm::details::toStdString<Policy>(val.y());
				const std::string zStr = Storm::details::toStdString<Policy>(val.z());
				const std::string wStr = Storm::details::toStdString<Policy>(val.w());

				result.reserve(static_cast<std::size_t>(10) + xStr.size() + yStr.size() + zStr.size() + wStr.size());

				result += "{ ";
				result += xStr;
				result += ", ";
				result += yStr;
				result += ", ";
				result += zStr;
				result += ", ";
				result += wStr;
				result += " }";

				return result;
			}

		public:
			template<class ValType>
			static auto parse(ValType &&val) -> decltype(parseImpl(std::forward<ValType>(val)))
			{
				return parseImpl(std::forward<ValType>(val));
			}
		};

		template<class Policy, class ValueType>
		struct EnumerationParser
		{
		private:
			template<class ValType>
			static auto parseImpl(const ValType val, int) -> decltype(Storm::details::toStdString<Policy>(static_cast<int64_t>(val)))
			{
				return Storm::details::toStdString<Policy>(static_cast<int64_t>(val));
			}

		public:
			template<class ValType>
			static auto parse(const ValType val) -> decltype(parseImpl(val, 0))
			{
				return parseImpl(val, 0);
			}
		};

		template<class Policy, class ValueType>
		struct NonParser
		{
			template<class ValType>
			static std::string parse(ValType &&)
			{
				static_assert(false, "This is not convertible! Duh...");
			}
		};

		template<class Policy = Storm::DefaultPolicy, class ValueType>
		decltype(auto) toStdString(ValueType &&val)
		{
#define STORM_CHECK_AND_RETURN_IF_SHOULD_USE(ParserName) \
		std::conditional_t<Storm::details::ParserValidator<Storm::details::ParserName<Policy, ValueType>, ValueType>::shouldUse, Storm::details::ParserName<Policy, ValueType>

			using BestFitParser =
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(CustomPolicyParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(CustomLaterImplementedTypeParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(ForwardParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(ConvertableParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(StandardParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(ContainerParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(NativeParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(CastableParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(MiscParser),
				STORM_CHECK_AND_RETURN_IF_SHOULD_USE(EnumerationParser),
				Storm::details::NonParser<Policy, ValueType>
				>>>>>>>>>>;

#undef STORM_CHECK_AND_RETURN_IF_SHOULD_USE

			return BestFitParser::parse(std::forward<ValueType>(val));
		}
	}


	template<class Policy = Storm::DefaultPolicy, class ValueType>
	std::string toStdString(ValueType &&val)
	{
		return Storm::details::toStdString<Policy>(std::forward<ValueType>(val));
	}
}
