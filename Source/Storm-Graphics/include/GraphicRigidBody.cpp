#include "GraphicRigidBody.h"

#include "MeshShader.h"

#include "XMStormHelpers.h"
#include "IRigidBody.h"

namespace
{
	struct MeshVertexType
	{
	public:
		using PointType = DirectX::XMVECTOR;

	public:
		MeshVertexType(float x1, float y1, float z1) :
			pt{ x1, y1, z1, 1.f }
		{}

	public:
		PointType pt;
	};

	template<class ContainerType>
	void purge(ContainerType &cont)
	{
		cont.clear();
		cont.shrink_to_fit();
	}

	DirectX::XMMATRIX makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot)
	{
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixAffineTransformation(
			DirectX::FXMVECTOR{ 1.f, 1.f, 1.f, 0.f },
			DirectX::FXMVECTOR{ 0.f, 0.f, 0.f, 1.f },
			DirectX::XMVECTOR{ rot.x(), rot.y(), rot.z(), rot.w() },
			Storm::convertToXM(trans)
		));
	}
}


Storm::GraphicRigidBody::GraphicRigidBody(const std::vector<Storm::Vector3> &vertices, const std::vector<Storm::Vector3> &/*normals*/, const std::vector<unsigned int> &indexes) :
	_indexesCount{ static_cast<unsigned int>(indexes.size()) },
	_tmpVertices{ vertices },
	_tmpIndexes{ indexes }
{

}

void Storm::GraphicRigidBody::initializeRendering(const ComPtr<ID3D11Device> &device)
{
	std::vector<MeshVertexType> meshVertexData;
	meshVertexData.reserve(_tmpVertices.size());

	for (const auto &vertex : _tmpVertices)
	{
		meshVertexData.emplace_back(vertex.x(), vertex.y(), vertex.z());
	}

	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(MeshVertexType) * static_cast<UINT>(meshVertexData.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = meshVertexData.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

	// Create Indexes data
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;

	indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * _indexesCount;
	indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = _tmpIndexes.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));

	// We don't need them anymore. So scrap them.
	purge(_tmpVertices);
	purge(_tmpIndexes);

	_shader = std::make_unique<Storm::MeshShader>(device);
}

void Storm::GraphicRigidBody::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	Storm::Vector3 trans;
	Storm::Quaternion rot;

	const std::shared_ptr<Storm::IRigidBody> &boundRbParent = this->getRbParent();
	boundRbParent->getRigidBodyTransform(trans, rot);

	DirectX::XMMATRIX worldTransform = makeTransform(trans, rot);

	_shader->setup(device, deviceContext, currentCamera, worldTransform);
	this->setupForRender(deviceContext);
	_shader->draw(_indexesCount, deviceContext);
}

void Storm::GraphicRigidBody::setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(MeshVertexType);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}
