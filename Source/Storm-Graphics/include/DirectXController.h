#pragma once


namespace Storm
{
	class Camera;
	class IRenderedElement;

	class DirectXController
	{
	public:
		void initialize(HWND hwnd);
		void cleanUp();

	public:
		void clearRenderTarget(const float(&clearColor)[4]);
		void drawRenderTarget();

	public:
		const ComPtr<ID3D11Device>& getDirectXDevice() const noexcept;
		const ComPtr<ID3D11DeviceContext>& getDirectXDeviceContext() const noexcept;

	public:
		void renderElements(const Storm::Camera &currentCamera, const std::vector<std::unique_ptr<Storm::IRenderedElement>> &renderedElementArrays) const;

	private:
		void setupSwapChain(HWND hwnd);

	public:
		float getViewportWidth() const noexcept;
		float getViewportHeight() const noexcept;

	private:
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _deviceContext;
		ComPtr<ID3D11RenderTargetView> _renderTarget;
		ComPtr<IDXGISwapChain1> _swapChain;
		ComPtr<ID3D11Texture2D> _depthStencilBuffer;
		ComPtr<ID3D11DepthStencilState> _depthStencilState;
		ComPtr<ID3D11DepthStencilView> _depthStencilView;
		ComPtr<ID3D11RasterizerState> _rasterState;

		float _viewportWidth;
		float _viewportHeight;
	};
}
