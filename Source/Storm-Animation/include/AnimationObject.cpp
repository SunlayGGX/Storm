#include "AnimationObject.h"
#include "AnimationKeyframe.h"

#include "SceneRigidBodyConfig.h"

#include "XmlReader.h"
#include "ThreadingSafety.h"

#include <boost\property_tree\ptree_fwd.hpp>
#include <boost\property_tree\xml_parser.hpp>


namespace
{
	void lerp(const Storm::Vector3 &vectBefore, const Storm::Vector3 &vectAfter, const float coeff, Storm::Vector3 &result)
	{
		result = vectBefore + (vectAfter - vectBefore) * coeff;
	}

	// Called keyframe a and b on purpose. a goes to b, no telling in what chronological direction it is
	// (if a time is less than b time, we goes to the future, otherwise we'll go to the past).
	void lerpKeyframe(const Storm::AnimationKeyframe &a, const Storm::AnimationKeyframe &b, const float time, Storm::Vector3 &outPos, Storm::Vector3 &outRot)
	{
		if (a._timeInSecond == b._timeInSecond)
		{
			outPos = a._position;
			outRot = a._rotation;
		}

		const float coeff = (time - a._timeInSecond) / (b._timeInSecond - a._timeInSecond);

		lerp(a._position, b._position, coeff, outPos);
		lerp(a._rotation, b._rotation, coeff, outRot);
	}
}


Storm::AnimationObject::AnimationObject(const Storm::SceneRigidBodyConfig &rbSceneConfig) :
	_isLooping{ false },
	_lastKeyIndex{ 0 }
{
	boost::property_tree::ptree rootXmlTree;
	boost::property_tree::read_xml(std::stringstream{} << rbSceneConfig._animationXmlContent, rootXmlTree);

	const boost::property_tree::ptree &animationXmlTree = rootXmlTree.get_child("Animation");
	_keyframes.reserve(animationXmlTree.size());

	for (const auto &animationKeys : animationXmlTree)
	{
		if (_isLooping)
		{
			Storm::throwException<Storm::Exception>("We mustn't find any keys after a loop key.");
		}

		if (animationKeys.first == "Keyframe")
		{
			const boost::property_tree::ptree &keyframeElementXmlTree = animationKeys.second;

			Storm::AnimationKeyframe &keyframe = _keyframes.emplace_back();
			const std::size_t lastIndex = _keyframes.size() - 1;

			if (lastIndex > 0)
			{
				const Storm::AnimationKeyframe &keyframeBefore = _keyframes[lastIndex - 1];
				keyframe._position = keyframeBefore._position;
				keyframe._rotation = keyframeBefore._rotation;
			}
			else
			{
				keyframe._position = rbSceneConfig._translation;
				keyframe._rotation = rbSceneConfig._rotation;
			}
			
			Storm::XmlReader::sureReadXmlAttribute(keyframeElementXmlTree, keyframe._timeInSecond, "time");

			if (keyframe._timeInSecond < 0.f)
			{
				Storm::throwException<Storm::Exception>("Keyframe time shouldn't be negative! (" + std::to_string(keyframe._timeInSecond) + ")");
			}

			if (lastIndex > 0 && _keyframes[lastIndex - 1]._timeInSecond >= keyframe._timeInSecond)
			{
				Storm::throwException<Storm::Exception>("Disorder detected in the keyframe declaration order! Please, declare them in chronological order.");
			}

			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._position.x(), "posX");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._position.y(), "posY");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._position.z(), "posZ");

			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.x(), "rotX");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.y(), "rotY");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.z(), "rotZ");
		}
		else if (animationKeys.first == "Loop")
		{
			_isLooping = true;
		}
	}

	if (_keyframes.empty())
	{
		Storm::throwException<Storm::Exception>("We must have at least one keyframe to animate!");
	}

	// The first keyframe should always be the one at t=0s
	if (_keyframes[0]._timeInSecond != 0.f)
	{
		Storm::AnimationKeyframe &keyframe = *_keyframes.emplace(std::begin(_keyframes));
		keyframe._timeInSecond = 0.f;
		keyframe._position = rbSceneConfig._translation;
		keyframe._rotation = rbSceneConfig._rotation;
	}

	_endAnimTime = _keyframes.back()._timeInSecond;
}

