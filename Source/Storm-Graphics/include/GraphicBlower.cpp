#include "GraphicBlower.h"

#include "AreaShader.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "SceneBlowerConfig.h"
#include "SceneGraphicConfig.h"

#include "BlowerState.h"


namespace
{
	DirectX::XMVECTOR convertStateToColor(const Storm::BlowerState blowerState, const float blowerAlpha)
	{
		const float k_disabledAlpha = blowerAlpha / 2.f;
		switch (blowerState)
		{
		case Storm::BlowerState::NotWorking:		return DirectX::XMVECTOR{ 0.2f, 0.2f, 0.2f, k_disabledAlpha };
		case Storm::BlowerState::Fading:			return DirectX::XMVECTOR{ 1.f, 0.5f, 0.f, blowerAlpha };
		case Storm::BlowerState::FullyFonctional:	return DirectX::XMVECTOR{ 0.1f, 8.f, 0.2f, blowerAlpha };
		default:									return DirectX::XMVECTOR{ 0.f, 0.f, 0.f, 0.f };
		}
	}
}


Storm::GraphicBlower::GraphicBlower(const ComPtr<ID3D11Device> &device, const Storm::SceneBlowerConfig &blowerConfig, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes) :
	_id{ blowerConfig._blowerId },
	_blowerState{ Storm::BlowerState::NotWorking }
{
	Storm::GraphicAreaBaseHolder::initializeShader(device, vertexes, nullptr, indexes, {});
}

void Storm::GraphicBlower::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	const float blowerAlpha = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getSceneGraphicConfig()._blowerAlpha;
	_areaShader->setup(device, deviceContext, currentCamera, convertStateToColor(_blowerState, blowerAlpha));
	Storm::GraphicAreaBaseHolder::setup(deviceContext);
	_areaShader->draw(_indexCount, deviceContext);
}

void Storm::GraphicBlower::setBlowerState(const Storm::BlowerState newState) noexcept
{
	_blowerState = newState;
}

std::size_t Storm::GraphicBlower::getId() const noexcept
{
	return _id;
}
