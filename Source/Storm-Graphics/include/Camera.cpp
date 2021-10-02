#include "Camera.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GeneralGraphicConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneSimulationConfig.h"

#include "UIFieldBase.h"
#include "UIFieldContainer.h"
#include "UIField.h"

#include "XMStormHelpers.h"
#include "GraphicHelpers.h"

#define STORM_CAMERA_POSITION_FIELD_NAME "Camera"
#define STORM_TARGET_POSITION_FIELD_NAME "Target"
#define STORM_ZNEAR_FIELD_NAME "zNear"
#define STORM_ZFAR_FIELD_NAME "zFar"
#define STORM_TRANSLATE_SPEED_FIELD_NAME "Cam trans. speed"
#define STORM_ROTATE_SPEED_FIELD_NAME "Cam rot. speed"


namespace
{
	enum class RotateAxis
	{
		X,
		Y,
	};

	enum class TranslateAxis
	{
		X,
		Y,
		Z,
	};

	template<RotateAxis axis>
	Storm::Vector3 rotateToNewPosition(const DirectX::XMFLOAT3 &dxPosition, const DirectX::XMFLOAT3 &dxTarget, float rad)
	{
		Storm::Vector3 position{ dxPosition.x, dxPosition.y, dxPosition.z };

		if (rad != 0.f)
		{
			const Storm::Vector3 target{ dxTarget.x, dxTarget.y, dxTarget.z };
			const Storm::Vector3 worldUp{ 0.f, 1.f, 0.f };

			/* Compute relative coord system. */

			Storm::Vector3 relativeZ = target - position;
			float distToTarget = relativeZ.norm();
			if (distToTarget < 0.0001f)
			{
				relativeZ = Storm::Vector3{ 0.f, 0.f, 1.f };
			}
			else
			{
				relativeZ /= distToTarget;
			}

			Storm::Vector3 relativeX = relativeZ.cross(worldUp);
			if (relativeX.isZero())
			{
				// If relativeX is 0 and since relativeX was computed from relativeZ and up, then relativeZ is collinear to up.
				// If it is AND since we prevent roll (even if then we would have a roll because we reverse, but we set it to a 0 degree roll), then relativeX is the world x.
				relativeX = Storm::Vector3{ 1.f, 0.f, 0.f };
			}
			else
			{
				relativeX.normalize();
			}

			// We're left-handed
			Storm::Vector3 relativeY = relativeX.cross(relativeZ).normalized();


			/* Translate with new distance to make (an approx) */

			// This is the vector to the new position in a trigonometric unit circle with target as a center and position at (1,0).
			// Since everything is normalized, this vector will be also normalized.
			Storm::Vector3 trigonometricDisplacmentVector;

			if constexpr (axis == RotateAxis::X)
			{
				trigonometricDisplacmentVector = cosf(rad) * relativeZ + sinf(rad) * relativeY;
			}
			else if constexpr (axis == RotateAxis::Y)
			{
				trigonometricDisplacmentVector = cosf(rad) * relativeZ + sinf(rad) * relativeX;
			}
			else
			{
				Storm::throwException<Storm::Exception>("Unknown translation axis!");
			}

			position = target - distToTarget * trigonometricDisplacmentVector;
		}

		return position;
	}

	constexpr float getRotationAngleRadCoeff()
	{
		return DirectX::XM_PI / 180.f;
	}

	template<TranslateAxis axis>
	Storm::Vector3 computePositionDisplacementTranslation(const DirectX::XMFLOAT3 &dxPosition, const DirectX::XMFLOAT3 &dxTarget, float dist)
	{
		if (dist != 0.f)
		{
			const Storm::Vector3 position{ dxPosition.x, dxPosition.y, dxPosition.z };
			const Storm::Vector3 target{ dxTarget.x, dxTarget.y, dxTarget.z };
			const Storm::Vector3 worldUp{ 0.f, 1.f, 0.f };

			/* Compute relative coord system. */

			Storm::Vector3 relativeZ = target - position;
			float distToTarget = relativeZ.norm();
			if (distToTarget < 0.0001f)
			{
				relativeZ = Storm::Vector3{ 0.f, 0.f, 1.f };
			}
			else
			{
				relativeZ /= distToTarget;
			}

			if constexpr (axis == TranslateAxis::Z)
			{
				return dist * relativeZ;
			}
			else
			{
				Storm::Vector3 relativeX = relativeZ.cross(worldUp);
				if (relativeX.isZero())
				{
					// If relativeX is 0 and since relativeX was computed from relativeZ and up, then relativeZ is collinear to up.
					// If it is AND since we prevent roll (even if then we would have a roll because we reverse, but we set it to a 0 degree roll), then relativeX is the world x.
					relativeX = Storm::Vector3{ 1.f, 0.f, 0.f };
				}
				else
				{
					relativeX.normalize();
				}

				if constexpr (axis == TranslateAxis::X)
				{
					return dist * relativeX;
				}
				else if constexpr (axis == TranslateAxis::Y)
				{
					// We're left-handed
					const Storm::Vector3 relativeY = relativeX.cross(relativeZ).normalized();
					return dist * relativeY;
				}
				else
				{
					Storm::throwException<Storm::Exception>("Unknown translation axis!");
				}
			}
		}

		return Storm::Vector3::Zero();
	}

