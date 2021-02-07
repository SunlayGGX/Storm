#pragma once


namespace Storm
{
	struct SceneBlowerConfig;

	class BlowerCubeArea
	{
	public:
		BlowerCubeArea(const Storm::SceneBlowerConfig &blowerConfig);

	protected:
		BlowerCubeArea(const Storm::SceneBlowerConfig &blowerConfig, int);

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

	// Cube Area that applies a stronger effect if the particle is the the plane passing by the center of the cube.
	class BlowerGradualDirectionalCubeArea : private Storm::BlowerCubeArea
	{
	public:
		BlowerGradualDirectionalCubeArea(const Storm::SceneBlowerConfig &blowerConfig);

	public:
		using Storm::BlowerCubeArea::isInside;

	public:
		static constexpr bool hasDistanceEffect() { return true; }

	public:
		void applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const;

	protected:
		Storm::Vector3 _planeDirectionVect;
		float _maxDistanceToCenterPlane;
	};

	class BlowerSphereArea
	{
	public:
		BlowerSphereArea(const Storm::SceneBlowerConfig &blowerConfig);

	protected:
		BlowerSphereArea(const Storm::SceneBlowerConfig &blowerConfig, int);

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
		BlowerRepulsionSphereArea(const Storm::SceneBlowerConfig &blowerConfig);

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
		BlowerExplosionSphereArea(const Storm::SceneBlowerConfig &blowerConfig);

	public:
		using Storm::BlowerSphereArea::isInside;

	protected:
		static constexpr bool hasDistanceEffect() { return true; }

	public:
		void applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const;

	protected:
		float _radius;
	};

	// For now, a cylinder that is vertical. (YAGNI)
	class BlowerCylinderArea
	{
	public:
		BlowerCylinderArea(const Storm::SceneBlowerConfig &blowerConfig);

	protected:
		BlowerCylinderArea(const Storm::SceneBlowerConfig &blowerConfig, int);

	public:
		__forceinline bool isInside(const Storm::Vector3 &relativePosDiff) const
		{
			if (std::fabs(relativePosDiff.y()) < _midHeight)
			{
				const float twoDRelativeLengthSquared = relativePosDiff.x() * relativePosDiff.x() + relativePosDiff.z() * relativePosDiff.z();
				return twoDRelativeLengthSquared < _radiusSquared;
			}

			return false;
		}

	protected:
		float _midHeight;
		float _radiusSquared;
	};

	// For now, a cylinder that is vertical (YAGNI). The plane is placed at mid height.
	class BlowerCylinderGradualMidPlanarArea : private Storm::BlowerCylinderArea
	{
	public:
		BlowerCylinderGradualMidPlanarArea(const Storm::SceneBlowerConfig &blowerConfig);

	public:
		using Storm::BlowerCylinderArea::isInside;

	protected:
		static constexpr bool hasDistanceEffect() { return true; }

	public:
		void applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const;
	};

	class BlowerConeArea
	{
	public:
		BlowerConeArea(const Storm::SceneBlowerConfig &blowerConfig);

	public:
		__forceinline bool isInside(const Storm::Vector3 &relativePosDiff) const
		{
			const float yAbs = std::fabs(relativePosDiff.y());
			if (yAbs < _midHeight)
			{
				const float alpha = yAbs / _midHeight;

				// This is a LERP
				const float currentRadiusSquared = _downRadiusSquared + _diffRadiusSquared * alpha;

				const float twoDRelativeLengthSquared = relativePosDiff.x() * relativePosDiff.x() + relativePosDiff.z() * relativePosDiff.z();
				return twoDRelativeLengthSquared < currentRadiusSquared;
			}

			return false;
		}

	protected:
		float _midHeight;
		float _diffRadiusSquared;
		float _downRadiusSquared;
	};
}
