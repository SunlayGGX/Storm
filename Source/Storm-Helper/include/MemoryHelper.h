#pragma once

#include "StringHijack.h"


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
		::memset(&val, 0, sizeof(Type));
	}


	namespace details
	{
		template<class FileStream, class StringType>
		__forceinline auto binaryReadImpl(FileStream &file, StringType &outValue, int) -> decltype(outValue.size(), outValue.data(), void())
		{
			uint64_t strSize;
			details::binaryReadImpl(file, strSize, 0);
			Storm::resize_hijack(outValue, strSize);

			file.read(reinterpret_cast<char*>(outValue.data()), strSize * sizeof(*outValue.data()));
		}

		template<class FileStream, class Type>
		__forceinline void binaryReadImpl(FileStream &file, Type &outValue, void*)
		{
			file.read(reinterpret_cast<char*>(&outValue), sizeof(Type));
		}

		template<class FileStream, class StringType>
		__forceinline auto binaryWriteImpl(FileStream &file, const StringType &inValue, int) -> decltype(inValue.size(), inValue.data(), void())
		{
			const uint64_t inValueSize = static_cast<uint64_t>(inValue.size());
			details::binaryWriteImpl(file, inValueSize, 0);
			file.write(reinterpret_cast<const char*>(inValue.data()), inValueSize * sizeof(*inValue.data()));
		}

		template<class FileStream, class Type>
		__forceinline void binaryWriteImpl(FileStream &file, const Type &inValue, void*)
		{
			file.write(reinterpret_cast<const char*>(&inValue), sizeof(Type));
		}
	}

	// Note that for binaryRead to work as expected (a lot for string), you should always use a binaryRead on data that was previously written using a binaryWrite
	template<class FileStream, class Type>
	void binaryRead(FileStream &file, Type &outValue)
	{
		Storm::details::binaryReadImpl(file, outValue, 0);
	}

	template<class FileStream, class Type>
	void binaryWrite(FileStream &file, const Type &inValue)
	{
		Storm::details::binaryWriteImpl(file, inValue, 0);
	}
}
