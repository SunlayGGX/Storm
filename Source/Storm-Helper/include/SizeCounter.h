#pragma once


namespace Storm
{
	template<class Type, class ... Others>
	struct SizeCounter
	{
	public:
		enum
		{
			value = SizeCounter<Type>::value + SizeCounter<Others...>::value
		};
	};

	template<class Type>
	struct SizeCounter<Type>
	{
	public:
		enum
		{
			value = sizeof(Type)
		};
	};
}
