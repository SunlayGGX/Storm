#pragma once

#include "EndPointIdentifier.h"
#include "Version.h"


namespace Storm
{
	struct OnConnectionStateChangedParam
	{
	public:
		bool _connected;
		Storm::EndPointIdentifier _applicationId;
		Storm::Version _applicationVersion;
	};
}
