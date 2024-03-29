#include "GraphicParticleSystem.h"

#include "ParticleShader.h"

#include "GraphicParticleData.h"
#include "GraphicParticleSystemModality.h"

#include "RenderModeState.h"



Storm::GraphicParticleSystem::GraphicParticleSystem(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::ParticleShader>(device) }
{

}

Storm::GraphicParticleSystem::~GraphicParticleSystem() = default;

void Storm::GraphicParticleSystem::refreshParticleSystemData(const ComPtr<ID3D11Device> &device, unsigned int particleSystemId, std::vector<Storm::GraphicParticleData> &&particlePosition, bool isFluids, bool isWall)
{
	auto &currentPBuffer = _particleSystemVBuffer[particleSystemId];

	if (isFluids)
	{
		currentPBuffer._modality = Storm::GraphicParticleSystemModality::Fluid;
	}
	else if (isWall)
	{
		currentPBuffer._modality = Storm::GraphicParticleSystemModality::RbWall;
	}
	else
	{
		currentPBuffer._modality = Storm::GraphicParticleSystemModality::RbNoWall;
	}

	const std::size_t newParticleCount = particlePosition.size();
	assert(newParticleCount != 0 && "particle count should be strictly greater than 0 when entering this method");

	const bool shouldRegenIndexBuffer = currentPBuffer._indexBuffer == nullptr || currentPBuffer._vertexCount != newParticleCount;

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

	currentPBuffer._vertexCount = newParticleCount;
}

void Storm::GraphicParticleSystem::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, Storm::RenderModeState currentRenderModeState, bool forceNoRb)
{
	_shader->setup(device, deviceContext, currentCamera, true);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (const auto &particleSystemBuffer : _particleSystemVBuffer)
	{
		bool shouldRender;
		switch (particleSystemBuffer.second._modality)
		{
		case Storm::GraphicParticleSystemModality::Fluid:
			shouldRender = currentRenderModeState != Storm::RenderModeState::SolidOnly;
			break;

		case Storm::GraphicParticleSystemModality::RbNoWall:
			if (forceNoRb)
			{
				shouldRender = false;
			}
			else
			{
				shouldRender =
					currentRenderModeState == Storm::RenderModeState::AllParticle ||
					currentRenderModeState == Storm::RenderModeState::NoWallParticles ||
					currentRenderModeState == Storm::RenderModeState::SolidOnly
					;
			}
			break;

		case Storm::GraphicParticleSystemModality::RbWall:
			shouldRender =
				currentRenderModeState == Storm::RenderModeState::AllParticle ||
				currentRenderModeState == Storm::RenderModeState::SolidOnly
				;
			break;

		default:
			assert(false && "Unknown particle system type");
			shouldRender = false;
			break;
		}

		if (shouldRender)
		{
			this->setupForRender(deviceContext, particleSystemBuffer.second);
			_shader->draw(static_cast<unsigned int>(particleSystemBuffer.second._vertexCount), deviceContext);
		}
	}
}

void Storm::GraphicParticleSystem::renderRbSecondPass(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera, Storm::RenderModeState currentRenderModeState)
{
	if (currentRenderModeState == Storm::RenderModeState::AllParticle ||
		currentRenderModeState == Storm::RenderModeState::NoWallParticles ||
		currentRenderModeState == Storm::RenderModeState::SolidOnly)
	{
		_shader->setup(device, deviceContext, currentCamera, false);

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		for (const auto &particleSystemBuffer : _particleSystemVBuffer)
		{
			switch (particleSystemBuffer.second._modality)
			{
			case Storm::GraphicParticleSystemModality::RbNoWall:
				this->setupForRender(deviceContext, particleSystemBuffer.second);
				_shader->draw(static_cast<unsigned int>(particleSystemBuffer.second._vertexCount), deviceContext);
				break;

			case Storm::GraphicParticleSystemModality::Fluid:
			case Storm::GraphicParticleSystemModality::RbWall:
				break;

			default:
				assert(false && "Unknown particle system type");
				break;
			}
		}
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
