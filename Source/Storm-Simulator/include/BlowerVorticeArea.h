#pragma once


namespace Storm
{
	struct SceneBlowerConfig;

	struct NoVortice { NoVortice(const Storm::SceneBlowerConfig &); };

	class DefaultVorticeArea
	{
	protected:
		constexpr static bool hasVorticeEffect() { return true; }

	protected:
		DefaultVorticeArea(const Storm::SceneBlowerConfig &);

	protected:
		Storm::Vector3 applyVortice(const Storm::Vector3 &force, const float forceNorm, const Storm::Vector3 &posDiff) const;

	private:
		float _coeff;
	};
}
