#include "GraphicAreaBaseHolder.h"

#include "AreaShader.h"


namespace
{
	struct AreaVertex
	{
	public:
		using VertexType = DirectX::XMVECTOR;

	public:
		AreaVertex(const Storm::Vector3 &other) :
			_vertex{ other.x(), other.y(), other.z(), 1.f }
		{}

	public:
		VertexType _vertex;
	};

	void convertVertexesToAreaVertexes(const std::vector<Storm::Vector3> &inTransitionBuffer, std::vector<AreaVertex> &outFinalVertexes)
	{
		const std::size_t vertexCountToConvert = inTransitionBuffer.size();
		if (vertexCountToConvert > 0)
		{
			outFinalVertexes.reserve(vertexCountToConvert);
			for (const Storm::Vector3 &toConvert : inTransitionBuffer)
			{
				outFinalVertexes.emplace_back(toConvert);
			}
		}
		else
		{
			assert(false && "We got 0 vertexes from a simple mesh generation, this shouldn't happen!");
		}
	}
}


Storm::GraphicAreaBaseHolder::~GraphicAreaBaseHolder() = default;

void Storm::GraphicAreaBaseHolder::initializeShader(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes)
{
	_indexCount = static_cast<uint32_t>(indexes.size());

	// Create Vertex data
	std::vector<AreaVertex> finalVertexes;
	convertVertexesToAreaVertexes(vertexes, finalVertexes);

	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(AreaVertex) * static_cast<UINT>(finalVertexes.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = finalVertexes.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

	// Create Indexes data
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;

	indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * _indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indexes.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));

	_areaShader = std::make_unique<Storm::AreaShader>(device, _indexCount);
}

void Storm::GraphicAreaBaseHolder::setup(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(AreaVertex);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}
