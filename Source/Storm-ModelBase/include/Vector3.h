#pragma once


namespace Storm
{
	// This is not intended to be a complete implementation of a Vector3 like other mathematic library does.
	// The reason of this object is to pass the Data around between modules that would use their own mathematic library
	// (i.e Physx would use its own transform data while DirectX would use its own matrixes and data structures)
	struct Vector3
	{
	public:
		constexpr Vector3() = default;

	public:
		float _x = 0.f;
		float _y = 0.f;
		float _z = 0.f;
	};
}
