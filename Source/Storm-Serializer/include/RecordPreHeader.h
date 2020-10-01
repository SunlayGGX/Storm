#pragma once

#include "Version.h"


namespace Storm
{
	// Do not change this, otherwise you'll break the retrocompatibility.
	// This is the minimal header part that tells us how to read the Header or what comes next.
	struct RecordPreHeader
	{
	public:
		uint32_t _magicWordChecksum; // Not a real checksum, just a magic word to tell us the record file was completed.
		Storm::Version _recordVersion;
		uint64_t _recordFileSize = 0;
	};
}
