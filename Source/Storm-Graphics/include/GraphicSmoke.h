#pragma once


namespace Storm
{
	class GraphicSmoke
	{
	public:
		GraphicSmoke(const ComPtr<ID3D11Device> &device);

	private:
		ComPtr<ID3D11Texture2D> _perlinNoiseTexture;
	};
}
