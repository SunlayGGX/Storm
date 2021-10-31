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

		void updateWatchedRb(const Storm::Vector3 &watchedRbPosition, const bool shouldTrackTranslation);
		void update();

		float getNearPlane() const noexcept;
		float getFarPlane() const noexcept;
		float getFieldOfView() const noexcept;

		const DirectX::XMMATRIX& getProjectionMatrix() const noexcept;
		const DirectX::XMMATRIX& getViewMatrix() const noexcept;
		const DirectX::XMMATRIX& getViewProjMatrix() const noexcept;
		const DirectX::XMMATRIX& getOrthoMatrix() const noexcept;

		const DirectX::XMMATRIX& getTransposedProjectionMatrix() const noexcept;
		const DirectX::XMMATRIX& getTransposedViewMatrix() const noexcept;
		const DirectX::XMMATRIX& getTransposedViewProjMatrix() const noexcept;
		const DirectX::XMMATRIX& getTransposedOrthoMatrix() const noexcept;

		const DirectX::XMFLOAT3& getPosition() const noexcept;
		const DirectX::XMFLOAT3& getTarget() const noexcept;

		float getCameraMoveSpeed() const noexcept;
		float getCameraPlaneSpeed() const noexcept;

		DirectX::XMVECTOR getDXEyePos() const;
		DirectX::XMVECTOR getDXEyeDir() const;

		Storm::Vector3 getEyePos() const;
		Storm::Vector3 getEyeDir() const;

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

		void makeCut(const DirectX::XMVECTOR &position, const float distance);
		void makeCut(const Storm::Vector3 &position, const float distance);

	private:
		void setPositionInternal(float x, float y, float z);
		void setTargetInternal(float x, float y, float z);

		void setCameraMoveSpeed(float newSpeed);
		void setCameraRotateSpeed(float newSpeed);
		void setCameraPlaneSpeed(float newSpeed);

		void translateRelative(const Storm::Vector3 &deltaTranslation, bool shouldSmooth);
		void translateRelative(const Storm::Vector3 &deltaTranslation);

		void buildProjectionMatrix();
		void buildOrthoMatrix();
		void buildViewMatrix();

		void buildViewProjectionMatrix();

	private:
		DirectX::XMMATRIX _projectionMatrix;
		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _viewProjMatrix;
		DirectX::XMMATRIX _orthoMatrix; // For 2D HUD

		DirectX::XMMATRIX _transposedProjectionMatrix;
		DirectX::XMMATRIX _transposedViewMatrix;
		DirectX::XMMATRIX _transposedViewProjMatrix; // For 2D HUD
		DirectX::XMMATRIX _transposedOrthoMatrix; // For 2D HUD

		DirectX::XMVECTOR _up;
		DirectX::XMFLOAT3 _position;
		DirectX::XMFLOAT3 _target;

		float _cameraMoveSpeed;
		float _cameraRotateSpeed;
		float _cameraPlaneSpeed;

		float _nearPlane;
		float _farPlane;
		float _fieldOfView;
		float _screenWidth;
		float _screenHeight;
		float _rescaledScreenWidth;
		float _rescaledScreenHeight;

		bool _planesFixedFromTranslatMoves;

		bool _shouldMoveSmooth;
		int _startSmoothMoveFramePos;
		Storm::Vector3 _deltaTranslationSmooth;

		std::unique_ptr<Storm::UIFieldContainer> _fields;
	};
}
