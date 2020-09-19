#include "ParticleForceRenderer.h"

#include "GraphicParticleData.h"

#include "XMStormHelpers.h"
#include "ThrowIfFailed.h"


Storm::ParticleForceRenderer::ParticleForceRenderer(const ComPtr<ID3D11Device> &device)
{

}

Storm::ParticleForceRenderer::~ParticleForceRenderer() = default;

bool Storm::ParticleForceRenderer::prepareData(unsigned int particleSystemId, std::vector<Storm::GraphicParticleData> &particlePosition, const std::pair<unsigned int, std::size_t> &selectedParticle)
{
	std::size_t particleCount = particlePosition.size();
	if (particleCount > 0)
	{
		if (particleSystemId == selectedParticle.first)
		{
			Storm::GraphicParticleData &particle = particlePosition[selectedParticle.second];

			Storm::GraphicParticleData::ColorType &selectedParticleColor = particle._color;
			selectedParticleColor.m128_f32[0] = 255.f;
			selectedParticleColor.m128_f32[1] = 255.f;
			selectedParticleColor.m128_f32[2] = 255.f;
		}

		return true;
	}

	return false;
}

void Storm::ParticleForceRenderer::refreshForceData(const ComPtr<ID3D11Device> &device, const Storm::Vector3 &selectedParticlePosition, const Storm::Vector3 &selectedParticleForce)
{
	if (_lastParticlePositionCached != selectedParticlePosition || _lastParticleForceCached != selectedParticleForce || _vertexBuffer == nullptr)
	{
		_lastParticlePositionCached = selectedParticlePosition;
		_lastParticleForceCached = selectedParticleForce;

		const DirectX::XMVECTOR force[2] =
		{
			Storm::convertToXM(selectedParticlePosition), // Origin
			Storm::convertToXM(selectedParticlePosition + selectedParticleForce) // End of the vector arrow
		};

		// In case it has a vertex buffer set (most of the time)
		_vertexBuffer.Reset();

		// Create Vertex data
		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(force);
		vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = force;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

		if (_indexBuffer == nullptr)
		{
			constexpr const uint32_t indexes[] = { 0, 1 };

			// Create Indexes data
			D3D11_BUFFER_DESC indexBufferDesc;
			D3D11_SUBRESOURCE_DATA indexData;

			indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = sizeof(indexes);
			indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;
			indexBufferDesc.StructureByteStride = 0;

			indexData.pSysMem = indexes;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;

			Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));
		}
	}
}

