#pragma once


namespace Storm
{
	struct BlowerData;

	class BlowerCubeArea
	{
	public:
		BlowerCubeArea(const Storm::BlowerData &blowerDataConfig);

	public:
		__forceinline bool isInside(const Storm::Vector3 &relativePosDiff) const
		{
			return
				std::fabs(relativePosDiff.x()) < _dimension.x() &&
				std::fabs(relativePosDiff.y()) < _dimension.y() &&
				std::fabs(relativePosDiff.z()) < _dimension.z();
		}

	protected:
		const Storm::Vector3 _dimension;
	};

	class BlowerSphereArea
	{
	public:
		BlowerSphereArea(const Storm::BlowerData &blowerDataConfig);

	public:
		__forceinline bool isInside(const Storm::Vector3 &relativePosDiff) const
		{
			float squared = relativePosDiff.x() * relativePosDiff.x();
			if (squared < _radiusSquared)
			{
				float val = squared;
				squared = relativePosDiff.y() * relativePosDiff.y();
				if (squared < _radiusSquared)
				{
					val += squared;
					return (val + (relativePosDiff.z() * relativePosDiff.z())) < _radiusSquared;
				}
			}

			return false;
		}

	protected:
		float _radiusSquared;
	};
}
