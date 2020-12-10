#include "GraphicPipe.h"

#include "GraphicParticleData.h"

#include "PushedParticleSystemData.h"
#include "ColoredSetting.h"

#include "RunnerHelper.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GraphicData.h"

#if _WIN32
#	define STORM_HIJACKED_TYPE Storm::GraphicParticleData
#	include "VectHijack.h"
#	undef STORM_HIJACKED_TYPE
#endif

namespace
{
	template<bool isFluids>
	constexpr DirectX::XMVECTOR getDefaultParticleColor()
	{
		if constexpr (isFluids)
		{
			return { 0.3f, 0.5f, 0.85f, 1.f };
		}
		else
		{
			return { 0.3f, 0.5f, 0.5f, 1.f };
		}
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

	template<bool enableDifferentColoring, Storm::ColoredSetting coloredSetting = Storm::ColoredSetting::Velocity>
	void fastTransCopyImpl(const Storm::PushedParticleSystemDataParameter &param, const Storm::GraphicPipe::ColorSetting &colorSetting, std::vector<Storm::GraphicParticleData> &inOutDxParticlePosDataTmp)
	{
		const DirectX::XMVECTOR defaultColor = getDefaultParticleColor<enableDifferentColoring>();
		const float deltaColorRChan = 1.f - defaultColor.m128_f32[0];
		const float deltaValueForColor = colorSetting._maxValue - colorSetting._minValue;

		const std::vector<Storm::Vector3> &particlePosData = *param._positionsData;

		const auto copyLambda = [&particlePosData, &param, &defaultColor, &colorSetting, deltaColorRChan, deltaValueForColor](Storm::GraphicParticleData &current, const std::size_t iter)
		{
			memcpy(&current._pos, &particlePosData[iter], sizeof(Storm::Vector3));
			reinterpret_cast<float*>(&current._pos)[3] = 1.f;

			if constexpr (enableDifferentColoring)
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
			else
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
}


Storm::GraphicPipe::GraphicPipe() :
	_selectedColoredSetting{ Storm::ColoredSetting::Velocity }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::GraphicData &graphicDataConfig = configMgr.getGraphicData();

	_velocitySetting._minValue = graphicDataConfig._velocityNormMinColor;
	_velocitySetting._maxValue = graphicDataConfig._velocityNormMaxColor;
	_pressureSetting._minValue = graphicDataConfig._pressureMinColor;
	_pressureSetting._maxValue = graphicDataConfig._pressureMaxColor;
	_densitySetting._minValue = graphicDataConfig._densityMinColor;
	_densitySetting._maxValue = graphicDataConfig._densityMaxColor;
}

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
			Storm::throwException<std::exception>("Unknown colored setting chosen.");
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

void Storm::GraphicPipe::cycleColoredSetting()
{
	_selectedColoredSetting = static_cast<Storm::ColoredSetting>((static_cast<uint8_t>(_selectedColoredSetting) + 1) % static_cast<uint8_t>(Storm::ColoredSetting::Count));
}

const Storm::GraphicPipe::ColorSetting& Storm::GraphicPipe::getUsedColorSetting() const
{
	switch (_selectedColoredSetting)
	{
	case Storm::ColoredSetting::Velocity: return _velocitySetting;
	case Storm::ColoredSetting::Pressure: return _pressureSetting;
	case Storm::ColoredSetting::Density: return _densitySetting;
	}

	Storm::throwException<std::exception>("Unknown colored setting chosen.");
}

