#pragma once


namespace Storm
{
	class NormalsShader;
	class Camera;
	class GraphicParameters;

	class GraphicNormals
	{
	public:
		struct GraphicNormalInternal
		{
		public:
			Storm::Vector3 _base;
			Storm::Vector3 _head;
		};

	public:
		GraphicNormals(const ComPtr<ID3D11Device> &device);
		~GraphicNormals();

	private:
		void refreshNormalsDataFromCachedInternal(const ComPtr<ID3D11Device> &device, const uint32_t normalCount);

	public:
		void refreshNormalsData(const ComPtr<ID3D11Device> &device, const Storm::GraphicParameters &params, const float oldNormalizationCoeff);
		void updateNormalsData(const ComPtr<ID3D11Device> &device, const Storm::GraphicParameters &params, const std::vector<Storm::Vector3> &positions, const std::vector<Storm::Vector3> &normals);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera);

	private:
		void setupForRender(const ComPtr<ID3D11DeviceContext> &deviceContext);

	private:
		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		uint32_t _lastSize;

		std::vector<Storm::GraphicNormals::GraphicNormalInternal> _cached;

		std::unique_ptr<Storm::NormalsShader> _shader;
	};
}
