#pragma once

namespace Storm
{
	class Camera
	{
	public:
		Camera(float viewportWidth, float viewportHeight);

	public:
		void reset();

		float getNearPlane() const noexcept;
		float getFarPlane() const noexcept;
		float getFieldOfView() const noexcept;

		const DirectX::XMMATRIX& getProjectionMatrix() const noexcept;
		const DirectX::XMMATRIX& getViewMatrix() const noexcept;
		const DirectX::XMMATRIX& getOrthoMatrix() const noexcept;

		const DirectX::XMMATRIX& getTransposedProjectionMatrix() const noexcept;
		const DirectX::XMMATRIX& getTransposedViewMatrix() const noexcept;
		const DirectX::XMMATRIX& getTransposedOrthoMatrix() const noexcept;

		const DirectX::XMFLOAT3& getPosition() const noexcept;
		const DirectX::XMFLOAT3& getTarget() const noexcept;

		float getCameraMoveSpeed() const noexcept;
		float getCameraPlaneSpeed() const noexcept;

		void increaseCameraSpeed();
		void decreaseCameraSpeed();

		void setNearPlane(float nearPlane);
		void setFarPlane(float farPlane);

		void increaseNearPlane();
		void decreaseNearPlane();
		void increaseFarPlane();
		void decreaseFarPlane();

		void setPosition(float x, float y, float z);
		void setTarget(float x, float y, float z);

		void positiveMoveXAxis();
		void positiveMoveYAxis();
		void positiveMoveZAxis();
		void negativeMoveXAxis();
		void negativeMoveYAxis();
		void negativeMoveZAxis();

		void positiveRotateXAxis();
		void positiveRotateYAxis();
		void negativeRotateXAxis();
		void negativeRotateYAxis();

	private:
		void setPositionInternal(float x, float y, float z);
		void setTargetInternal(float x, float y, float z);

		void translateRelative(const Storm::Vector3 &deltaTranslation);

		void buildProjectionMatrix();
		void buildOrthoMatrix();
		void buildViewMatrix();

	private:
		DirectX::XMMATRIX _projectionMatrix;
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _orthoMatrix; // For 2D HUD

		DirectX::XMMATRIX _transposedProjectionMatrix;
		DirectX::XMMATRIX _transposedViewMatrix;
		DirectX::XMMATRIX _transposedOrthoMatrix; // For 2D HUD

		float _cameraMoveSpeed;
		float _cameraRotateSpeed;
		float _cameraPlaneSpeed;

		DirectX::XMFLOAT3 _position;
		DirectX::XMFLOAT3 _target;
		DirectX::XMVECTOR _up;

		float _nearPlane;
		float _farPlane;
		float _fieldOfView;
		float _screenWidth;
		float _screenHeight;
	};
}
