#pragma once

#include "NonInstanciable.h"
#include "StaticAssertionsMacros.h"


// The reason I'm not using boost for some cases are :
// 1- too slow
// 2- Need to play with many functions to do what I want : i.e. I need to split a string but empty entries cannot be ignored, therefore I need to trim my string before splitting it : 2 separate operations.
// 3- Not useful because the predicate concatenation are awkward, and I need to reexecute the same operation again and again with different modality 
// (i.e I want to split my string by '\n', ' ', " = ", '\t', '\0' and "superman", I need to reexecute the split on splitted items of splitted items of splitted items for a lot of those parameters. It is too slow and complex for nothing.

namespace Storm
{
	struct StringAlgo : private Storm::NonInstanciable
	{
	public:
		template<class Type, std::size_t count>
		static constexpr std::size_t extractReplacingSize(Type(&)[count], int)
		{
			return count - 1;
		}

		template<class StrType>
		static constexpr auto extractReplacingSize(const StrType &val, int) -> decltype(val.size())
		{
			return val.size();
		}

		template<class CharType>
		static constexpr std::size_t extractReplacingSize(const CharType, void*)
		{
			return 1;
		}

		template<class Type, std::size_t count>
		static constexpr std::size_t extractSize(Type(&)[count], int)
		{
			// count cannot be equal to 0 because of the terminating \0 at the end of any literal string.
			return std::max(count - 1, static_cast<std::size_t>(1));
		}

		template<class StrType>
		static constexpr auto extractSize(const StrType &val, int) -> decltype(val.size())
		{
			return val.size();
		}

		template<class CharType>
		static constexpr std::size_t extractSize(const CharType, void*)
		{
			return 1;
		}

	public:
		template<class Type>
		static constexpr std::size_t extractSize(const Type &val)
		{
			return StringAlgo::extractSize(val, 0);
		}

	private:
		// Tags
		struct And {};
		struct Or {};

		struct SplitterAlgoTag {};
		struct ReplaceAlgoTag {};

