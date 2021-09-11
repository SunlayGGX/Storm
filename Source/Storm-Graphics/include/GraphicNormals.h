#pragma once


namespace Storm
{
	class NormalsShader;
	class Camera;

	class GraphicNormals
	{
	public:
		GraphicNormals(const ComPtr<ID3D11Device> &device);
		~GraphicNormals();

	public:
		void refreshNormalsData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &positions, const std::vector<Storm::Vector3> &normals);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		uint32_t _lastSize;

		std::unique_ptr<Storm::NormalsShader> _shader;
	};
}
