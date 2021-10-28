#pragma once


namespace Storm
{
	class TextureMergerDepthShader;

	class GraphicTextureMergerDepth
	{
	public:
		GraphicTextureMergerDepth(const ComPtr<ID3D11Device> &device);
		~GraphicTextureMergerDepth();

	private:
		std::unique_ptr<Storm::TextureMergerDepthShader> _shader;
	};
}