	constexpr float getMinimalNearPlaneValue()
	{
		return 0.001f;
	}

	struct CustomFieldParser
	{
	public:
		static std::wstring parseToWString(const DirectX::XMFLOAT3 &val)
		{
			std::wstring result;

			std::wstring xWStr = std::to_wstring(val.x);
			Storm::GraphicHelpers::removeUselessZeros(xWStr);

			std::wstring yWStr = std::to_wstring(val.y);
			Storm::GraphicHelpers::removeUselessZeros(yWStr);

			std::wstring zWStr = std::to_wstring(val.z);
			Storm::GraphicHelpers::removeUselessZeros(zWStr);

			result.reserve(4 + xWStr.size() + yWStr.size() + zWStr.size());

			result += xWStr;
			result += L", ";
			result += yWStr;
			result += L", ";
			result += zWStr;

			return result;
		}
	};

	enum
	{
		k_noFrame = -1,
		k_finalSmoothFrameIter = 120 * 12,

		// The step x from 0 to 1 is divided by 62 samples. The 2 samples are to prevent the start 0th and last frame iteration to have a complete speed of 0.
		// So this is a sampling of k_finalSmoothFrameIter ticks + the 2 ticks at the beginning and ending of the domain.
		k_smoothFrameCountNormalized = k_finalSmoothFrameIter + 2,
	};

	__forceinline bool isValidFrame(const int frameIter)
	{
		return frameIter != k_noFrame;
	}

	__forceinline bool isNearZero(const float value)
	{
		return std::fabs(value) < 0.000001f;
	}

	Storm::Vector3 extractSmoothedDisplacement(const Storm::Vector3 &srcDisplacement, const int frameIter)
	{
		const float currentStepX = static_cast<float>(frameIter + 1) / static_cast<float>(k_smoothFrameCountNormalized);

		// We want a formula that starts with a smooth velocity (start from 0 and increase gradually until the climax),
		// then climax in the middle (max speed at the (k_finalSmoothFrameIter/2)th sample), then goes back to 0 at the k_smoothFrameCountNormalized th sample.
		// Plus, we want to have 0 displacement remaining at the 60th sample.
		// In addition, we pose that there should be no remaining displacement.
		// 
		// The function is 6(x - x²) => To find this function, we posed f(0) = 0, f(1) = 0, f(0.5) = 1. And this is a degree 2 polynomial function.
		// Those values are temporaries and just express that the climax is at 50% of the curve, and we start and end with no velocity.
		// Making the formula : 4(x - x²). Plus another assumption is that we would like to have no displacement remaining : the integral of such function should be equal to 1.
		// Except the integral between 0 and 1 of 4(x - x²) is 2/3, then the normalization coefficient is 3/2.
		// 3/2 * 4(x - x²) <=> 6(x-x²)

		const float normalizationCoeff = 6.f * (currentStepX - currentStepX * currentStepX);

		return srcDisplacement * normalizationCoeff;
	}
}


