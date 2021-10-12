#include "GeometryConfig.h"

#include "GeometryType.h"


Storm::GeometryConfig::GeometryConfig() :
	_type{ Storm::GeometryType::None },
	_holeModality{ Storm::GeometryConfig::HoleModality::None },
	_sampleCountMDeserno{ 0 }  // Actually a stupid value
{

}
