#include "Camera.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GraphicData.h"



Storm::Camera::Camera(float viewportWidth, float viewportHeight) :
	_screenRatio{ viewportWidth / viewportHeight },
	_fieldOfView{ DirectX::XM_PI / 4.f },
	_worldMatrix{ DirectX::XMMatrixIdentity() },
	_screenWidth{ viewportWidth },
	_screenHeight{ viewportHeight }
{
	Storm::IConfigManager* configMgr = Storm::SingletonHolder::instance().getFacet<Storm::IConfigManager>();
	const Storm::GraphicData &currentGraphicData = configMgr->getGraphicData();

	_nearPlane = currentGraphicData._zNear;
	_farPlane = currentGraphicData._zFar;

	_position.x = currentGraphicData._cameraPosition._x;
	_position.y = currentGraphicData._cameraPosition._y;
	_position.z = currentGraphicData._cameraPosition._z;

	_target.x = currentGraphicData._cameraLookAt._x;
	_target.y = currentGraphicData._cameraLookAt._y;
	_target.z = currentGraphicData._cameraLookAt._z;

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

float Storm::Camera::getScreenRatio() const noexcept
{
	return _screenRatio;
}

const DirectX::XMMATRIX& Storm::Camera::getWorldMatrix() const noexcept
{
	return _worldMatrix;
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

const DirectX::XMFLOAT3& Storm::Camera::getPosition() const noexcept
{
	return _position;
}

const DirectX::XMFLOAT3& Storm::Camera::getTarget() const noexcept
{
	return _target;
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

void Storm::Camera::setPosition(float x, float y, float z)
{
	_position.x = x;
	_position.y = y;
	_position.z = z;

	this->buildViewMatrix();
}

void Storm::Camera::setTarget(float x, float y, float z)
{
	_target.x = x;
	_target.y = y;
	_target.z = z;

	this->buildViewMatrix();

}

void Storm::Camera::moveXAxis(float dx)
{
	this->setPosition(_position.x + dx, _position.y, _position.z);
}

void Storm::Camera::moveYAxis(float dy)
{
	this->setPosition(_position.x, _position.y + dy, _position.z);
}

void Storm::Camera::moveZAxis(float dz)
{
	this->setPosition(_position.x, _position.y, _position.z + dz);
}

void Storm::Camera::buildProjectionMatrix()
{
	_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _screenRatio, _nearPlane, _farPlane);
}

void Storm::Camera::buildOrthoMatrix()
{
	_orthoMatrix = DirectX::XMMatrixOrthographicLH(_screenWidth, _screenHeight, _nearPlane, _farPlane);
}

void Storm::Camera::buildViewMatrix()
{
	_viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&_position), DirectX::XMLoadFloat3(&_target), _up);
}
