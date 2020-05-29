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
		void clearView(const float(&clearColor)[4]);
		void unbindTargetView();
		void initView();
		void presentToDisplay();
		void reportLiveObject();

	public:
		const ComPtr<ID3D11Device>& getDirectXDevice() const noexcept;
		const ComPtr<ID3D11DeviceContext>& getImmediateContext() const noexcept;

	public:
		void renderElements(const Storm::Camera &currentCamera, const std::vector<std::unique_ptr<Storm::IRenderedElement>> &renderedElementArrays) const;

	public:
		float getViewportWidth() const noexcept;
		float getViewportHeight() const noexcept;

	public:
		void setWireFrameState();
		void setSolidCullNoneState();
		void setSolidCullBackState();

		void setEnableZBuffer(bool enable);
		void setEnableBlendAlpha(bool enable);

	private:
		void internalCreateDXDevices(HWND hwnd);
		void internalCreateRenderView();
		void internalInitializeDepthBuffer();
		void internalConfigureStates();
		void internalInitializeDebugDevice();
		void internalConfigureImmediateContextToDefault();
		void internalInitializeViewPort();

	private:
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _immediateContext;
		ComPtr<IDXGISwapChain> _swapChain;
		ComPtr<ID3D11RenderTargetView> _renderTargetView;

		ComPtr<ID3D11DepthStencilView> _depthStencilView;
		ComPtr<ID3D11Texture2D> _depthTexture;

		ComPtr<ID3D11DepthStencilState> _zBufferEnabled;
		ComPtr<ID3D11DepthStencilState> _zBufferDisabled;

		ComPtr<ID3D11RasterizerState> _rasterAlphaBlendEnabled;
		ComPtr<ID3D11RasterizerState> _rasterAlphaBlendDisabled;

		ComPtr<ID3D11RasterizerState> _solidCullBackRS;
		ComPtr<ID3D11RasterizerState> _solidCullNoneRS;
		ComPtr<ID3D11RasterizerState> _wireframeCullNoneRS;

		ComPtr<ID3D11DepthStencilState> _zBufferEnable;
		ComPtr<ID3D11DepthStencilState> _zBufferDisable;

		ComPtr<ID3D11BlendState> _alphaBlendEnable;
		ComPtr<ID3D11BlendState> _alphaBlendDisable;

		float _viewportWidth;
		float _viewportHeight;

#if defined(DEBUG) || defined(_DEBUG)
		ComPtr<ID3D11Debug> _debugDevice;
#endif
	};
}
