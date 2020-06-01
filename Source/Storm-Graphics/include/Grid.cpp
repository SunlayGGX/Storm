#include "ThrowException.h"
#include "Grid.h"
#include "GridShader.h"


namespace
{
	struct GridVertexType
	{
	public:
		using PointType = DirectX::XMVECTOR;

	public:
		GridVertexType(float x1, float y1, float z1) :
			pt{ x1, y1, z1, 1.f }
		{}

	public:
		PointType pt;
	};
}


Storm::Grid::Grid(const ComPtr<ID3D11Device> &device, Storm::Vector3 maxPt)
{
	maxPt._x = static_cast<float>(fabs(static_cast<int>(maxPt._x)));
	maxPt._z = static_cast<float>(fabs(static_cast<int>(maxPt._z)));

	if (maxPt._x >= 1.f && maxPt._z >= 1.f)
	{
		const Storm::Vector3 mirror{ -maxPt._x, maxPt._y, -maxPt._z };

		const unsigned int xLineCount = static_cast<unsigned int>(maxPt._x - mirror._x + 1);
		const unsigned int zLineCount = static_cast<unsigned int>(maxPt._z - mirror._z + 1);
		const unsigned int totalLineCount = xLineCount + zLineCount;
		const uint32_t totalIndexCount = totalLineCount * 2;

		std::vector<GridVertexType> gridVertexData;
		gridVertexData.reserve(totalIndexCount);

		float currentPtCoord;

		// Fill x lines
		for (unsigned int xIndex = 0; xIndex < xLineCount; ++xIndex)
		{
			currentPtCoord = static_cast<float>(xIndex) - maxPt._x;

			gridVertexData.emplace_back(currentPtCoord, maxPt._y, maxPt._z);
			gridVertexData.emplace_back(currentPtCoord, maxPt._y, mirror._z);
		}

		// Fill z lines
		for (unsigned int zIndex = 0; zIndex < zLineCount; ++zIndex)
		{
			currentPtCoord = static_cast<float>(zIndex) - maxPt._z;

			gridVertexData.emplace_back(maxPt._x, maxPt._y, currentPtCoord);
			gridVertexData.emplace_back(mirror._x, maxPt._y, currentPtCoord);
		}

		assert(gridVertexData.size() == totalIndexCount && "Line count mismatch what was expected!");
		std::unique_ptr<uint32_t[]> gridIndexData = std::make_unique<uint32_t[]>(totalIndexCount);
		for (uint32_t index = 0; index < totalIndexCount; ++index)
		{
			gridIndexData[index] = index;
		}

		// Create Vertex data
		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(GridVertexType) * static_cast<UINT>(gridVertexData.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = gridVertexData.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		Storm::throwIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

		// Create Indexes data
		D3D11_BUFFER_DESC indexBufferDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		indexData.pSysMem = gridIndexData.get();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		Storm::throwIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));

		_gridShader = std::make_unique<Storm::GridShader>(device, totalIndexCount);
	}
	else
	{
		Storm::throwException<std::exception>("maxPt should be far enough from 0,?,0 to be displayed properly");
	}
}

void Storm::Grid::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	_gridShader->setup(device, deviceContext, currentCamera);
	this->drawGrid(deviceContext);
	_gridShader->render(device, deviceContext, currentCamera);
}

void Storm::Grid::drawGrid(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	constexpr UINT stride = sizeof(GridVertexType);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}