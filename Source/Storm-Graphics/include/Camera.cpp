#include "Camera.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GraphicData.h"

#include "UIFieldBase.h"
#include "UIFieldContainer.h"
#include "UIField.h"

#include "XMStormHelpers.h"

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
				Storm::throwException<std::exception>("Unknown translation axis!");
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
					Storm::throwException<std::exception>("Unknown translation axis!");
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
	private:
		static void removeUselessZeros(std::wstring &inOutValue)
		{
			while (inOutValue.back() == L'0')
			{
				inOutValue.pop_back();
			}
		}

	public:
		static std::wstring parseToWString(const DirectX::XMFLOAT3 &val)
		{
			std::wstring result;

			std::wstring xWStr = std::to_wstring(val.x);
			CustomFieldParser::removeUselessZeros(xWStr);

			std::wstring yWStr = std::to_wstring(val.y);
			CustomFieldParser::removeUselessZeros(yWStr);

			std::wstring zWStr = std::to_wstring(val.z);
			CustomFieldParser::removeUselessZeros(zWStr);

			result.reserve(4 + xWStr.size() + yWStr.size() + zWStr.size());

			result += xWStr;
			result += L", ";
			result += yWStr;
			result += L", ";
			result += zWStr;

			return result;
		}
	};
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
}

Storm::Camera::~Camera() = default;

void Storm::Camera::reset()
{
	this->setCameraMoveSpeed(1.f);
	this->setCameraPlaneSpeed(1.f);
	this->setCameraRotateSpeed(10.f);

	Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GraphicData &currentGraphicData = configMgr.getGraphicData();

	_nearPlane = currentGraphicData._zNear;
	_farPlane = currentGraphicData._zFar;

	_position.x = currentGraphicData._cameraPosition.x();
	_position.y = currentGraphicData._cameraPosition.y();
	_position.z = currentGraphicData._cameraPosition.z();

	_target.x = currentGraphicData._cameraLookAt.x();
	_target.y = currentGraphicData._cameraLookAt.y();
	_target.z = currentGraphicData._cameraLookAt.z();

	DirectX::XMFLOAT3 upFloat3{ 0.f, 1.f, 0.f };
	_up = DirectX::XMLoadFloat3(&upFloat3);

	this->buildProjectionMatrix();
	this->buildOrthoMatrix();
	this->buildViewMatrix();
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

	DirectX::XMMATRIX matVPInverse = DirectX::XMMatrixInverse(nullptr, matView * matProj);

	const Storm::Vector2 rescaledPosition{ screenPos.x() / _rescaledScreenWidth * _screenWidth, screenPos.y() / _rescaledScreenHeight * _screenHeight };

	DirectX::XMVECTOR screenPosTo3DPoint{ rescaledPosition.x(), rescaledPosition.y(), 0.f, 1.f };
	DirectX::XMVector4Transform(screenPosTo3DPoint, matVPInverse);
	outRayOrigin = Storm::convertToStorm(screenPosTo3DPoint);

	DirectX::XMVECTOR screenPosTo3DVector{ rescaledPosition.x(), rescaledPosition.y(), 1.f, 0.f };
	DirectX::XMVector4Transform(screenPosTo3DVector, matVPInverse);
	outRayDirection = Storm::convertToStorm(screenPosTo3DVector);
}

Storm::Vector3 Storm::Camera::convertScreenPositionTo3DPosition(const Storm::Vector3 &screenPos3D) const
{
	const DirectX::XMMATRIX &matView = this->getViewMatrix();
	const DirectX::XMMATRIX &matProj = this->getProjectionMatrix();

	// The vector in 3D
	DirectX::XMVECTOR vectClipSpace = Storm::convertToXM(screenPos3D);

#if false
	// Revert the z coming from the Z-buffer (therefore a normalized value between (0.f (the near plane) and 1.f (the far plane)) into a real depth z.
	const float z = matProj.r[3].m128_f32[2] / (screenPos3D.z() - matProj.r[2].m128_f32[2]);
	vectClipSpace.m128_f32[2] = z;
#endif

	// Unproject the screen pos into the world pos (normally, this method is meant to revert to object space, but using the identity matrix as the world pos makes the object space into the world space).
	const DirectX::XMVECTOR unprojected = DirectX::XMVector3Unproject(vectClipSpace, 0.f, 0.f, _screenWidth, _screenHeight, 0.f, 1.f, matProj, matView, DirectX::XMMatrixIdentity());

	return Storm::convertToStorm(unprojected);
}

void Storm::Camera::setRescaledDimension(float newViewportWidth, float newViewportHeight)
{
	_rescaledScreenWidth = newViewportWidth;
	_rescaledScreenHeight = newViewportHeight;
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
	if (_cameraMoveSpeed != newSpeed)
	{
		_cameraMoveSpeed = newSpeed;
		_fields->pushField(STORM_TRANSLATE_SPEED_FIELD_NAME);
	}
}

void Storm::Camera::setCameraRotateSpeed(float newSpeed) // In degrees
{
	if (_cameraRotateSpeed != newSpeed)
	{
		_cameraRotateSpeed = newSpeed;
		_fields->pushField(STORM_ROTATE_SPEED_FIELD_NAME);
	}
}

void Storm::Camera::setCameraPlaneSpeed(float newSpeed)
{
	_cameraPlaneSpeed = newSpeed;
}

void Storm::Camera::translateRelative(const Storm::Vector3 &deltaTranslation)
{
	this->setPositionInternal(_position.x + deltaTranslation.x(), _position.y + deltaTranslation.y(), _position.z + deltaTranslation.z());
	this->setTargetInternal(_target.x + deltaTranslation.x(), _target.y + deltaTranslation.y(), _target.z + deltaTranslation.z());

	this->buildViewMatrix();
}

void Storm::Camera::buildProjectionMatrix()
{
	const float screenRatio = _screenWidth / _screenHeight;
	_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, screenRatio, _nearPlane, _farPlane);
	_transposedProjectionMatrix = DirectX::XMMatrixTranspose(_projectionMatrix);
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
}
