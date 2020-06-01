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

	template<class FileStream, class Type>
	void binaryRead(FileStream &file, Type &outValue)
	{
		file.readsome(reinterpret_cast<char*>(&outValue), sizeof(Type));
	}

	template<class FileStream, class Type>
	void binaryWrite(FileStream &file, const Type &inValue)
	{
		file.write(reinterpret_cast<const char*>(&inValue), sizeof(Type));
	}
}
