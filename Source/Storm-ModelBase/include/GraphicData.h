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
	};
}
