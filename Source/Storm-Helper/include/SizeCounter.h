#pragma once


namespace Storm
{
	template<class Type, class ... Others>
	struct SizeCounter
	{
	public:
		enum : std::size_t
		{
			value = static_cast<std::size_t>(SizeCounter<Type>::value) + static_cast<std::size_t>(SizeCounter<Others...>::value)
		};
	};

	template<class Type>
	struct SizeCounter<Type>
	{
	public:
		enum : std::size_t
		{
			value = sizeof(Type)
		};
	};
}
