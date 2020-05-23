#pragma once

namespace Storm
{
	template<class Type, class ... Args>
	void ZeroMemories(Type &val, Args &... others)
	{
		ZeroMemories(val);
		ZeroMemories(others...);
	}

	template<class Type>
	void ZeroMemories(Type &val)
	{
		ZeroMemory(&val, sizeof(Type));
	}
}