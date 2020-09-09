#pragma once


namespace Storm
{
	class GraphicConstraintSystem
	{
	public:
		GraphicConstraintSystem(const ComPtr<ID3D11Device> &device);
		~GraphicConstraintSystem();

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		uint32_t _constraintVertexCount;

		std::unique_ptr<Storm::ConstraintShader> _shader;
	};
}
