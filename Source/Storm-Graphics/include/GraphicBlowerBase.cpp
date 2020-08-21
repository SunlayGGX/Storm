#include "GraphicBlowerBase.h"

#include "BlowerData.h"
#include "BlowerShader.h"

#include "XMStormHelpers.h"


namespace
{
	struct BlowerVertex
	{
	public:
		using VertexType = DirectX::XMVECTOR;

	public:
		BlowerVertex(const Storm::Vector3 &other) :
			_vertex{ other.x(), other.y(), other.z(), 1.f }
		{}

	public:
		VertexType _vertex;
	};

	void convertVertexesToBlowerVertexes(const std::vector<Storm::Vector3> &inTransitionBuffer, std::vector<BlowerVertex> &outFinalVertexes)
	{
		const std::size_t vertexCountToConvert = inTransitionBuffer.size();
		if (vertexCountToConvert > 0)
		{
			outFinalVertexes.reserve(outFinalVertexes.size() + vertexCountToConvert);
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


Storm::GraphicBlowerBase::GraphicBlowerBase(const Storm::BlowerData &blowerData) :
	_id{ blowerData._id }
{
	// Take the Blower position.
	// TODO: One day, have a rotation for blowers... When we'll need it.
	_blowerWorldMatrix = Storm::makeTransform(blowerData._blowerPosition, Storm::Quaternion::Identity());
}

Storm::GraphicBlowerBase::~GraphicBlowerBase() = default;

void Storm::GraphicBlowerBase::instantiateShader(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes)
{
	_indexCount = static_cast<uint32_t>(indexes.size());

	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(BlowerVertex) * static_cast<UINT>(vertexes.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertexes.data();
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

	_blowerShader = std::make_unique<Storm::BlowerShader>(device, _indexCount);
}

void Storm::GraphicBlowerBase::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	_blowerShader->setup(device, deviceContext, currentCamera, _blowerWorldMatrix);
	this->setupBlower(deviceContext);
	_blowerShader->draw(_indexCount, deviceContext);
}

void Storm::GraphicBlowerBase::setupBlower(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(BlowerVertex);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}

std::size_t Storm::GraphicBlowerBase::getId() const
{
	return _id;
}
