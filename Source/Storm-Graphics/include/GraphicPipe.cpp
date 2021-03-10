#include "GraphicPipe.h"

#include "GraphicParticleData.h"

#include "PushedParticleSystemData.h"
#include "ColoredSetting.h"

#include "RunnerHelper.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "ISimulatorManager.h"

#include "SceneGraphicConfig.h"
#include "SceneRigidBodyConfig.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#if _WIN32
#	define STORM_HIJACKED_TYPE Storm::GraphicParticleData
#		include "VectHijack.h"
#	undef STORM_HIJACKED_TYPE
#endif

namespace
{
	constexpr DirectX::XMVECTOR getDefaultFluidParticleColor()
	{
		return { 0.3f, 0.5f, 0.85f, 1.f };
	}

	DirectX::XMVECTOR getRbColor(unsigned int rbId)
	{
		STORM_STATIC_ASSERT(sizeof(DirectX::XMFLOAT4) == sizeof(Storm::Vector4), "The folowing lines is an optimization that works only if the type have the same layout.");

		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		const Storm::Vector4 &colorRGBA = configMgr.getSceneRigidBodyConfig(rbId)._color;

		return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&colorRGBA.x()));
	}

	template<Storm::ColoredSetting coloredSetting>
	float getColoredMonoDimensionUsedData(const Storm::PushedParticleSystemDataParameter &param, const std::size_t iter);

	template<>
	float getColoredMonoDimensionUsedData<Storm::ColoredSetting::Velocity>(const Storm::PushedParticleSystemDataParameter &param, const std::size_t iter)
	{
		return (*param._velocityData)[iter].squaredNorm();
	}

	template<>
	float getColoredMonoDimensionUsedData<Storm::ColoredSetting::Pressure>(const Storm::PushedParticleSystemDataParameter &param, const std::size_t iter)
	{
		return (*param._pressureData)[iter];
	}

	template<>
	float getColoredMonoDimensionUsedData<Storm::ColoredSetting::Density>(const Storm::PushedParticleSystemDataParameter &param, const std::size_t iter)
	{
		return (*param._densityData)[iter];
	}

	template<bool isFluid, Storm::ColoredSetting coloredSetting = Storm::ColoredSetting::Velocity>
	void fastTransCopyImpl(const Storm::PushedParticleSystemDataParameter &param, const Storm::GraphicPipe::ColorSetting &colorSetting, std::vector<Storm::GraphicParticleData> &inOutDxParticlePosDataTmp)
	{
		DirectX::XMVECTOR defaultColor = isFluid ? getDefaultFluidParticleColor() : getRbColor(param._particleSystemId);

		const float deltaColorRChan = 1.f - defaultColor.m128_f32[0];
		const float deltaValueForColor = colorSetting._maxValue - colorSetting._minValue;

		const std::vector<Storm::Vector3> &particlePosData = *param._positionsData;

		const auto copyLambda = [&particlePosData, &param, &defaultColor, &colorSetting, deltaColorRChan, deltaValueForColor](Storm::GraphicParticleData &current, const std::size_t iter)
		{
			memcpy(&current._pos, &particlePosData[iter], sizeof(Storm::Vector3));
			reinterpret_cast<float*>(&current._pos)[3] = 1.f;

			if constexpr (isFluid)
			{
				float coeff = getColoredMonoDimensionUsedData<coloredSetting>(param, iter) - colorSetting._minValue;

				current._color = defaultColor;

				if (coeff > 0.f)
				{
					coeff = coeff / deltaValueForColor;

					if (coeff > 1.f)
					{
						current._color.m128_f32[0] = 1.f;
					}
					else
					{
						current._color.m128_f32[0] = defaultColor.m128_f32[0] + deltaColorRChan * coeff;
					}
				}
			}
			else // Rigidbody
			{
				current._color = defaultColor;
			}
		};

		const std::size_t thresholdMultiThread = std::thread::hardware_concurrency() * 1500;

		const std::size_t particleCount = particlePosData.size();
		if (particleCount > thresholdMultiThread)
		{
			Storm::runParallel(inOutDxParticlePosDataTmp, copyLambda);
		}
		else
		{
			for (std::size_t iter = 0; iter < particleCount; ++iter)
			{
				copyLambda(inOutDxParticlePosDataTmp[iter], iter);
			}
		}
	}

	std::wstring_view parseColoredSetting(const Storm::ColoredSetting setting)
	{
#define STORM_CASE_PARSE_COLORED_SETTING(ColoredSettingValue) case Storm::ColoredSetting::ColoredSettingValue: return STORM_TEXT(#ColoredSettingValue)
		switch (setting)
		{
			STORM_CASE_PARSE_COLORED_SETTING(Velocity);
			STORM_CASE_PARSE_COLORED_SETTING(Pressure);
			STORM_CASE_PARSE_COLORED_SETTING(Density);
		}
#undef STORM_CASE_PARSE_COLORED_SETTING

		Storm::throwException<Storm::Exception>("Unknown colored setting chosen.");
	}

	std::string parseFloatLeanAndMean(const float value)
	{
		std::string result = std::to_string(value);
		
		while (result.size() > 1)
		{
			const char lastChar = result.back();
			switch (lastChar)
			{
			case '0':
			case ' ':
				result.pop_back();
				break;

			case '.':
				result.pop_back();

			default:
				goto endLoop;
			}
		}

		endLoop:
		return result;
	}
}

