#pragma once


namespace Storm
{
	// Workaround for std::function and lambda need to copy instead of move somewhere along the object forwarding 
	// (an implementation bug with the std::function "unperfect" perfect forwarding if used with lambdas)
	template<class Object>
	struct FuncObjectMovePass
	{
	public:
		FuncObjectMovePass(Object &&object) :
			_object{ std::move(object) }
		{}

		FuncObjectMovePass(FuncObjectMovePass &&other) = default;
		FuncObjectMovePass(const FuncObjectMovePass &other) :
			FuncObjectMovePass{ std::move(const_cast<FuncObjectMovePass &>(other)) }
		{}

	public:
		Object _object;
	};
}
