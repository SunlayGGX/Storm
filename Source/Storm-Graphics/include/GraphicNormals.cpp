#include "GraphicNormals.h"

#include "NormalsShader.h"

#define STORM_HIJACKED_TYPE uint32_t
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#include "XMStormHelpers.h"


namespace
{
	struct GraphicNormal
	{
	public:
		Storm::Vector3 _base;
		Storm::Vector3 _head;
	};
}

#define STORM_HIJACKED_TYPE GraphicNormal
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


Storm::GraphicNormals::GraphicNormals(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::NormalsShader>(device) },
	_lastSize{ 0 }
{

}

Storm::GraphicNormals::~GraphicNormals() = default;

void Storm::GraphicNormals::refreshNormalsData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &positions, const std::vector<Storm::Vector3> &normals)
{
	const uint32_t normalCount = static_cast<uint32_t>(normals.size());

	std::vector<GraphicNormal> normalsChanged;
	Storm::setNumUninitialized_safeHijack(normalsChanged, Storm::VectorHijacker{ normalCount });

	for (uint32_t iter = 0; iter < normalCount; ++iter)
	{
		const Storm::Vector3 &currentPos = positions[iter];
		const Storm::Vector3 &currentNormal = normals[iter];

		GraphicNormal &graphicNormal = normalsChanged[iter];
		graphicNormal._base = currentPos;
		graphicNormal._head = currentPos + (currentNormal * 0.01f);
	}

	// In case it has a vertex buffer set (most of the time)
	_vertexBuffer.Reset();

	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(GraphicNormal) * normalCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = normalsChanged.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

	uint32_t indexCount = normalCount * 2;
	if (_indexBuffer == nullptr || _lastSize != indexCount)
	{
		std::vector<uint32_t> indexes;
		Storm::setNumUninitialized_safeHijack(indexes, Storm::VectorHijacker{ indexCount });

		for(uint32_t iter = 0; iter < indexCount; ++iter)
		{
			indexes[iter] = iter;
		}
		
		// Create Indexes data
		D3D11_BUFFER_DESC indexBufferDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(uint32_t) * indexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		indexData.pSysMem = indexes.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));

		_lastSize = indexCount;
	}
}

void Storm::GraphicNormals::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	_shader->setup(device, deviceContext, currentCamera);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	this->setupForRender(deviceContext);
	_shader->draw(_lastSize, deviceContext);
}

void Storm::GraphicNormals::setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(Storm::Vector3);
	constexpr UINT offset = 0;

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}
