#include "ParticleForceRenderer.h"

#include "GraphicManager.h"

#include "GraphicParticleData.h"

#include "ParticleForceShader.h"

#include "XMStormHelpers.h"
#include "ThrowIfFailed.h"


Storm::ParticleForceRenderer::ParticleForceRenderer(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::ParticleForceShader>(device) }
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

		const Storm::Vector3 force[2] =
		{
			selectedParticlePosition, // Origin
			selectedParticlePosition + selectedParticleForce // End of the vector arrow
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

void Storm::ParticleForceRenderer::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	if (Storm::GraphicManager::instance().hasSelectedParticle())
	{
		_shader->setup(device, deviceContext, currentCamera);

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		this->setupForRender(deviceContext);
		_shader->draw(2, deviceContext);
	}
}

void Storm::ParticleForceRenderer::setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(Storm::Vector3);
	constexpr UINT offset = 0;

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}

