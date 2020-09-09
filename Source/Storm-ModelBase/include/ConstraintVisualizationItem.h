#pragma once

namespace Storm
{
	struct ConstraintVisualizationItem
	{
	public:
		ConstraintVisualizationItem(const std::size_t id, const Storm::Vector3 &actor0Pos, const Storm::Vector3 &actor1Pos);

	public:
		const std::size_t _id;
		const Storm::Vector3 _actor0Position;
		const Storm::Vector3 _actor1Position;
	};
}
