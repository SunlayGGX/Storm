#include "AnimationObject.h"
#include "AnimationKeyframe.h"

#include "SceneRigidBodyConfig.h"

#include "ThreadingSafety.h"

#include "XmlReader.h"

#include <boost\property_tree\ptree_fwd.hpp>
#include <boost\property_tree\xml_parser.hpp>



namespace
{
	void lerp(const Storm::Vector3 &vectBefore, const Storm::Vector3 &vectAfter, const float coeff, Storm::Vector3 &result)
	{
		result = vectBefore + (vectAfter - vectBefore) * coeff;
	}

	void lerp(const Storm::Rotation &vectBefore, const Storm::Rotation &vectAfter, const float coeff, Storm::Rotation &result)
	{
		result.axis() = vectBefore.axis();  // TODO : Maybe a slerp. But I don't need it for now since axises are constants
		result.angle() = std::lerp(vectBefore.angle(), vectAfter.angle(), coeff);
	}

	// Called keyframe a and b on purpose. a goes to b, no telling in what chronological direction it is
	// (if a time is less than b time, we goes to the future, otherwise we'll go to the past).
	void lerpKeyframe(const Storm::AnimationKeyframe &a, const Storm::AnimationKeyframe &b, const float time, Storm::Vector3 &outPos, Storm::Rotation &outRot)
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
	{
		std::stringstream str;
		str << rbSceneConfig._animationXmlContent;
		boost::property_tree::read_xml(str, rootXmlTree);
	}
	

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

			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.angle(), "angle");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.axis().x(), "rotX");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.axis().y(), "rotY");
			Storm::XmlReader::readXmlAttribute(keyframeElementXmlTree, keyframe._rotation.axis().z(), "rotZ");
		}
		else if (animationKeys.first == "Motor")
		{
			const boost::property_tree::ptree &motorElementXmlTree = animationKeys.second;

			Storm::Vector3 initialRbPosition = _keyframes.empty() ? rbSceneConfig._translation : (_keyframes.back()._position);

			Storm::Vector3 pivotPoint = initialRbPosition;
			Storm::XmlReader::readXmlAttribute(motorElementXmlTree, pivotPoint.x(), "pivotX");
			Storm::XmlReader::readXmlAttribute(motorElementXmlTree, pivotPoint.y(), "pivotY");
			Storm::XmlReader::readXmlAttribute(motorElementXmlTree, pivotPoint.z(), "pivotZ");

			Storm::Rotation rotationAxis = Storm::Rotation::Identity();
			Storm::XmlReader::readXmlAttribute(motorElementXmlTree, rotationAxis.axis().x(), "rotX");
			Storm::XmlReader::readXmlAttribute(motorElementXmlTree, rotationAxis.axis().y(), "rotY");
			Storm::XmlReader::readXmlAttribute(motorElementXmlTree, rotationAxis.axis().z(), "rotZ");

			const float rotationAxisNorm = rotationAxis.axis().norm();
			if (rotationAxisNorm < 0.00001f)
			{
				Storm::throwException<Storm::Exception>("Rotation axis shouldn't be a null vector!");
			}

			rotationAxis.axis() /= rotationAxisNorm;
			rotationAxis.angle() = std::fmodf(rotationAxis.angle(), static_cast<float>(2.0 * M_PI));

			float rotateSpeed;
			Storm::XmlReader::sureReadXmlAttribute(motorElementXmlTree, rotateSpeed, "speed");
			if (rotateSpeed == 0.f)
			{
				Storm::throwException<Storm::Exception>("Motor should move!");
			}

			constexpr unsigned int angularTurnDivision = 180;
			const float angularTimeDivision = 1.f / (static_cast<float>(angularTurnDivision) * std::fabs(rotateSpeed));

			constexpr float angularStep = static_cast<float>(2.0 * M_PI / static_cast<double>(angularTurnDivision));

			Storm::Rotation rotationAxisDescription = rotationAxis;
			rotationAxisDescription.angle() = angularStep;

			const bool rotateClockwise = rotateSpeed > 0.f;

			const Storm::Quaternion rotationQuaternion = rotateClockwise ? Storm::Quaternion{ rotationAxisDescription } : Storm::Quaternion{ rotationAxisDescription }.inverse();
			const Storm::Quaternion conjugateRotationQuaternion = rotationQuaternion.conjugate();

			_keyframes.reserve(_keyframes.size() + angularTurnDivision + 1);

			{
				bool noKeyframe = _keyframes.empty();
				Storm::Rotation currentRotation = noKeyframe ? rotationAxis : (_keyframes.back()._rotation);

				Storm::AnimationKeyframe &keyframe = _keyframes.emplace_back();
				keyframe._timeInSecond = noKeyframe ? 0.f : (_keyframes.back()._timeInSecond + angularTimeDivision);
				keyframe._position = initialRbPosition;
				keyframe._rotation = currentRotation;
			}

			Storm::Vector3 relativePos = initialRbPosition - pivotPoint;

			for (unsigned int iter = 1; iter < angularTurnDivision; ++iter)
			{
				const Storm::AnimationKeyframe &lastKeyframe = _keyframes.back();

				const Storm::Quaternion lastPosAsPureQuat{ 0.f, relativePos.x(), relativePos.y(), relativePos.z() };

				const Storm::Quaternion newPos = rotationQuaternion * lastPosAsPureQuat * conjugateRotationQuaternion;
				relativePos = newPos.vec();

				Storm::AnimationKeyframe &keyframe = _keyframes.emplace_back();
				keyframe._timeInSecond = lastKeyframe._timeInSecond + angularTimeDivision;
				keyframe._position = relativePos + pivotPoint;
				keyframe._rotation.axis() = lastKeyframe._rotation.axis(); // TODO : Maybe a slerp. But I don't need it for now since axises are constants
				keyframe._rotation.angle() = lastKeyframe._rotation.angle() + rotationAxisDescription.angle();
			}

			_isLooping = true;
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

bool Storm::AnimationObject::getCurrentFrameOrShouldBeDestroyed(float currentTime, Storm::Vector3 &outPos, Storm::Rotation &outRot) const
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