Storm::AnimationObject::~AnimationObject() = default;

bool Storm::AnimationObject::getCurrentFrameOrShouldBeDestroyed(float currentTime, Storm::Vector3 &outPos, Storm::Vector3 &outRot) const
{
	assert(Storm::isAnimationThread() && "This method should only be called in animation thread!");

	const std::size_t lastKeyframeIndex = _keyframes.size() - 1;

	if (_isLooping)
	{
		while (currentTime > _endAnimTime)
		{
			currentTime -= _endAnimTime;
		}
		while (currentTime < 0)
		{
			currentTime += _endAnimTime;
		}
	}

	const Storm::AnimationKeyframe* lastAnimKeyframe = &_keyframes[_lastKeyIndex];
	if (lastAnimKeyframe->_timeInSecond < currentTime) // We go forward in time.
	{
		if (_lastKeyIndex == lastKeyframeIndex)
		{
			if (_isLooping)
			{
				_lastKeyIndex = 0;
			}
			else
			{
				// We're freed from the animation object.
				return false;
			}
		}

		do 
		{
			const Storm::AnimationKeyframe &animKeyframeAfter = _keyframes[_lastKeyIndex + 1];
			if (animKeyframeAfter._timeInSecond < currentTime)
			{
				++_lastKeyIndex;

				if (_lastKeyIndex == lastKeyframeIndex)
				{
					if (_isLooping)
					{
						_lastKeyIndex = 0;
					}
					else
					{
						// We're freed from the animation object.
						return false;
					}
				}

				lastAnimKeyframe = &_keyframes[_lastKeyIndex];
			}
			else
			{
				if (animKeyframeAfter._timeInSecond == currentTime)
				{
					++_lastKeyIndex;
					outPos = animKeyframeAfter._position;
					outRot = animKeyframeAfter._rotation;
				}
				else
				{
					lerpKeyframe(*lastAnimKeyframe, animKeyframeAfter, currentTime, outPos, outRot);
				}

				break;
			}

		} while (true);
	}
	else if (lastAnimKeyframe->_timeInSecond == currentTime) // We are already at the current frame.
	{
		outPos = lastAnimKeyframe->_position;
		outRot = lastAnimKeyframe->_rotation;
	}
	else  // We go backward in time. 
	{
		do
		{
			if (_lastKeyIndex == 0)
			{
				if (_isLooping)
				{
					_lastKeyIndex = lastKeyframeIndex;
				}
				else
				{
					Storm::throwException<Storm::Exception>(
						"We're going backward in time and since we don't loop, we have requested a negative time to come here.\n"
						"This is not allowed!\n"
						"Time value was " + std::to_string(currentTime)
					);
				}
			}

			const Storm::AnimationKeyframe &animKeyframeBefore = _keyframes[_lastKeyIndex - 1];

			if (animKeyframeBefore._timeInSecond > currentTime)
			{
				--_lastKeyIndex;

				if (_lastKeyIndex == 0)
				{
					if (_isLooping)
					{
						_lastKeyIndex = lastKeyframeIndex;
					}
					else
					{
						// We're freed from the animation object.
						return false;
					}
				}

				lastAnimKeyframe = &_keyframes[_lastKeyIndex];
			}
			else
			{
				if (animKeyframeBefore._timeInSecond == currentTime)
				{
					--_lastKeyIndex;
					outPos = animKeyframeBefore._position;
					outRot = animKeyframeBefore._rotation;
				}
				else
				{
					lerpKeyframe(*lastAnimKeyframe, animKeyframeBefore, currentTime, outPos, outRot);
				}

				break;
			}

		} while (true);
	}

	return _isLooping || _lastKeyIndex != lastKeyframeIndex;
}

std::size_t Storm::AnimationObject::getKeyframeCount() const
{
	return _keyframes.size();
}
