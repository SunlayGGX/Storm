#pragma once

namespace Storm
{
	struct VectorHijacker
	{
		std::size_t _newSize;
	};

	using VectorHijackerMakeBelieve = const VectorHijacker &;
}
