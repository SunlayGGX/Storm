#include "ConstraintVisualizationItem.h"


Storm::ConstraintVisualizationItem::ConstraintVisualizationItem(const std::size_t id, const Storm::Vector3 &actor0Pos, const Storm::Vector3 &actor1Pos) :
	_id{ id },
	_actor0Position{ actor0Pos },
	_actor1Position{ actor1Pos }
{}
