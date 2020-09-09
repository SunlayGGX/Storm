#pragma once


namespace Storm
{
	class ConstraintShader;
	class Camera;

	class GraphicConstraintSystem
	{
	public:
		GraphicConstraintSystem(const ComPtr<ID3D11Device> &device);
		~GraphicConstraintSystem();

	public:
		void refreshConstraintsData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &constraintsData);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		uint32_t _constraintVertexCount;

		std::unique_ptr<Storm::ConstraintShader> _shader;
	};
}
