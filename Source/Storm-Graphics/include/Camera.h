#pragma once

namespace Storm
{
	class Camera
	{
	public:
		Camera(float viewportWidth, float viewportHeight);

	public:
		float getNearPlane() const noexcept;
		float getFarPlane() const noexcept;
		float getFieldOfView() const noexcept;
		float getScreenRatio() const noexcept;

		const DirectX::XMMATRIX& getWorldMatrix() const noexcept;
		const DirectX::XMMATRIX& getProjectionMatrix() const noexcept;
		const DirectX::XMMATRIX& getViewMatrix() const noexcept;
		const DirectX::XMMATRIX& getOrthoMatrix() const noexcept;

		const DirectX::XMFLOAT3& getPosition() const noexcept;
		const DirectX::XMFLOAT3& getTarget() const noexcept;

		void setNearPlane(float nearPlane);
		void setFarPlane(float farPlane);

		void setPosition(float x, float y, float z);
		void setTarget(float x, float y, float z);

	public:
		void buildProjectionMatrix();
		void buildOrthoMatrix();
		void buildViewMatrix();

	private:
		DirectX::XMMATRIX _worldMatrix;
		DirectX::XMMATRIX _projectionMatrix;
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _orthoMatrix; // For 2D HUD

		DirectX::XMFLOAT3 _position;
		DirectX::XMFLOAT3 _target;
		DirectX::XMVECTOR _up;

		float _nearPlane;
		float _farPlane;
		float _fieldOfView;
		float _screenRatio;
		float _screenWidth;
		float _screenHeight;
	};
}
