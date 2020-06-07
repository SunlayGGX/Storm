#include "Camera.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GraphicData.h"


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

	constexpr float getRotationAngleRad()
	{
		// 10 degrees
		return DirectX::XM_PI / 18.f;
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
}


Storm::Camera::Camera(float viewportWidth, float viewportHeight) :
	_fieldOfView{ DirectX::XM_PI / 4.f },
	_screenWidth{ viewportWidth },
	_screenHeight{ viewportHeight }
{
	this->reset();
}

void Storm::Camera::reset()
{
	_cameraMoveSpeed = 1.f;
	_cameraPlaneSpeed = 1.f;

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
	_cameraMoveSpeed *= 2.f;
	_cameraPlaneSpeed *= 2.f;
}

void Storm::Camera::decreaseCameraSpeed()
{
	_cameraMoveSpeed = std::max(_cameraMoveSpeed / 2.f, getMinimalNearPlaneValue());
	_cameraPlaneSpeed = std::max(_cameraPlaneSpeed / 2.f, getMinimalNearPlaneValue());
}

void Storm::Camera::setNearPlane(float nearPlane)
{
	_nearPlane = nearPlane;
	this->buildProjectionMatrix();
	this->buildOrthoMatrix();
}

void Storm::Camera::setFarPlane(float farPlane)
{
	_farPlane = farPlane;
	this->buildProjectionMatrix();
	this->buildOrthoMatrix();
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
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::X>(_position, _target, getRotationAngleRad());
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::positiveRotateYAxis()
{
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::Y>(_position, _target, getRotationAngleRad());
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::negativeRotateXAxis()
{
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::X>(_position, _target, -getRotationAngleRad());
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::negativeRotateYAxis()
{
	const Storm::Vector3 newPosition = rotateToNewPosition<RotateAxis::Y>(_position, _target, -getRotationAngleRad());
	this->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
}

void Storm::Camera::setPositionInternal(float x, float y, float z)
{
	_position.x = x;
	_position.y = y;
	_position.z = z;
}

void Storm::Camera::setTargetInternal(float x, float y, float z)
{
	_target.x = x;
	_target.y = y;
	_target.z = z;
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
