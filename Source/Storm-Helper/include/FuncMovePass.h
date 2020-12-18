#pragma once


namespace Storm
{
	// Workaround for std::function and lambda need to copy instead of move somewhere along the object forwarding 
	// (an implementation bug with the std::function "unperfect" perfect forwarding if used with lambdas)
	template<class Object>
	struct FuncMovePass
	{
	public:
		FuncMovePass(Object &&object) :
			_object{ std::move(object) }
		{}

		FuncMovePass(FuncMovePass &&other) = default;
		FuncMovePass(const FuncMovePass &other) :
			FuncMovePass{ std::move(const_cast<FuncMovePass &>(other)) }
		{}

	public:
		Object _object;
	};
}