Storm::Camera::Camera(float viewportWidth, float viewportHeight) :
	_fieldOfView{ DirectX::XM_PI / 4.f },
	_screenWidth{ viewportWidth },
	_screenHeight{ viewportHeight },
	_rescaledScreenWidth{ viewportWidth },
	_rescaledScreenHeight{ viewportHeight },
	_fields{ std::make_unique<Storm::UIFieldContainer>() }
{
	this->reset();

	(*_fields)
		.bindField(STORM_CAMERA_POSITION_FIELD_NAME, _position)
		.bindField(STORM_TARGET_POSITION_FIELD_NAME, _target)
		.bindField(STORM_TRANSLATE_SPEED_FIELD_NAME, _cameraMoveSpeed)
		.bindField(STORM_ROTATE_SPEED_FIELD_NAME, _cameraRotateSpeed)
		.bindField(STORM_ZNEAR_FIELD_NAME, _nearPlane)
		.bindField(STORM_ZFAR_FIELD_NAME, _farPlane)
		;

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralGraphicConfig &generalGraphicConfig = configMgr.getGeneralGraphicConfig();
	_planesFixedFromTranslatMoves = generalGraphicConfig._fixNearFarPlanesWhenTranslating;
	_shouldMoveSmooth = generalGraphicConfig._smoothCameraTransition;
}

Storm::Camera::~Camera() = default;

void Storm::Camera::reset()
{
	this->setCameraMoveSpeed(1.f);
	this->setCameraPlaneSpeed(1.f);
	this->setCameraRotateSpeed(10.f);

	_deltaTranslationSmooth = Storm::Vector3::Zero();
	_startSmoothMoveFramePos = k_noFrame;

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneGraphicConfig &sceneGraphicConfig = configMgr.getSceneGraphicConfig();
	const Storm::GeneralGraphicConfig &generalGraphicConfig = configMgr.getGeneralGraphicConfig();

	_nearPlane = sceneGraphicConfig._zNear;
	_farPlane = sceneGraphicConfig._zFar;

	this->setPositionInternal(sceneGraphicConfig._cameraPosition.x(), sceneGraphicConfig._cameraPosition.y(), sceneGraphicConfig._cameraPosition.z());
	this->setTargetInternal(sceneGraphicConfig._cameraLookAt.x(), sceneGraphicConfig._cameraLookAt.y(), sceneGraphicConfig._cameraLookAt.z());

	const Storm::Vector3 &gravity = configMgr.getSceneSimulationConfig()._gravity;
	if (generalGraphicConfig._spinCameraToGravityUp && !gravity.isZero())
	{
		_up = Storm::convertToXM(-gravity);
		_up.m128_f32[3] = 0.f;
	}
	else
	{
		DirectX::XMFLOAT3 upFloat3{ 0.f, 1.f, 0.f };
		_up = DirectX::XMLoadFloat3(&upFloat3);
	}

	this->buildProjectionMatrix();
	this->buildOrthoMatrix();
	this->buildViewMatrix();
}

void Storm::Camera::updateWatchedRb(const Storm::Vector3 &watchedRbPosition)
{
	const DirectX::XMVECTOR watchedRbPositionAsDX = Storm::convertToXM(watchedRbPosition);
	const DirectX::XMVECTOR currentCamPosition = DirectX::XMLoadFloat3(&_position);
	const DirectX::XMVECTOR targetPosition = DirectX::XMLoadFloat3(&_target);

	const DirectX::XMVECTOR eyeDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(targetPosition, currentCamPosition));
	const DirectX::XMVECTOR camToWatchedRb = DirectX::XMVectorSubtract(watchedRbPositionAsDX, currentCamPosition);

	const float newExpectedNearPlane = DirectX::XMVector3Dot(eyeDir, camToWatchedRb).m128_f32[0];

	constexpr float nearPlaneEpsilon = getMinimalNearPlaneValue();
	if (newExpectedNearPlane > nearPlaneEpsilon)
	{
		if (newExpectedNearPlane < _farPlane)
		{
			this->setNearPlane(newExpectedNearPlane);
		}
		else
		{
			this->setNearAndFarPlane(newExpectedNearPlane, newExpectedNearPlane + nearPlaneEpsilon);
		}
	}
}

void Storm::Camera::update()
{
	if (_shouldMoveSmooth)
	{
		if (isValidFrame(_startSmoothMoveFramePos))
		{
			if (_deltaTranslationSmooth != Storm::Vector3::Zero())
			{
				++_startSmoothMoveFramePos;

				const Storm::Vector3 partTranslation = extractSmoothedDisplacement(_deltaTranslationSmooth, _startSmoothMoveFramePos);

				this->translateRelative(partTranslation, false);

				// It adds another level of smoothness and allows a smaller object ;). But it is not part of the explanation on the mathematical formula described inside extractSmoothedDisplacement.
				// This makes the mathematical abstraction a little off, but the visual effect is really really good.
				_deltaTranslationSmooth -= partTranslation;

				if (isNearZero(_deltaTranslationSmooth.x()) && isNearZero(_deltaTranslationSmooth.y()) && isNearZero(_deltaTranslationSmooth.z()))
				{
					this->translateRelative(_deltaTranslationSmooth, false);

					_deltaTranslationSmooth = Storm::Vector3::Zero();
					_startSmoothMoveFramePos = k_noFrame;
				}
			}
			else
			{
				_startSmoothMoveFramePos = k_noFrame;
			}
		}
	}
}