#define STORM_COLORED_SETTING_FIELD_NAME "Coloration"
#define STORM_COLOR_VALUE_SETTING_FIELD_NAME "Color value"

Storm::GraphicPipe::GraphicPipe() :
	_selectedColoredSetting{ Storm::ColoredSetting::Velocity },
	_fields{ std::make_unique<Storm::UIFieldContainer>() },
	_chosenColorSetting{ &_velocitySetting }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SceneGraphicConfig &sceneGraphicConfig = configMgr.getSceneGraphicConfig();

	_velocitySetting._minValue = sceneGraphicConfig._velocityNormMinColor;
	_velocitySetting._maxValue = sceneGraphicConfig._velocityNormMaxColor;
	_pressureSetting._minValue = sceneGraphicConfig._pressureMinColor;
	_pressureSetting._maxValue = sceneGraphicConfig._pressureMaxColor;
	_densitySetting._minValue = sceneGraphicConfig._densityMinColor;
	_densitySetting._maxValue = sceneGraphicConfig._densityMaxColor;

	_coloredSettingWStr = parseColoredSetting(_selectedColoredSetting);

	(*_fields)
		.bindField(STORM_COLORED_SETTING_FIELD_NAME, _coloredSettingWStr)
		.bindField(STORM_COLOR_VALUE_SETTING_FIELD_NAME, _chosenColorSetting)
		;
}

Storm::GraphicPipe::~GraphicPipe() = default;

std::vector<Storm::GraphicParticleData> Storm::GraphicPipe::fastOptimizedTransCopy(const Storm::PushedParticleSystemDataParameter &param)
{
	std::vector<Storm::GraphicParticleData> dxParticlePosDataTmp;

	const bool enableDifferentColoring = param._isFluids;

#if _WIN32

	const Storm::VectorHijackerMakeBelieve hijacker{ param._positionsData->size() };
	dxParticlePosDataTmp.reserve(hijacker._newSize);

	// Huge optimization that completely destroys resize method... Cannot be much faster than this, it is like Unreal technology (TArray provides a SetNumUninitialized).
	// (Except that Unreal implemented their own TArray instead of using std::vector. Since I'm stuck with this, I didn't have much choice than to hijack... Note that this code isn't portable because it relies heavily on how Microsoft implemented std::vector (to find out the breach in the armor, we must know whose armor it is ;) )).
	Storm::setNumUninitialized_hijack(dxParticlePosDataTmp, hijacker);

	const Storm::GraphicPipe::ColorSetting &usedColorSetting = this->getUsedColorSetting();
	if (enableDifferentColoring)
	{
#define STORM_CASE_FAST_TRANSFER_WITH_COLORED_SETTING(ColoredSettingValue) case ColoredSettingValue: \
		fastTransCopyImpl<true, ColoredSettingValue>(param, usedColorSetting, dxParticlePosDataTmp); \
		break

		switch (_selectedColoredSetting)
		{
			STORM_CASE_FAST_TRANSFER_WITH_COLORED_SETTING(Storm::ColoredSetting::Velocity);
			STORM_CASE_FAST_TRANSFER_WITH_COLORED_SETTING(Storm::ColoredSetting::Pressure);
			STORM_CASE_FAST_TRANSFER_WITH_COLORED_SETTING(Storm::ColoredSetting::Density);

		case Storm::ColoredSetting::Count:
		default:
			Storm::throwException<Storm::Exception>("Unknown colored setting chosen.");
		}
#undef STORM_CASE_FAST_TRANSFER_WITH_COLORED_SETTING

	}
	else
	{
		fastTransCopyImpl<false>(param, usedColorSetting, dxParticlePosDataTmp);
	}

#else
	dxParticlePosDataTmp.reserve(hijacker._newSize);

	const std::vector<Storm::Vector3> &particlePosData = *param._positionsData;
	
	const DirectX::XMVECTOR defaultColor = enableDifferentColoring ? getDefaultParticleColor<true>() : getDefaultParticleColor<false>();
	for (const Storm::Vector3 &pos : particlePosData)
	{
		dxParticlePosDataTmp.emplace_back(pos, defaultColor);
	}

#endif

	return dxParticlePosDataTmp;
}

Storm::GraphicPipe::ColorSetting& Storm::GraphicPipe::getColorSettingFromSelection(const Storm::ColoredSetting setting) const
{
	switch (setting)
	{
	case Storm::ColoredSetting::Velocity: return const_cast<Storm::GraphicPipe::ColorSetting &>(_velocitySetting);
	case Storm::ColoredSetting::Pressure: return const_cast<Storm::GraphicPipe::ColorSetting &>(_pressureSetting);
	case Storm::ColoredSetting::Density: return const_cast<Storm::GraphicPipe::ColorSetting &>(_densitySetting);
	default:
		Storm::throwException<Storm::Exception>("Unknown colored setting chosen.");
	}
}

