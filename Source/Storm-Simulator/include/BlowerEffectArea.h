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

	protected:
		BlowerSphereArea(const Storm::BlowerData &blowerDataConfig, int);

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

	class BlowerRepulsionSphereArea : private Storm::BlowerSphereArea
	{
	public:
		BlowerRepulsionSphereArea(const Storm::BlowerData &blowerDataConfig);

	public:
		using Storm::BlowerSphereArea::isInside;

	protected:
		static constexpr bool hasDistanceEffect() { return true; }

	public:
		void applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const;
	};

	class BlowerExplosionSphereArea : private Storm::BlowerSphereArea
	{
	public:
		BlowerExplosionSphereArea(const Storm::BlowerData &blowerDataConfig);

	public:
		using Storm::BlowerSphereArea::isInside;

	protected:
		static constexpr bool hasDistanceEffect() { return true; }

	public:
		void applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const;

	protected:
		float _radius;
	};
}
