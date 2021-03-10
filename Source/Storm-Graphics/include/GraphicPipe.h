#pragma once


namespace Storm
{
	enum class ColoredSetting : uint8_t;
	struct GraphicParticleData;
	struct PushedParticleSystemDataParameter;

	class UIFieldContainer;

	// Even if the name contains pipe, it isn't thread safe and expect to be used as the pipe between Simulator and Graphics thread. From the Simulator thread side.
	class GraphicPipe
	{
	public:
		struct ColorSetting
		{
		public:
			operator std::string() const;

		public:
			float _minValue;
			float _maxValue;
		};

	public:
		GraphicPipe();
		~GraphicPipe();

	public:
		std::vector<Storm::GraphicParticleData> fastOptimizedTransCopy(const Storm::PushedParticleSystemDataParameter &param);

	private:
		Storm::GraphicPipe::ColorSetting& getColorSettingFromSelection(const Storm::ColoredSetting setting) const;

	public:
		void cycleColoredSetting();
		const Storm::GraphicPipe::ColorSetting& getUsedColorSetting() const;
		void setUsedColorSetting(const Storm::ColoredSetting setting);

		void setMinMaxColorationValue(float newMinValue, float newMaxValue, const Storm::ColoredSetting setting);
		void setMinMaxColorationValue(float newMinValue, float newMaxValue);
		void changeMinColorationValue(float deltaValue, const Storm::ColoredSetting setting);
		void changeMaxColorationValue(float deltaValue, const Storm::ColoredSetting setting);
		void changeMinColorationValue(float deltaValue);
		void changeMaxColorationValue(float deltaValue);

	private:
		void notifyCurrentGraphicPipeColorationSettingChanged() const;

	private:
		Storm::ColoredSetting _selectedColoredSetting;

		Storm::GraphicPipe::ColorSetting _velocitySetting;
		Storm::GraphicPipe::ColorSetting _pressureSetting;
		Storm::GraphicPipe::ColorSetting _densitySetting;

		const Storm::GraphicPipe::ColorSetting* _chosenColorSetting;
		std::wstring _coloredSettingWStr;
		std::unique_ptr<Storm::UIFieldContainer> _fields;
	};
}