void Storm::GraphicPipe::cycleColoredSetting()
{
	this->setUsedColorSetting(static_cast<Storm::ColoredSetting>((static_cast<uint8_t>(_selectedColoredSetting) + 1) % static_cast<uint8_t>(Storm::ColoredSetting::Count)));
}

const Storm::GraphicPipe::ColorSetting& Storm::GraphicPipe::getUsedColorSetting() const
{
	return *_chosenColorSetting;
}

void Storm::GraphicPipe::setUsedColorSetting(const Storm::ColoredSetting setting)
{
	if (setting == Storm::ColoredSetting::Count)
	{
		Storm::throwException<Storm::Exception>("Colored Setting 'Count' is not allowed!");
	}

	if (_selectedColoredSetting != setting)
	{
		_selectedColoredSetting = setting;
		_chosenColorSetting = &this->getColorSettingFromSelection(_selectedColoredSetting);

		_coloredSettingWStr = parseColoredSetting(_selectedColoredSetting);

		_fields->pushField(STORM_COLORED_SETTING_FIELD_NAME);
		_fields->pushField(STORM_COLOR_VALUE_SETTING_FIELD_NAME);
	}
}

void Storm::GraphicPipe::setMinMaxColorationValue(float newMinValue, float newMaxValue, const Storm::ColoredSetting setting)
{
	if (newMinValue > newMaxValue)
	{
		Storm::throwException<Storm::Exception>("new coloration min value (" + std::to_string(newMinValue) + ") cannot be greater than new coloration max value (" + std::to_string(newMaxValue) + ")!");
	}

	Storm::GraphicPipe::ColorSetting &settingToChange = this->getColorSettingFromSelection(setting);

	const bool changed =
		newMinValue != settingToChange._minValue ||
		newMaxValue != settingToChange._maxValue
		;

	settingToChange._minValue = newMinValue;
	settingToChange._maxValue = newMaxValue;

	if (setting == _selectedColoredSetting && changed)
	{
		_fields->pushField(STORM_COLOR_VALUE_SETTING_FIELD_NAME);
		this->notifyCurrentGraphicPipeColorationSettingChanged();
	}
}

void Storm::GraphicPipe::setMinMaxColorationValue(float newMinValue, float newMaxValue)
{
	this->setMinMaxColorationValue(newMinValue, newMaxValue, _selectedColoredSetting);
}

void Storm::GraphicPipe::changeMinColorationValue(float deltaValue, const Storm::ColoredSetting setting)
{
	if (deltaValue != 0.f)
	{
		Storm::GraphicPipe::ColorSetting &settingToChange = this->getColorSettingFromSelection(setting);
		const float expectedValue = std::min(settingToChange._minValue + deltaValue, settingToChange._maxValue);
		if (expectedValue != settingToChange._minValue)
		{
			settingToChange._minValue = expectedValue;
			if (setting == _selectedColoredSetting)
			{
				_fields->pushField(STORM_COLOR_VALUE_SETTING_FIELD_NAME);
				this->notifyCurrentGraphicPipeColorationSettingChanged();
			}
		}
		else
		{
			LOG_DEBUG_WARNING << "Field Coloration min value change ignored because we cannot go over max value.";
		}
	}
}

void Storm::GraphicPipe::changeMaxColorationValue(float deltaValue, const Storm::ColoredSetting setting)
{
	if (deltaValue != 0.f)
	{
		Storm::GraphicPipe::ColorSetting &settingToChange = this->getColorSettingFromSelection(setting);
		const float expectedValue = std::max(settingToChange._maxValue + deltaValue, settingToChange._minValue);
		if (expectedValue != settingToChange._maxValue)
		{
			settingToChange._maxValue = expectedValue;
			if (setting == _selectedColoredSetting)
			{
				_fields->pushField(STORM_COLOR_VALUE_SETTING_FIELD_NAME);
				this->notifyCurrentGraphicPipeColorationSettingChanged();
			}
		}
		else
		{
			LOG_DEBUG_WARNING << "Field Coloration max value change ignored because we cannot go under min value.";
		}
	}
}

void Storm::GraphicPipe::changeMinColorationValue(float deltaValue)
{
	this->changeMinColorationValue(deltaValue, _selectedColoredSetting);
}

void Storm::GraphicPipe::changeMaxColorationValue(float deltaValue)
{
	this->changeMaxColorationValue(deltaValue, _selectedColoredSetting);
}

void Storm::GraphicPipe::notifyCurrentGraphicPipeColorationSettingChanged() const
{
	Storm::ISimulatorManager &simulMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISimulatorManager>();
	simulMgr.onGraphicParticleSettingsChanged();
}

Storm::GraphicPipe::ColorSetting::operator std::string() const
{
	std::string result;
	
	const std::string minStr = parseFloatLeanAndMean(_minValue);
	const std::string maxStr = parseFloatLeanAndMean(_maxValue);
	result.reserve(8 + minStr.size() + maxStr.size());

	result += "{ ";
	result += minStr;
	result += ", ";
	result += maxStr;
	result += " }";

	return result;
}
