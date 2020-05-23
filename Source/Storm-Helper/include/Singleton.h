#pragma once

#include <boost\noncopyable.hpp>


namespace Storm
{
	template<class Type> struct SingletonAllocatorHelper;

	struct NoDefaultImplementation {};

	/*
	All singletons should inherit from this base class (better be in private) and then, use XMARTY_DECLARE_SINGLETON inside the ChildSingletonType class body.
	- This is a CRTP pattern so Child should the Singleton type that inherits from Singleton.
	- DefaultImplementationTraits is what specify if what is the default implementation of initialize_Implementation and cleanUp_Implementation are (or if there is any). By default, we don't define them. But you can define an empty implementation by using DefineDefaultInitAndCleanupImplementation instead.
	*/
	template<class Child, class DefaultImplementationTraits = NoDefaultImplementation>
	class Singleton :
		private boost::noncopyable,
		protected DefaultImplementationTraits
	{
	private:
		friend Child;
		friend SingletonAllocatorHelper<Child>;

	protected:
		// Defining those aliases allow the Child to call Singleton explicitly (to know about its grand fathers) (used in XMARTY_DECLARE_SINGLETON).
		using DefaultImplementationTraitsType = DefaultImplementationTraits;

	private:
		constexpr Singleton() : _initialized{ false } {}

	private:
		template<class ... Args>
		auto callInitialize(int, Args &&... args)
			-> decltype(std::enable_if_t<std::is_same_v<decltype(s_instance->initialize_Implementation(std::forward<Args>(args)...)), bool>, std::true_type>::value, true)
		{
			return static_cast<Child*>(this)->initialize_Implementation(std::forward<Args>(args)...);
		}

		template<class ... Args>
		bool callInitialize(void*, Args &&... args)
		{
			static_cast<Child*>(this)->initialize_Implementation(std::forward<Args>(args)...);
			return true;
		}

		template<class ... Args>
		auto callCleanUp(int, Args &&... args)
			-> decltype(std::enable_if_t<std::is_same_v<decltype(s_instance->cleanUp_Implementation(std::forward<Args>(args)...)), bool>, std::true_type>::value, true)
		{
			return static_cast<Child*>(this)->cleanUp_Implementation(std::forward<Args>(args)...);
		}

		template<class ... Args>
		bool callCleanUp(void*, Args &&... args)
		{
			static_cast<Child*>(this)->cleanUp_Implementation(std::forward<Args>(args)...);
			return true;
		}

	public:
		template<class ... Args>
		void initialize(Args &&... args)
		{
			std::lock_guard<std::mutex> lock{ s_initMutex };
			if (!_initialized)
			{
				_initialized = this->callInitialize(0, std::forward<Args>(args)...);
			}
		}

		template<class ... Args>
		void cleanUp(Args &&... args)
		{
			std::lock_guard<std::mutex> lock{ s_initMutex };
			if (_initialized)
			{
				_initialized = !this->callCleanUp(0, std::forward<Args>(args)...);
			}
		}

		bool isInitialized() const
		{
			return _initialized;
		}

		static bool isAlive()
		{
			return s_instance != nullptr;
		}

		static Child& instance()
		{
			return *s_instance;
		}

	private:
		static inline Child* s_instance = nullptr;
		static inline std::mutex s_initMutex;
		bool _initialized;
	};
}

#define STORM_DECLARE_SINGLETON(SingletonClassName)                                             \
public:                                                                                         \
using Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>::instance;          \
using Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>::isAlive;           \
using Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>::isInitialized;     \
using Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>::initialize;        \
using Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>::cleanUp;           \
public:                                                                                         \
using SingletonParent = Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>;  \
private:                                                                                        \
friend Storm::Singleton<SingletonClassName, DefaultImplementationTraitsType>;                   \
friend Storm::SingletonAllocatorHelper<SingletonClassName>;                                     \
SingletonClassName();                                                                           \
~SingletonClassName()
