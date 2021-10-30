#pragma once


struct D2D_RECT_F;
using D2D1_RECT_F = D2D_RECT_F;

namespace Storm
{
	class IRenderedElement;
	struct RenderedElementProxy;
	class Camera;
	enum class RenderModeState;

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

		void reportDeviceMessages() const;

	public:
		const ComPtr<ID3D11Device>& getDirectXDevice() const noexcept;
		const ComPtr<ID3D11DeviceContext>& getImmediateContext() const noexcept;
		const ComPtr<ID2D1RenderTarget>& getUIRenderTarget() const noexcept;

	public:
		void renderElements(const Storm::Camera &currentCamera, const Storm::RenderedElementProxy &paramToRender);

	public:
		float getViewportWidth() const noexcept;
		float getViewportHeight() const noexcept;

	public:
		void setWireFrameState();
		void setSolidCullNoneState();
		void setSolidCullBackState();
		void setAllParticleState();
		void setRenderNoWallParticle();
		void setRenderNoWallSolid();
		void setRenderSolidOnly();

		void setEnableZBuffer(bool enable);
		void setEnableBlendAlpha(bool enable);

	public:
		void drawUI(const std::vector<std::unique_ptr<Storm::IRenderedElement>> &renderedElementArrays, const std::map<std::wstring_view, std::wstring> &texts);
		void notifyFieldCount(std::size_t fieldCount);
		void setTextHeightCoeff(float textHeightCoeff);

		void setUIFieldDrawEnabled(const bool enable);
		bool getUIFieldDrawEnabled() const noexcept;

	private:
		void drawTextBackground(const D2D1_RECT_F &rectPosition);
		void drawText(const D2D1_RECT_F &rectPosition);

	private:
		// 3D
		void internalCreateDXDevices(HWND hwnd);
		void internalCreateRenderView();
		void internalInitializeDepthBuffer();
		void internalConfigureStates();
		void internalInitializeDebugDevice();
		void internalConfigureImmediateContextToDefault();
		void internalInitializeViewPort();

		//2D
		void internalCreateDirect2DDevices(HWND hwnd);
		void internalCreateDirectWrite();

	public:
		float getDepthBufferAtPixel(int xPos, int yPos) const;

	private:
		// 3D
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _immediateContext;
		ComPtr<IDXGISwapChain> _swapChain;
		ComPtr<ID3D11RenderTargetView> _renderTargetView;

		ComPtr<ID3D11DepthStencilView> _depthStencilView;
		ComPtr<ID3D11Texture2D> _depthTexture;
		ComPtr<ID3D11Texture2D> _depthTextureCpuSide;

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

		Storm::RenderModeState _currentRenderModeState;

		// 2D
		ComPtr<ID2D1Factory> _direct2DFactory;
		ComPtr<ID2D1RenderTarget> _direct2DRenderTarget;

		ComPtr<ID2D1SolidColorBrush> _direct2DRectSolidBrush;

		ComPtr<ID2D1SolidColorBrush> _direct2DTextSolidBrush;
		ComPtr<IDWriteFactory> _writeFactory;
		ComPtr<IDWriteTextFormat> _textFormat;

		// Misc
		bool _zBufferStateEnabled;

		float _viewportWidth;
		float _viewportHeight;

		bool _logDeviceMessage;

		bool _uiFieldWriteInfoEnabled;

		float _textHeightCoeff;

		float _writeRectLeft;
		float _writeRectRight;
		float _writeRectHeight;

		std::wstring _writeTextTemp;

#if defined(DEBUG) || defined(_DEBUG)
		ComPtr<ID3D11Debug> _debugDevice;
#endif
	};
}
