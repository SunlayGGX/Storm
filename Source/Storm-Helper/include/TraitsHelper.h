#pragma once


namespace Storm
{
	template<class Type>
	std::add_rvalue_reference_t<Type> declref(); // Like declval, no definition should be made since its purpose is only to blind the compiler.
}
