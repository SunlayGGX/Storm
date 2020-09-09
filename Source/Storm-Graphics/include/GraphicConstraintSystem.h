#pragma once


namespace Storm
{
	class GraphicConstraintSystem
	{
	public:
		GraphicConstraintSystem(const ComPtr<ID3D11Device> &device);
		~GraphicConstraintSystem();

	public:
		void refreshConstraintsData(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &constraintsData);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		uint32_t _constraintVertexCount;

		std::unique_ptr<Storm::ConstraintShader> _shader;
	};
}
