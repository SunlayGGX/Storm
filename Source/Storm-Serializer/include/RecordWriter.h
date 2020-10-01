#pragma once

#include "RecordHandlerBase.h"


namespace Storm
{
	class RecordWriter : public Storm::RecordHandlerBase
	{
	public:
		RecordWriter(Storm::SerializeRecordHeader &&header);
	};
}
