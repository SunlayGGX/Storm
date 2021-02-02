#include "GraphicKernelEffectArea.h"

#include "AreaShader.h"

#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"

#include "GraphicParticleData.h"


namespace
{
	constexpr DirectX::XMVECTOR getKernelEffectAreaColor()
	{
		return DirectX::XMVECTOR{ 0.8f, 0.4f, 0.f, 0.35f };
	}
}


Storm::GraphicKernelEffectArea::GraphicKernelEffectArea(const ComPtr<ID3D11Device> &device) :
	_enabled{ false },
	_hasParticleHook{ false }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IAssetLoaderManager &assetMgr = singletonHolder.getSingleton<Storm::IAssetLoaderManager>();

	std::vector<Storm::Vector3> sphereVertexes;
	std::vector<Storm::Vector3> sphereNormals;
	std::vector<uint32_t> sphereIndexes;
	assetMgr.generateSimpleSphere(Storm::Vector3::Zero(), 1.f, sphereVertexes, sphereIndexes, &sphereNormals);

	constexpr std::string_view macros[] = { "STORM_HIGHLIGHT_BORDER" };
	Storm::GraphicKernelEffectArea::initializeShader(device, sphereVertexes, &sphereNormals, sphereIndexes, macros);
}

void Storm::GraphicKernelEffectArea::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	if (_hasParticleHook && _enabled)
	{
		_areaShader->setup(device, deviceContext, currentCamera, getKernelEffectAreaColor());
		Storm::GraphicAreaBaseHolder::setup(deviceContext);
		_areaShader->draw(_indexCount, deviceContext);
	}
}

void Storm::GraphicKernelEffectArea::setAreaRadius(const float radius)
{
	if (_kernelRadius != radius)
	{
		_kernelRadius = radius;
		_areaShader->updateWorldMat(_currentAreaPosition, Storm::Quaternion::Identity(), Storm::Vector3{ _kernelRadius, _kernelRadius, _kernelRadius });
	}
}

void Storm::GraphicKernelEffectArea::setAreaPosition(const Storm::GraphicParticleData &selectedParticleData)
{
	if (!DirectX::XMVector3Equal(_currentAreaPosition, selectedParticleData._pos))
	{
		_currentAreaPosition = selectedParticleData._pos;
		_areaShader->updateWorldMat(_currentAreaPosition, Storm::Quaternion::Identity(), Storm::Vector3{ _kernelRadius, _kernelRadius, _kernelRadius });
	}
}

void Storm::GraphicKernelEffectArea::setHasParticleHook(bool hasHook) noexcept
{
	_hasParticleHook = hasHook;
}

void Storm::GraphicKernelEffectArea::tweakEnabled() noexcept
{
	_enabled = !_enabled;
}
