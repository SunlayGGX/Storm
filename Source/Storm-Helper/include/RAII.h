#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct RAIIMaker : private Storm::NonInstanciable
	{
	private:
		template<class Lambda> friend auto makeLazyRAIIObject(Lambda &&lambda);

	private:
		template<class Lambda>
		static auto makeLazyRAIIObject_Internal(Lambda &&lambda)
		{
			class ProtectedUPtr : protected std::unique_ptr<void, std::remove_cvref_t<Lambda>>
			{
			private:
				using Parent = std::unique_ptr<void, Lambda>;

			public:
				using Parent::Parent;
				using Parent::release;
				using Parent::reset;

				// Never have a get because this object is a lie. It references a local variable that will be gone.
				// It is ok since it is a pointer, the pointer can point to something that doesn't exist anymore (like when we point to nullptr)
				// But dereferencing it is undefined behavior.
				//
				// The reason I didn't use a nullptr is because the lambda won't be called (the nullcheck happens before calling the lambda).
				// That's why I used an object on the stack that have a valid address (even if the object in this address is invalid, the address itself is valid).
			};

			int _dummy;
			return ProtectedUPtr{ static_cast<void*>(&_dummy), std::forward<Lambda>(lambda) };
		}
	};

	template<class Lambda>
	static auto makeLazyRAIIObject(Lambda &&lambda)
	{
		// The lambda encapsulating the original lambda is to hide the pointer. Now nobody can play with it because the object is invalid by construction
		// and undefined behavior will result in accessing it.
		return Storm::RAIIMaker::makeLazyRAIIObject_Internal([func = std::forward<Lambda>(lambda)](void*)
		{
			func();
		});
	}
}
