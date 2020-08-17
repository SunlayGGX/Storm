#pragma once


namespace Storm
{
	// 26 + 1 because the last one is intended to always be nullptr for an iteration check. The maximum valid neighborhood bunk count is 26 (we have 26 3D cells around the main cell).
	enum : std::size_t { k_neighborLinkedBunkCount = 27 };
}
