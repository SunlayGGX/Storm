#pragma once


namespace Storm
{
	class DirectXController
	{
	public:
		void initialize(HWND hwnd);
		void cleanUp();

	public:
		void clearRenderTarget(const float(&clearColor)[4]);
		void drawRenderTarget();

	private:
		void setupSwapChain(HWND hwnd);

	private:
		ComPtr<ID3D11RenderTargetView> _renderTarget;
		ComPtr<IDXGISwapChain1> _swapChain;
		ComPtr<ID3D11Texture2D> _depthStencilBuffer;
		ComPtr<ID3D11DepthStencilState> _depthStencilState;
		ComPtr<ID3D11DepthStencilView> _depthStencilView;
		ComPtr<ID3D11RasterizerState> _rasterState;
		ComPtr<ID3D11DeviceContext> _deviceContext;
		ComPtr<ID3D11Device> _device;
	};
}
