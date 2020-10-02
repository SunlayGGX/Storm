#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	class RecordReader : public Storm::RecordHandlerBase
	{
	public:
		RecordReader(Storm::SerializeRecordHeader &&header);
	};
}
