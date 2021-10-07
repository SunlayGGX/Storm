#pragma once


namespace Storm
{
	struct MemoryInfos
	{
	public:
		using BytesCount = std::size_t;

	public:
		BytesCount _usedMemory;
		BytesCount _availableMemory;
		BytesCount _totalMemory;
	};
}
