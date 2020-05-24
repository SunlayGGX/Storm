#pragma once

#include <boost\core\noncopyable.hpp>


namespace Storm
{
	template<class ValueType>
	class ValueGuard :
		private boost::noncopyable
	{
	public:
		ValueGuard(ValueType &value) :
			_srcValue{ &value },
			_snapshot{ value }
		{}

		~ValueGuard()
		{
			if (_srcValue != nullptr)
			{
				this->apply();
			}
		}

		ValueGuard(ValueGuard &&other) :
			_srcValue{ other._srcValue },
			_snapshot{ std::move(other._snapshot) }
		{
			other.release();
		}

		ValueGuard& operator=(ValueGuard &&other)
		{
			_srcValue = other._srcValue;
			_snapshot = std::move(other._snapshot);
			other.release();

			return *this;
		}

		// Once you've released, this object becomes useless!
		void release()
		{
			_srcValue = nullptr;
		}

		void apply()
		{
			// No nullcheck on purpose... Don't do bad things because this class is straight forward, if it is null then this is because you called release before,
			// So like std::unique_ptr, we don't care about dev mistakes.
			*_srcValue = _snapshot;
		}

		void redoSnapshot()
		{
			// No nullcheck on purpose... Don't do bad things because this class is straight forward, if it is null then this is because you called release before,
			// so like std::unique_ptr, we don't care about dev mistakes.
			_snapshot = *_srcValue;
		}

	private:
		ValueType* _srcValue;
		ValueType _snapshot;
	};
}
