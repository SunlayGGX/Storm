#pragma once

#include "NonInstanciable.h"


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
		};


		template<class StringType, class SeparatorType, class ... OtherSeparators>
		struct SearchPredicate
		{
		public:
			template<class SeparatorSemanticDependentType, class ... OtherSeparatorsSemanticDependent>
			constexpr SearchPredicate(SeparatorSemanticDependentType &&sep, OtherSeparatorsSemanticDependent &&... others) :
				_pred{ std::forward<SeparatorSemanticDependentType>(sep) },
				_otherPred{ std::forward<OtherSeparatorsSemanticDependent>(others)... }
			{}

		public:
			bool operator()(const PredicateSearchInParam<StringType> &inParam, PredicateSearchOutParam<StringType> &inOutParam) const noexcept
			{
				return _pred(inParam, inOutParam) || _otherPred(inParam, inOutParam);
			}

			constexpr std::size_t getMaxPredicateSize() const
			{
				return std::max(_pred.getMaxPredicateSize(), _otherPred.getMaxPredicateSize());
			}

			constexpr std::size_t getMinPredicateSize() const
			{
				return std::min(_pred.getMinPredicateSize(), _otherPred.getMinPredicateSize());
			}

		public:
			SearchPredicate<StringType, SeparatorType> _pred;
			SearchPredicate<StringType, OtherSeparators...> _otherPred;
		};

		template<class StringType, class SeparatorType>
		struct SearchPredicate<StringType, SeparatorType>
		{
		public:
			template<class SeparatorSemanticDependentType>
			constexpr SearchPredicate(SeparatorSemanticDependentType &&sep) :
				_toFind{ std::forward<SeparatorSemanticDependentType>(sep) }
			{
				_predicateSize = Storm::StringAlgo::extractSize(_toFind, 0);
			}

		public:
			bool operator()(const PredicateSearchInParam<StringType> &inParam, PredicateSearchOutParam<StringType> &inOutParam) const noexcept
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

			constexpr std::size_t getMaxPredicateSize() const
			{
				return _predicateSize;
			}

			constexpr std::size_t getMinPredicateSize() const
			{
				return _predicateSize;
			}

		public:
			std::size_t _predicateSize;
			SeparatorType _toFind;
		};

	public:
		template<class StringType = std::string, class ... AllSeparators>
		static auto makePredicate(AllSeparators &&... separators)
		{
			return SearchPredicate<StringType, AllSeparators...>{ std::forward<AllSeparators>(separators)... };
		}

	public:
		template<class StringType, class Predicate>
		static std::vector<StringType>& split(std::vector<StringType> &inOutResult, const StringType &input, const Predicate &pred)
		{
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
					else
					{
						++param._currentPos;
						--param._remainingSize;

						if (param._remainingSize < minPredicateSize)
						{
							break;
						}
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
	};
}

