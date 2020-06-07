#pragma once

#include "CallbackIdType.h"
#include "ValueGuard.h"


namespace Storm
{
	template<class ReturnType>
	struct MultiCallbackCallResult
	{
	public:
		using StoredResultType = std::remove_reference_t<ReturnType>;

	public:
		template<class StoredResultType>
		struct MultiCallbackCallResultElement
		{
			CallbackIdType _callbackId;
			StoredResultType _returnValue;
			std::string _error;
		};

		template<>
		struct MultiCallbackCallResultElement<void>
		{
			CallbackIdType _callbackId;
			std::string _error;
		};

	public:
		std::vector<MultiCallbackCallResultElement<ReturnType>> _bunkedResults;
	};

	template<class Fn>
	class MultiCallback
	{
	private:
		template<class Type> struct FuncReturnTypeDeducer {};
		template<class RetType, class ... Args> struct FuncReturnTypeDeducer<std::function<RetType(Args...)>> 
		{
			using ReturnType = RetType;
		};

	public:
		using ReturnType = FuncReturnTypeDeducer<Fn>::template ReturnType;
		using FnType = Fn;

	public:
		MultiCallback() : 
			_isExecuting{ false },
			_generator{ 0 }
		{
			enum { k_initialCallbackCountNoRealloc = 8 };

			_callbacks.reserve(k_initialCallbackCountNoRealloc);
			_ids.reserve(k_initialCallbackCountNoRealloc);
			_toBeRemoved.reserve(k_initialCallbackCountNoRealloc);
		}

	private:
		template<class Callback, class ResultStorage, class ... Args2>
		auto call(Callback &callback, ResultStorage &result, int, const Args2 &... args) -> decltype(result._returnValue = callback(result._callbackId, args...), void())
		{
			result._returnValue = callback(callbackId, args...);
		}

		template<class Callback, class ResultStorage, class ... Args2>
		auto call(Callback &callback, ResultStorage &result, int, const Args2 &... args) -> decltype(result._returnValue = callback(args...), void())
		{
			result._returnValue = callback(args...);
		}

		template<class Callback, class ResultStorage, class ... Args2>
		auto call(Callback &callback, ResultStorage &result, void*, const Args2 &... args) -> decltype(callback(result._callbackId, args...), void())
		{
			callback(callbackId, args...);
		}

		template<class Callback, class ResultStorage, class ... Args2>
		auto call(Callback &callback, ResultStorage &, void*, const Args2 &... args) -> decltype(callback(args...), void())
		{
			callback(args...);
		}

	public:
		template<class Func>
		CallbackIdType add(Func &&newFn)
		{
			CallbackIdType newId = ++_generator;
			_callbacks.emplace_back(std::forward<Func>(newFn));
			_ids.emplace_back(newId);

			return newId;
		}

		void remove(CallbackIdType callbackId)
		{
			if (_isExecuting)
			{
				// Will be removed when we would end execution.
				_toBeRemoved.emplace_back(callbackId);
				return;
			}

			const std::size_t callbackCount = _callbacks.size();
			assert(callbackCount == _ids.size() && "Mismatch between registered ids count and actual callback count isn't allowed!");

			for (std::size_t iter = 0; iter < callbackCount; ++iter)
			{
				CallbackIdType &currentId = _ids[iter];
				if (currentId == callbackId)
				{
					if (iter != (callbackCount - 1))
					{
						std::swap(_callbacks[iter], _callbacks.back());
						std::swap(currentId, _ids.back());
					}

					_callbacks.pop_back();
					_ids.pop_back();
					return;
				}
			}
		}

		template<class ... Args2, class CallRetType = MultiCallbackCallResult<ReturnType>>
		CallRetType operator()(const Args2 &... args)
		{
			_isExecuting = false;
			CallRetType results;
			
			{
				Storm::ValueGuard<bool> executingGuard(_isExecuting);
				_isExecuting = true;

				const std::size_t callbackCount = _callbacks.size();
				assert(callbackCount == _ids.size() && "Mismatch between registered ids count and actual callback count isn't allowed!");

				results._bunkedResults.resize(callbackCount);

				for (std::size_t iter = 0; iter < callbackCount; ++iter)
				{
					CallRetType::MultiCallbackCallResultElement<ReturnType> &currentReturnElement = results._bunkedResults[iter];
					currentReturnElement._callbackId = _ids[iter];

					try
					{
						this->call(_callbacks[iter], currentReturnElement, 0, args...);
					}
					catch (const std::exception &ex)
					{
						currentReturnElement._error = ex.what();
					}
					catch (...)
					{
						currentReturnElement._error = "Unknown exception happened when calling callback " + std::to_string(currentReturnElement._callbackId);
					}
				}
			}

			for (const Storm::CallbackIdType callbackIdToRemove : _toBeRemoved)
			{
				this->remove(callbackIdToRemove);
			}
			_toBeRemoved.clear();

			return results;
		}

		void clear()
		{
			_callbacks.clear();
			_ids.clear();
			_toBeRemoved.clear();
		}

		bool empty() const
		{
			return _callbacks.empty();
		}

		void transfertTo(Storm::MultiCallback<Fn> &other)
		{
			std::swap(_callbacks, other._callbacks);
			std::swap(_ids, other._ids);
			std::swap(_toBeRemoved, other._toBeRemoved);

			if (_generator > other._generator)
			{
				other._generator = _generator;
			}
			else
			{
				_generator = other._generator;
			}
		}

	private:
		bool _isExecuting;
		CallbackIdType _generator;
		std::vector<FnType> _callbacks;
		std::vector<CallbackIdType> _ids;
		std::vector<CallbackIdType> _toBeRemoved;
	};

	template<class Fn, class ... Args>
	auto prettyCallMultiCallback_Exec(const std::string_view &callbackName, const std::string_view &fromFunc, MultiCallback<Fn> &multiCallback, const Args &... args)
	{
		auto results = multiCallback(args...);

		for (const auto &resultElement : results._bunkedResults)
		{
			if (!resultElement._error.empty())
			{
				LOG_ERROR <<
					"Callback with id '" << resultElement._callbackId << 
					"' from '" << callbackName << 
					"' and called from '" << fromFunc << 
					"' was aborted because it threw an error : \"" << resultElement._error << '"';
			}
		}

		return results;
	}

#define prettyCallMultiCallback(multiCallback, ...) prettyCallMultiCallback_Exec(#multiCallback, __FUNCTION__, multiCallback, __VA_ARGS__)
}
