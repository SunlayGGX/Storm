#pragma once


namespace Storm
{
	enum class GeometryType;

	struct GeometryConfig
	{
	public:
		enum class HoleModality : uint8_t
		{
			None =			0x0,
			OpenedXLeft =	0b000001,
			OpenedXRight =	0b000010,
			OpenedYUp =		0b000100,
			OpenedYDown =	0b001000,
			OpenedZFront =	0b010000,
			OpenedZBack =	0b100000,
		};

	public:
		GeometryConfig();

	public:
		Storm::GeometryType _type;

		// In case of Cube
		HoleModality _holeModality;

		// The sample count used as input for MDeserno Algorithm. Only useful if chosen sampler is the uniform one and _geometry is Storm::GeometryType::EquiSphere_MarkusDeserno.
		std::size_t _sampleCountMDeserno;
	};
}
