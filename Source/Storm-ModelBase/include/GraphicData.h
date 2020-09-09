#pragma once


namespace Storm
{
	struct GraphicData
	{
	public:
		GraphicData();

	public:
		// Camera
		Storm::Vector3 _grid;
		Storm::Vector3 _cameraPosition;
		Storm::Vector3 _cameraLookAt;
		float _zNear;
		float _zFar;

		bool _displaySolidAsParticles;

		float _valueForMinColor;
		float _valueForMaxColor;

		float _blowerAlpha;

		// Constraints
		float _constraintThickness;
		Storm::Vector4 _constraintColor;
	};
}
