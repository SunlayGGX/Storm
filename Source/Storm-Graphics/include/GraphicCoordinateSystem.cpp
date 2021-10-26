#include "GraphicCoordinateSystem.h"

#include "CoordinateSystemShader.h"


namespace
{
	struct CoordinateVectorType
	{
	public:
		using PointType = DirectX::XMVECTOR;
		using ColorType = DirectX::XMVECTOR;

	public:
		constexpr CoordinateVectorType(float x1, float y1, float z1, float r, float g, float b) :
			_axisVect{ x1, y1, z1, 0.f },
			_color{ r, g, b, 0.9f }
		{}

	public:
		PointType _axisVect;
		ColorType _color;
	};

	template<std::size_t indexCount>
	struct CoordinateSystemIndexBuffer
	{
	public:
		constexpr CoordinateSystemIndexBuffer()
		{
			for (std::size_t iter = 0; iter < indexCount; ++iter)
			{
				_indexes[iter] = static_cast<uint32_t>(iter);
			}
		}

	public:
		uint32_t _indexes[indexCount];
	};
}


Storm::GraphicCoordinateSystem::GraphicCoordinateSystem(const ComPtr<ID3D11Device> &device, const bool visible) :
	_shouldShow{ visible }
{
	enum : std::size_t
	{
		k_vectorCount = 3,
		k_indexCount = k_vectorCount
	};

	// Index and vertex CPU buffers
	constexpr const CoordinateVectorType k_coordinateSystemVertexes[k_vectorCount] =
	{
		CoordinateVectorType{ 1.f, 0.f, 0.f, 1.f, 0.f, 0.f },
		CoordinateVectorType{ 0.f, 1.f, 0.f, 0.f, 1.f, 0.f },
		CoordinateVectorType{ 0.f, 0.f, 1.f, 0.f, 0.f, 1.f }
	};

	constexpr const CoordinateSystemIndexBuffer<k_indexCount> k_coordinateSystemIndexes;

	// Create Vertex data
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(k_coordinateSystemVertexes);
	vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = k_coordinateSystemVertexes;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));


	// Create Indexes data
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;

	indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(k_coordinateSystemIndexes);
	indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = k_coordinateSystemIndexes._indexes;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));

	_coordinateSysShader = std::make_unique<Storm::CoordinateSystemShader>(device, k_indexCount);
}

void Storm::GraphicCoordinateSystem::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	if (_shouldShow)
	{
		_coordinateSysShader->setup(device, deviceContext, currentCamera);
		this->setup(deviceContext);
		_coordinateSysShader->draw(deviceContext);
	}
}

void Storm::GraphicCoordinateSystem::setup(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(CoordinateVectorType);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}

void Storm::GraphicCoordinateSystem::show(bool shouldShow)
{
	_shouldShow = shouldShow;
}

void Storm::GraphicCoordinateSystem::switchShow()
{
	this->show(!_shouldShow);
}
