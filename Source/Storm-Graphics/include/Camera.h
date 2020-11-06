#pragma once



namespace Storm
{
	class UIFieldContainer;

	class Camera
	{
	public:
		Camera(float viewportWidth, float viewportHeight);
		~Camera();

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
		void setNearAndFarPlane(float nearPlane, float farPlane);

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

		void convertScreenPositionToRay(const Storm::Vector2 &screenPos, Storm::Vector3 &outRayOrigin, Storm::Vector3 &outRayDirection) const;
		Storm::Vector3 convertScreenPositionTo3DPosition(const Storm::Vector3 &screenPos3D) const;

		void rescaleScreenPosition(float &outXPos, float &outYPos) const;
		void setRescaledDimension(float newViewportWidth, float newViewportHeight);

	private:
		void setPositionInternal(float x, float y, float z);
		void setTargetInternal(float x, float y, float z);

		void setCameraMoveSpeed(float newSpeed);
		void setCameraRotateSpeed(float newSpeed);
		void setCameraPlaneSpeed(float newSpeed);

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
		float _rescaledScreenWidth;
		float _rescaledScreenHeight;

		bool _planesFixedFromTranslatMoves;

		std::unique_ptr<Storm::UIFieldContainer> _fields;
	};
}
