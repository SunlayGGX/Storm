#pragma once

#include "RigidBodyHolder.h"


namespace Storm
{
	class MeshShader;
	class Camera;

	class GraphicRigidBody : public Storm::RigidBodyHolder
	{
	public:
		GraphicRigidBody(const std::vector<Storm::Vector3> &vertices, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes);

	public:
		void initializeRendering(const ComPtr<ID3D11Device> &device);

		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		unsigned int _indexesCount;

		std::unique_ptr<Storm::MeshShader> _shader;

		std::vector<Storm::Vector3> _tmpVertices;
		std::vector<unsigned int> _tmpIndexes;
	};
}
