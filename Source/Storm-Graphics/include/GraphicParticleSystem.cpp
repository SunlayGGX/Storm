#include "GraphicParticleSystem.h"

#include "ParticleShader.h"

#include "GraphicParticleData.h"

#include "GraphicManager.h"
#include "DirectXController.h"



Storm::GraphicParticleSystem::GraphicParticleSystem(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::ParticleShader>(device) }
{

}

Storm::GraphicParticleSystem::~GraphicParticleSystem() = default;

void Storm::GraphicParticleSystem::refreshParticleSystemData(unsigned int particleSystemId, std::vector<Storm::GraphicParticleData> &&particlePosition)
{
	auto &currentPBuffer = _particleSystemVBuffer[particleSystemId];

	const std::size_t newParticleCount = particlePosition.size();
	if (newParticleCount == 0)
	{
		return;
	}

	const bool shouldRegenIndexBuffer = currentPBuffer._indexBuffer == nullptr || currentPBuffer._vertexCount != newParticleCount;
	currentPBuffer._vertexCount = newParticleCount;

	// In case it has a vertex buffer set (most of the time)
	currentPBuffer._vertexBuffer = nullptr;

	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Storm::GraphicParticleData) * static_cast<UINT>(newParticleCount);
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = particlePosition.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	auto &device = Storm::GraphicManager::instance().getController().getDirectXDevice();

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &currentPBuffer._vertexBuffer));

	if (shouldRegenIndexBuffer)
	{
		std::unique_ptr<uint32_t[]> indexes = std::make_unique<uint32_t[]>(newParticleCount);
		for (uint32_t iter = 0; iter < newParticleCount; ++iter)
		{
			indexes[iter] = iter;
		}

		// Create Indexes data
		D3D11_BUFFER_DESC indexBufferDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * newParticleCount);
		indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		indexData.pSysMem = indexes.get();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &currentPBuffer._indexBuffer));
	}
}

void Storm::GraphicParticleSystem::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	_shader->setup(device, deviceContext, currentCamera);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (const auto &particleSystemBuffer : _particleSystemVBuffer)
	{
		this->setupForRender(deviceContext, particleSystemBuffer.second);
		_shader->draw(static_cast<unsigned int>(particleSystemBuffer.second._vertexCount), deviceContext);
	}
}

void Storm::GraphicParticleSystem::setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::GraphicParticleSystem::InternalParticleSystemBuffer &sysBufferToRender)
{
	constexpr UINT stride = sizeof(Storm::GraphicParticleData);
	constexpr UINT offset = 0;

	deviceContext->IASetIndexBuffer(sysBufferToRender._indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = sysBufferToRender._vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}