float Storm::Camera::getNearPlane() const noexcept
{
	return _nearPlane;
}

float Storm::Camera::getFarPlane() const noexcept
{
	return _farPlane;
}

float Storm::Camera::getFieldOfView() const noexcept
{
	return _fieldOfView;
}

const DirectX::XMMATRIX& Storm::Camera::getProjectionMatrix() const noexcept
{
	return _projectionMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getViewMatrix() const noexcept
{
	return _viewMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getViewProjMatrix() const noexcept
{
	return _viewProjMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getOrthoMatrix() const noexcept
{
	return _orthoMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getTransposedProjectionMatrix() const noexcept
{
	return _transposedProjectionMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getTransposedViewMatrix() const noexcept
{
	return _transposedViewMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getTransposedViewProjMatrix() const noexcept
{
	return _transposedViewProjMatrix;
}

const DirectX::XMMATRIX& Storm::Camera::getTransposedOrthoMatrix() const noexcept
{
	return _transposedOrthoMatrix;
}

const DirectX::XMFLOAT3& Storm::Camera::getPosition() const noexcept
{
	return _position;
}

const DirectX::XMFLOAT3& Storm::Camera::getTarget() const noexcept
{
	return _target;
}

float Storm::Camera::getCameraMoveSpeed() const noexcept
{
	return _cameraMoveSpeed;
}

float Storm::Camera::getCameraPlaneSpeed() const noexcept
{
	return _cameraPlaneSpeed;
}

DirectX::XMVECTOR Storm::Camera::getDXEyePos() const
{
	return DirectX::XMLoadFloat3(&this->getPosition());
}

DirectX::XMVECTOR Storm::Camera::getDXEyeDir() const
{
	DirectX::XMVECTOR dxEyePos = this->getDXEyePos();
	DirectX::XMVECTOR dxTargetPos = DirectX::XMLoadFloat3(&this->getTarget());
	return DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(dxTargetPos, dxEyePos));
}

Storm::Vector3 Storm::Camera::getEyePos() const
{
	return Storm::convertToStorm(this->getDXEyePos());
}

Storm::Vector3 Storm::Camera::getEyeDir() const
{
	return Storm::convertToStorm(this->getDXEyeDir());
}

void Storm::Camera::increaseCameraSpeed()
{
	this->setCameraMoveSpeed(_cameraMoveSpeed * 2.f);

	// Don't go beyond 45 degrees...
	this->setCameraRotateSpeed(std::min(_cameraRotateSpeed * 2.f, 45.f));

	this->setCameraPlaneSpeed(_cameraPlaneSpeed * 2.f);
}

void Storm::Camera::decreaseCameraSpeed()
{
	this->setCameraMoveSpeed(std::max(_cameraMoveSpeed / 2.f, getMinimalNearPlaneValue()));
	this->setCameraRotateSpeed(std::max(_cameraRotateSpeed / 2.f, 0.1f));
	this->setCameraPlaneSpeed(std::max(_cameraPlaneSpeed / 2.f, getMinimalNearPlaneValue()));
}

void Storm::Camera::setNearPlane(float nearPlane)
{
	if (_nearPlane != nearPlane)
	{
		_nearPlane = nearPlane;
		this->buildProjectionMatrix();
		this->buildOrthoMatrix();

		_fields->pushField(STORM_ZNEAR_FIELD_NAME);
	}
}

void Storm::Camera::setFarPlane(float farPlane)
{
	if (_farPlane != farPlane)
	{
		_farPlane = farPlane;
		this->buildProjectionMatrix();
		this->buildOrthoMatrix();

		_fields->pushField(STORM_ZFAR_FIELD_NAME);
	}
}

void Storm::Camera::setNearAndFarPlane(float nearPlane, float farPlane)
{
	constexpr float nearPlaneEpsilon = getMinimalNearPlaneValue();
	if (nearPlane >= (farPlane - nearPlaneEpsilon))
	{
		LOG_ERROR <<
			"Cannot set near plane to a value equal or greater than far plane.\n"
			"Requested near plane: " << nearPlane << ".\n"
			"Requested far plane: " << farPlane << ".\n"
			;
		return;
	}

	bool rebuildMatrices = false;

	if (_nearPlane != nearPlane)
	{
		_nearPlane = nearPlane;
		_fields->pushField(STORM_ZNEAR_FIELD_NAME);
		rebuildMatrices = true;
	}

	if (_farPlane != farPlane)
	{
		_farPlane = farPlane;
		_fields->pushField(STORM_ZFAR_FIELD_NAME);
		rebuildMatrices = true;
	}

	if (rebuildMatrices)
	{
		this->buildProjectionMatrix();
		this->buildOrthoMatrix();
	}
}

void Storm::Camera::increaseNearPlane()
{
	float expected = _nearPlane + _cameraPlaneSpeed;
	constexpr float nearPlaneEpsilon = getMinimalNearPlaneValue();
	if (expected >= (_farPlane - nearPlaneEpsilon))
	{
		const float newExpected = std::max(_farPlane - _cameraPlaneSpeed, nearPlaneEpsilon);
		LOG_ERROR << 
			"Cannot set near plane to a value equal or greater than far plane. Therefore we will set it to " << newExpected << ".\n"
			"Expected : " << expected << ".\n"
			"Current far plane value : " << _farPlane << '.'
			;

		if (newExpected == _nearPlane)
		{
			return;
		}

		expected = newExpected;
	}

	this->setNearPlane(expected);
}

void Storm::Camera::decreaseNearPlane()
{
	float expected = _nearPlane - _cameraPlaneSpeed;
	constexpr float minimalNearPlaneValue = getMinimalNearPlaneValue();
	if (expected < minimalNearPlaneValue)
	{
		LOG_WARNING << "Cannot set the near plane distance under " << minimalNearPlaneValue << '.';
		expected = minimalNearPlaneValue;
	}
	this->setNearPlane(expected);
}

void Storm::Camera::increaseFarPlane()
{
	this->setFarPlane(_farPlane + _cameraPlaneSpeed);
}

void Storm::Camera::decreaseFarPlane()
{
	float expected = _farPlane - _cameraPlaneSpeed;
	constexpr float nearPlaneEpsilon = getMinimalNearPlaneValue();
	if (expected <= (_nearPlane + nearPlaneEpsilon))
	{
		const float newExpected = _nearPlane + _cameraPlaneSpeed;
		LOG_ERROR <<
			"Cannot set near plane to a value equal or greater than far plane. Therefore we will set it to " << newExpected << ".\n"
			"Expected : " << expected << ".\n"
			"Current far plane value : " << _farPlane << '.'
			;

		if (newExpected == _farPlane)
		{
			return;
		}

		expected = newExpected;
	}

	this->setFarPlane(expected);
}

void Storm::Camera::setPosition(float x, float y, float z)
{
	this->setPositionInternal(x, y, z);
	this->buildViewMatrix();
}

void Storm::Camera::setTarget(float x, float y, float z)
{
	this->setTargetInternal(x, y, z);
	this->buildViewMatrix();
}

void Storm::Camera::positiveMoveXAxis()
{
	const Storm::Vector3 deltaTranslation = computePositionDisplacementTranslation<TranslateAxis::X>(_position, _target, _cameraMoveSpeed);
	this->translateRelative(deltaTranslation);
}

void Storm::Camera::positiveMoveYAxis()
{
	const Storm::Vector3 deltaTranslation = computePositionDisplacementTranslation<TranslateAxis::Y>(_position, _target, _cameraMoveSpeed);
	this->translateRelative(deltaTranslation);
}

void Storm::Camera::positiveMoveZAxis()
{
	const Storm::Vector3 deltaTranslation = computePositionDisplacementTranslation<TranslateAxis::Z>(_position, _target, _cameraMoveSpeed);
	this->translateRelative(deltaTranslation);

	if (_planesFixedFromTranslatMoves)
	{
		this->setNearAndFarPlane(_nearPlane - _cameraMoveSpeed, _farPlane - _cameraMoveSpeed);
	}
}

void Storm::Camera::negativeMoveXAxis()
{
	const Storm::Vector3 deltaTranslation = computePositionDisplacementTranslation<TranslateAxis::X>(_position, _target, -_cameraMoveSpeed);
	this->translateRelative(deltaTranslation);
}

void Storm::Camera::negativeMoveYAxis()
{
	const Storm::Vector3 deltaTranslation = computePositionDisplacementTranslation<TranslateAxis::Y>(_position, _target, -_cameraMoveSpeed);
	this->translateRelative(deltaTranslation);
}

void Storm::Camera::negativeMoveZAxis()
{
	const Storm::Vector3 deltaTranslation = computePositionDisplacementTranslation<TranslateAxis::Z>(_position, _target, -_cameraMoveSpeed);
	this->translateRelative(deltaTranslation);

	if (_planesFixedFromTranslatMoves)
	{
		this->setNearAndFarPlane(_nearPlane + _cameraMoveSpeed, _farPlane + _cameraMoveSpeed);
	}
}

void Storm::Camera::positiveRotateXAxis()
{
	const float rotateDegrees = _cameraRotateSpeed * getRotationAngleRadCoeff();
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::X>(_position, _target, rotateDegrees);
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::positiveRotateYAxis()
{
	const float rotateDegrees = _cameraRotateSpeed * getRotationAngleRadCoeff();
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::Y>(_position, _target, rotateDegrees);
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::negativeRotateXAxis()
{
	const float rotateDegrees = -_cameraRotateSpeed * getRotationAngleRadCoeff();
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::X>(_position, _target, rotateDegrees);
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::negativeRotateYAxis()
{
	const float rotateDegrees = -_cameraRotateSpeed * getRotationAngleRadCoeff();
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::Y>(_position, _target, rotateDegrees);
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::convertScreenPositionToRay(const Storm::Vector2 &screenPos, Storm::Vector3 &outRayOrigin, Storm::Vector3 &outRayDirection) const
{
	const DirectX::XMMATRIX &matView = this->getViewMatrix();
	const DirectX::XMMATRIX &matProj = this->getProjectionMatrix();

	const DirectX::XMMATRIX matVPInverse = DirectX::XMMatrixInverse(nullptr, matView * matProj);

	const Storm::Vector2 rescaledPosition{ screenPos.x() / _rescaledScreenWidth * _screenWidth, screenPos.y() / _rescaledScreenHeight * _screenHeight };

	const DirectX::XMVECTOR screenPosTo3DPoint{ rescaledPosition.x(), rescaledPosition.y(), 0.f, 1.f };
	DirectX::XMVector4Transform(screenPosTo3DPoint, matVPInverse);
	outRayOrigin = Storm::convertToStorm(screenPosTo3DPoint);

	const DirectX::XMVECTOR screenPosTo3DVector{ rescaledPosition.x(), rescaledPosition.y(), 1.f, 0.f };
	DirectX::XMVector4Transform(screenPosTo3DVector, matVPInverse);
	outRayDirection = Storm::convertToStorm(screenPosTo3DVector);
}

Storm::Vector3 Storm::Camera::convertScreenPositionTo3DPosition(const Storm::Vector3 &screenPos3D) const
{
	const DirectX::XMMATRIX &matView = this->getViewMatrix();
	const DirectX::XMMATRIX &matProj = this->getProjectionMatrix();

	// The vector in 3D
	const DirectX::XMVECTOR vectClipSpace = Storm::convertToXM(screenPos3D);

#if false
	// Revert the z coming from the Z-buffer (therefore a normalized value between (0.f (the near plane) and 1.f (the far plane)) into a real depth z.
	const float z = matProj.r[3].m128_f32[2] / (screenPos3D.z() - matProj.r[2].m128_f32[2]);
	vectClipSpace.m128_f32[2] = z;
#endif

	// Unproject the screen pos into the world pos (normally, this method is meant to revert to object space, but using the identity matrix as the world pos makes the object space into the world space).
	const DirectX::XMVECTOR unprojected = DirectX::XMVector3Unproject(vectClipSpace, 0.f, 0.f, _screenWidth, _screenHeight, 0.f, 1.f, matProj, matView, DirectX::XMMatrixIdentity());

	return Storm::convertToStorm(unprojected);
}

void Storm::Camera::rescaleScreenPosition(float &outXPos, float &outYPos) const
{
	outXPos = outXPos / _rescaledScreenWidth * _screenWidth;
	outYPos = outYPos / _rescaledScreenHeight * _screenHeight;
}

void Storm::Camera::setRescaledDimension(float newViewportWidth, float newViewportHeight)
{
	_rescaledScreenWidth = newViewportWidth;
	_rescaledScreenHeight = newViewportHeight;
}

void Storm::Camera::makeCut(const DirectX::XMVECTOR &position, const float distance)
{
	const DirectX::XMVECTOR dxEyePos = this->getDXEyePos();
	const DirectX::XMVECTOR dxEyeDir = this->getDXEyeDir();

	const float newExpectedNearPlane = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(position, dxEyePos), dxEyeDir).m128_f32[0];
	if (newExpectedNearPlane > 0.f)
	{
		this->setNearAndFarPlane(newExpectedNearPlane, newExpectedNearPlane + distance);
	}
	else
	{
		LOG_ERROR << "Position is behind the eye. We cannot make a cut behind what we currently see.";
	}
}

void Storm::Camera::makeCut(const Storm::Vector3 &position, const float distance)
{
	this->makeCut(Storm::convertToXM(position), distance);
}

void Storm::Camera::setPositionInternal(float x, float y, float z)
{
	if (_position.x != x || _position.y != y || _position.z != z)
	{
		_position.x = x;
		_position.y = y;
		_position.z = z;

		_fields->pushField(STORM_CAMERA_POSITION_FIELD_NAME);
	}
}

void Storm::Camera::setTargetInternal(float x, float y, float z)
{
	if (_target.x != x || _target.y != y || _target.z != z)
	{
		_target.x = x;
		_target.y = y;
		_target.z = z;

		_fields->pushField(STORM_TARGET_POSITION_FIELD_NAME);
	}
}

void Storm::Camera::setCameraMoveSpeed(float newSpeed)
{
	Storm::updateField(*_fields, STORM_TRANSLATE_SPEED_FIELD_NAME, _cameraMoveSpeed, newSpeed);
}

void Storm::Camera::setCameraRotateSpeed(float newSpeed) // In degrees
{
	Storm::updateField(*_fields, STORM_ROTATE_SPEED_FIELD_NAME, _cameraRotateSpeed, newSpeed);
}

void Storm::Camera::setCameraPlaneSpeed(float newSpeed)
{
	_cameraPlaneSpeed = newSpeed;
}

void Storm::Camera::translateRelative(const Storm::Vector3 &deltaTranslation, bool shouldSmooth)
{
	if (shouldSmooth)
	{
		_deltaTranslationSmooth += deltaTranslation;
		_startSmoothMoveFramePos = 0;
	}
	else
	{
		this->setPositionInternal(_position.x + deltaTranslation.x(), _position.y + deltaTranslation.y(), _position.z + deltaTranslation.z());
		this->setTargetInternal(_target.x + deltaTranslation.x(), _target.y + deltaTranslation.y(), _target.z + deltaTranslation.z());

		this->buildViewMatrix();
	}
}

void Storm::Camera::translateRelative(const Storm::Vector3 &deltaTranslation)
{
	this->translateRelative(deltaTranslation, _shouldMoveSmooth);
}

void Storm::Camera::buildProjectionMatrix()
{
	const float screenRatio = _screenWidth / _screenHeight;
	_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, screenRatio, _nearPlane, _farPlane);
	_transposedProjectionMatrix = DirectX::XMMatrixTranspose(_projectionMatrix);

	this->buildViewProjectionMatrix();
}

void Storm::Camera::buildOrthoMatrix()
{
	_orthoMatrix = DirectX::XMMatrixOrthographicLH(_screenWidth, _screenHeight, _nearPlane, _farPlane);
	_transposedOrthoMatrix = DirectX::XMMatrixTranspose(_orthoMatrix);
}

void Storm::Camera::buildViewMatrix()
{
	_viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&_position), DirectX::XMLoadFloat3(&_target), _up);
	_transposedViewMatrix = DirectX::XMMatrixTranspose(_viewMatrix);

	this->buildViewProjectionMatrix();
}

void Storm::Camera::buildViewProjectionMatrix()
{
	_viewProjMatrix = DirectX::XMMatrixMultiply(_viewMatrix, _projectionMatrix);
	_transposedViewProjMatrix = DirectX::XMMatrixTranspose(_viewProjMatrix);
}