		template<class Predicate> constexpr static auto extractAlgoTag(int) -> decltype(std::declval<Predicate::template AlgoritmUsedFor>());
		template<class Predicate> constexpr static void extractAlgoTag(void*);

#define STORM_CHECK_PREDICATE_TAG_IMPL(PredicateType, ExpectedTag) std::is_same_v<std::remove_cvref_t<decltype(Storm::StringAlgo::extractAlgoTag<PredicateType>(0))>, ExpectedTag>
#define STORM_PREDICATE_CHECK_TAG(PredicateType, ExpectedTag, FromMethod)																									\
		STORM_STATIC_ASSERT_TO_VS_OUTPUT(STORM_CHECK_PREDICATE_TAG_IMPL(PredicateType, ExpectedTag), "Wrong predicate used! We expect one coming from " #FromMethod)

	private:
		// Params
		template<class StringType>
		struct PredicateSearchInParam
		{
		public:
			const StringType::template value_type* _currentPos;
			std::size_t _remainingSize;
		};

		template<class StringType>
		struct PredicateSearchOutParam
		{
		public:
			std::size_t _lastPredicateSize;
			std::size_t _lastPredicateIndex;
		};


		template<class SearchTag, class AlgoTag, class StringType, class SeparatorType, class ... OtherSeparators>
		struct SearchPredicate
		{
		public:
			using AlgoritmUsedFor = AlgoTag;

		public:
			template<class SeparatorSemanticDependentType, class ... OtherSeparatorsSemanticDependent>
			constexpr SearchPredicate(SeparatorSemanticDependentType &&sep, OtherSeparatorsSemanticDependent &&... others) :
				_pred{ std::forward<SeparatorSemanticDependentType>(sep) },
				_otherPred{ std::forward<OtherSeparatorsSemanticDependent>(others)... }
			{}

		public:
			template<class StringParamType>
			bool operator()(const PredicateSearchInParam<StringParamType> &inParam, PredicateSearchOutParam<StringParamType> &inOutParam) const noexcept
			{
				if constexpr (std::is_same_v<SearchTag, Or>)
				{
					return _pred(inParam, inOutParam) || _otherPred(inParam, inOutParam);
				}
				else if constexpr (std::is_same_v<SearchTag, And>)
				{
					return _pred(inParam, inOutParam) && _otherPred(inParam, inOutParam);
				}
				else
				{
					STORM_COMPILE_ERROR("Unknown search tag!");
				}
			}

			constexpr std::size_t getMaxPredicateSize() const noexcept
			{
				return std::max(_pred.getMaxPredicateSize(), _otherPred.getMaxPredicateSize());
			}

			constexpr std::size_t getMinPredicateSize() const noexcept
			{
				return std::min(_pred.getMinPredicateSize(), _otherPred.getMinPredicateSize());
			}

		public:
			SearchPredicate<SearchTag, AlgoTag, StringType, SeparatorType> _pred;
			SearchPredicate<SearchTag, AlgoTag, StringType, OtherSeparators...> _otherPred;
		};

		template<class SearchTag, class AlgoTag, class StringType, class SeparatorType>
		struct SearchPredicate<SearchTag, AlgoTag, StringType, SeparatorType>
		{
		public:
			using AlgoritmUsedFor = AlgoTag;

		public:
			template<class SeparatorSemanticDependentType>
			constexpr SearchPredicate(SeparatorSemanticDependentType &&sep) :
				_toFind{ std::forward<SeparatorSemanticDependentType>(sep) }
			{
				_predicateSize = Storm::StringAlgo::extractSize(_toFind, 0);
			}

		public:
			template<class StringParamType>
			bool operator()(const PredicateSearchInParam<StringParamType> &inParam, PredicateSearchOutParam<StringParamType> &inOutParam) const noexcept
			{
				inOutParam._lastPredicateSize = _predicateSize;

				if (_predicateSize <= inParam._remainingSize)
				{
					if constexpr (std::is_same_v<std::remove_cvref_t<SeparatorType>, StringType::value_type>) // If it is a single character
					{
						return *inParam._currentPos == _toFind;
					}
					else
					{
						for (std::size_t iter = 0; iter < _predicateSize; ++iter)
						{
							if (inParam._currentPos[iter] != _toFind[iter])
							{
								return false;
							}
						}

						return true;
					}
				}

				return false;
			}

			constexpr std::size_t getMaxPredicateSize() const noexcept
			{
				return _predicateSize;
			}

			constexpr std::size_t getMinPredicateSize() const noexcept
			{
				return _predicateSize;
			}

		private:
			std::size_t _predicateSize;
			SeparatorType _toFind;
		};

	private:
		template<class InputStringType>
		struct AutoString
		{
		private:
			template<class InputStringType> static auto autoDetectStringToUse(InputStringType &&input) -> decltype(std::string{ std::forward<InputStringType>(input) });
			template<class InputStringType> static auto autoDetectStringToUse(InputStringType &&input) -> decltype(std::wstring{ std::forward<InputStringType>(input) });
			template<class InputStringType> static auto autoDetectStringToUse(InputStringType &&input) -> decltype(std::u8string{ std::forward<InputStringType>(input) });
			template<class InputStringType> static auto autoDetectStringToUse(InputStringType &&input) -> decltype(std::u32string{ std::forward<InputStringType>(input) });

		public:
			using Type = decltype(AutoString::autoDetectStringToUse<InputStringType>(std::declval<InputStringType>()));
		};

	public:
		template<class StringType = std::string, class ... AllSeparators>
		static auto makeSplitPredicate(AllSeparators &&... separators)
		{
			return SearchPredicate<Or, SplitterAlgoTag, StringType, AllSeparators...>{ std::forward<AllSeparators>(separators)... };
		}

		template<class StringType = std::string, class ... AllSeparators>
		static auto makeReplacePredicate(AllSeparators &&... separators)
		{
			return SearchPredicate<Or, ReplaceAlgoTag, StringType, AllSeparators...>{ std::forward<AllSeparators>(separators)... };
		}


		//********************************************************************************//
		//******************************** Split *****************************************//
		//********************************************************************************//

	public:
		template<class StringType, class Predicate>
		static std::vector<StringType>& split(std::vector<StringType> &inOutResult, const StringType &input, const Predicate &pred)
		{
			STORM_PREDICATE_CHECK_TAG(Predicate, Storm::StringAlgo::SplitterAlgoTag, Storm::StringAlgo::makeSplitPredicate);

			PredicateSearchInParam<StringType> param{
				._currentPos = &*std::begin(input),
				._remainingSize = Storm::StringAlgo::extractSize(input, 0)
			};

			PredicateSearchOutParam<StringType> additionalSearchRes;

			// trim
			while (pred(param, additionalSearchRes))
			{
				param._currentPos += additionalSearchRes._lastPredicateSize;
				param._remainingSize -= additionalSearchRes._lastPredicateSize;
			}

			if (param._remainingSize)
			{
				decltype(param._currentPos) lastPos = param._currentPos;

				const std::size_t maxPredicateSize = pred.getMaxPredicateSize();
				const std::size_t minPredicateSize = pred.getMinPredicateSize();

				const std::size_t inOutResultSize = inOutResult.size();
				if (inOutResultSize < 8)
				{
					if (maxPredicateSize > 1)
					{
						inOutResult.reserve(std::max(static_cast<std::size_t>(8), inOutResultSize + param._remainingSize / (maxPredicateSize + 1)));
					}
					else
					{
						inOutResult.reserve(inOutResultSize + 8);
					}
				}

				do
				{
					if (pred(param, additionalSearchRes))
					{
						if (lastPos != param._currentPos)
						{
							inOutResult.emplace_back(lastPos, param._currentPos);
						}

						param._currentPos += additionalSearchRes._lastPredicateSize;
						param._remainingSize -= additionalSearchRes._lastPredicateSize;

						lastPos = param._currentPos;
					}
					else if (param._remainingSize > minPredicateSize)
					{
						++param._currentPos;
						--param._remainingSize;
					}
					else
					{
						param._currentPos += param._remainingSize;
						break;
					}
				} while (param._remainingSize);


				if (lastPos != param._currentPos)
				{
					inOutResult.emplace_back(lastPos, param._currentPos);
				}
			}

			return inOutResult;
		}

		// By default, split without the first template argument is std::string
		template<class Predicate>
		static std::vector<std::string>& split(std::vector<std::string> &inOutResult, const std::string &input, const Predicate &pred)
		{
			return Storm::StringAlgo::split<std::string>(inOutResult, input, pred);
		}


		//********************************************************************************//
		//******************************** Replace ***************************************//
		//********************************************************************************//

	private:
		template<class PtrType, class ReplacementType>
		static auto replaceImpl(PtrType* &ptr, const ReplacementType &replacement, const std::size_t, int) -> decltype(*ptr = replacement, void())
		{
			*ptr = replacement;
			++ptr;
		}

		template<class PtrType, class ReplacementType>
		static auto replaceImpl(PtrType* &ptr, const ReplacementType &replacement, const std::size_t replacementSize, void*) -> decltype(std::copy(std::begin(replacement), std::begin(replacement) + replacementSize, ptr), void())
		{
			std::copy(std::begin(replacement), std::begin(replacement) + replacementSize, ptr);
			ptr += replacementSize;
		}

	public:
		template<class StringType, class ReplacementType, class Predicate>
		static StringType& replaceAll(StringType &inOutStr, const ReplacementType &replacement, const Predicate &pred)
		{
			STORM_PREDICATE_CHECK_TAG(Predicate, Storm::StringAlgo::ReplaceAlgoTag, Storm::StringAlgo::makeReplacePredicate);

			PredicateSearchInParam<StringType> param{
				._currentPos = &*std::begin(inOutStr),
				._remainingSize = Storm::StringAlgo::extractSize(inOutStr, 0)
			};

			if (param._remainingSize > 0)
			{
				PredicateSearchOutParam<StringType> additionalSearchRes;

				const std::size_t maxPredicateSize = pred.getMaxPredicateSize();
				const std::size_t minPredicateSize = pred.getMinPredicateSize();

				const std::size_t replacementSize = Storm::StringAlgo::extractReplacingSize(replacement, 0);

				if (replacementSize <= minPredicateSize)
				{
					auto replacementPos = &*std::begin(inOutStr);

					do
					{
						if (pred(param, additionalSearchRes))
						{
							param._currentPos += additionalSearchRes._lastPredicateSize;
							param._remainingSize -= additionalSearchRes._lastPredicateSize;

							replaceImpl(replacementPos, replacement, replacementSize, 0);
						}
						else
						{
							*replacementPos = *param._currentPos;
							++replacementPos;
							++param._currentPos;
							--param._remainingSize;
						}
					} while (param._remainingSize);

					std::size_t toRemove = param._currentPos - replacementPos;
					while (toRemove)
					{
						inOutStr.pop_back();
						--toRemove;
					}
				}
				else
				{
					// The string is bigger after than it is before. We need a temporary string.
					StringType tmp;
					tmp.reserve(param._remainingSize + param._remainingSize / 2);

					do
					{
						if (pred(param, additionalSearchRes))
						{
							param._currentPos += additionalSearchRes._lastPredicateSize;
							param._remainingSize -= additionalSearchRes._lastPredicateSize;

							tmp += replacement;
						}
						else
						{
							tmp += *param._currentPos;
							++param._currentPos;
							--param._remainingSize;
						}
					} while (param._remainingSize);

					inOutStr = std::move(tmp);
				}
			}

			return inOutStr;
		}

		template<class ReplacementType, class InputStringType, class Predicate>
		static auto replaceAllCopy(InputStringType &&inOutStr, const ReplacementType &replacement, const Predicate &pred)
		{
			auto result = AutoString<InputStringType>::template Type{ std::forward<InputStringType>(inOutStr) };
			Storm::StringAlgo::replaceAll(result, replacement, pred);
			return result;
		}
	};
}

#undef STORM_CHECK_PREDICATE_TAG_IMPL
#undef STORM_PREDICATE_CHECK_TAG
