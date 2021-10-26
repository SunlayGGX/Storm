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


Storm::Grid::Grid(const ComPtr<ID3D11Device> &device, Storm::Vector3 maxPt, bool showGrid) :
	_visible{ showGrid }
{
	constexpr float epsilon = 0.000001f;
	maxPt.x() = static_cast<float>(fabs(static_cast<int>(ceilf(maxPt.x()) + epsilon)));
	maxPt.z() = static_cast<float>(fabs(static_cast<int>(ceilf(maxPt.z()) + epsilon)));

	if (maxPt.x() >= 1.f && maxPt.z() >= 1.f)
	{
		const Storm::Vector3 mirror{ -maxPt.x(), maxPt.y(), -maxPt.z() };

		const unsigned int xLineCount = static_cast<unsigned int>(maxPt.x() - mirror.x()) + 1;
		const unsigned int zLineCount = static_cast<unsigned int>(maxPt.z() - mirror.z()) + 1;
		const unsigned int totalLineCount = xLineCount + zLineCount;
		const uint32_t totalIndexCount = totalLineCount * 2;

		std::vector<GridVertexType> gridVertexData;
		gridVertexData.reserve(totalIndexCount);

		float currentPtCoord;

		// Fill x lines
		for (unsigned int xIndex = 0; xIndex < xLineCount; ++xIndex)
		{
			currentPtCoord = static_cast<float>(xIndex) - maxPt.x();

			gridVertexData.emplace_back(currentPtCoord, maxPt.y(), maxPt.z());
			gridVertexData.emplace_back(currentPtCoord, maxPt.y(), mirror.z());
		}

		// Fill z lines
		for (unsigned int zIndex = 0; zIndex < zLineCount; ++zIndex)
		{
			currentPtCoord = static_cast<float>(zIndex) - maxPt.z();

			gridVertexData.emplace_back(maxPt.x(), maxPt.y(), currentPtCoord);
			gridVertexData.emplace_back(mirror.x(), maxPt.y(), currentPtCoord);
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
		Storm::throwException<Storm::Exception>("maxPt should be far enough from 0,?,0 to be displayed properly");
	}
}

void Storm::Grid::render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera)
{
	if (_visible)
	{
		_gridShader->setup(device, deviceContext, currentCamera);
		this->setupGrid(deviceContext);
		_gridShader->draw(deviceContext);
	}
}

void Storm::Grid::setupGrid(const ComPtr<ID3D11DeviceContext> &deviceContext)
{
	assert(_visible && "We should have tested for visibility before entering this method!");

	constexpr UINT stride = sizeof(GridVertexType);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer*const tmpVertexBuffer = _vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &tmpVertexBuffer, &stride, &offset);
}

bool Storm::Grid::setVisibility(const bool visible)
{
	if (_visible != visible)
	{
		_visible = visible;
		return true;
	}
	return false;
}
