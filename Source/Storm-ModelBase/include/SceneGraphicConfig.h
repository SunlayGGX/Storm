#pragma once


namespace Storm
{
	struct SceneGraphicConfig
	{
	public:
		SceneGraphicConfig();

	public:
		// Camera
		Storm::Vector3 _grid;
		Storm::Vector3 _cameraPosition;
		Storm::Vector3 _cameraLookAt;
		float _zNear;
		float _zFar;

		bool _displaySolidAsParticles;

		float _velocityNormMinColor;
		float _velocityNormMaxColor;
		float _pressureMinColor;
		float _pressureMaxColor;
		float _densityMinColor;
		float _densityMaxColor;

		float _blowerAlpha;

		// Constraints
		float _constraintThickness;
		Storm::Vector4 _constraintColor;

		// Force
		float _forceThickness;
		Storm::Vector4 _forceColor;
	};
}
