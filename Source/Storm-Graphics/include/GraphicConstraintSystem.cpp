#include "GraphicConstraintSystem.h"

#include "ConstraintShader.h"

#include "ThrowIfFailed.h"


Storm::GraphicConstraintSystem::GraphicConstraintSystem(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::ConstraintShader>(device) },
	_constraintVertexCount{ 0 }
{

}

Storm::GraphicConstraintSystem::~GraphicConstraintSystem() = default;

void Storm::GraphicConstraintSystem::refreshConstraintsData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &constraintsData)
{
	const std::size_t newParticleCount = constraintsData.size();
	if (newParticleCount == 0)
	{
		return;
	}

	const bool shouldRegenIndexBuffer = static_cast<std::size_t>(_constraintVertexCount) != newParticleCount;
	_constraintVertexCount = static_cast<uint32_t>(newParticleCount);

	// In case it has a vertex buffer set (most of the time)
	_vertexBuffer = nullptr;

	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Storm::Vector3) * static_cast<UINT>(newParticleCount);
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = constraintsData.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

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

		Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));
	}
}

