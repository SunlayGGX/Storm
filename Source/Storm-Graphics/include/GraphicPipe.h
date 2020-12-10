#pragma once


namespace Storm
{
	enum class ColoredSetting : uint8_t;
	struct GraphicParticleData;
	struct PushedParticleSystemDataParameter;

	// Even if the name contains pipe, it isn't thread safe and expect to be used as the pipe between Simulator and Graphics thread. From the Simulator thread side.
	class GraphicPipe
	{
	public:
		struct ColorSetting
		{
			float _minValue;
			float _maxValue;
		};

	public:
		GraphicPipe();

	public:
		std::vector<Storm::GraphicParticleData> fastOptimizedTransCopy(const Storm::PushedParticleSystemDataParameter &param);

		void cycleColoredSetting();
		const Storm::GraphicPipe::ColorSetting& getUsedColorSetting() const;

	private:
		Storm::ColoredSetting _selectedColoredSetting;

		Storm::GraphicPipe::ColorSetting _velocitySetting;
		Storm::GraphicPipe::ColorSetting _pressureSetting;
		Storm::GraphicPipe::ColorSetting _densitySetting;
	};
}
