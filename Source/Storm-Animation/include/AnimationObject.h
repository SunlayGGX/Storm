#pragma once


namespace Storm
{
	struct SceneRigidBodyConfig;
	struct AnimationKeyframe;

	class AnimationObject
	{
	public:
		AnimationObject(const Storm::SceneRigidBodyConfig &rbSceneConfig);
		~AnimationObject();

	public:
		bool getCurrentFrameOrShouldBeDestroyed(float currentTime, Storm::Vector3 &outPos, Storm::Vector3 &outRot) const;

	private:
		std::vector<Storm::AnimationKeyframe> _keyframes;
		bool _isLooping;
		float _endAnimTime;

		// This is a cache of the last keyframe position to not search all the time.
		mutable std::size_t _lastKeyIndex;
	};
}
