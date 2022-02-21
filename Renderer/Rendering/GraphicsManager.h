#pragma once

#include "DataTypes.h"
#include "GraphicsTypes.h"

namespace renderer
{
	/** Class used to initialize DirectX11 and manage its device and context */
	class GraphicsManager
	{
	public:
		/** Initializes DirectX 11 and the graphics manager */
		static GraphicsManager* Initialize(HWND windowHandle, const GraphicsConfig& config);
		~GraphicsManager();
		void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology);
		void SetViewports(int numViewPorts, const D3D11_VIEWPORT* viewports);
		D3D11_VIEWPORT* GetViewport(int slot);
		void EnableClockwiseCulling();
		void EnableCounterClockwiseCulling();
		void EnableWireframeRendering();
		void DisableCulling();
		void EnableAplhaBlending();
		void EnableColourBlending(float redFactor, float greenFactor, float blueFactor, float alphaFactor);
		void DisableBlending();
		void EnableFullDepth();
		void UseDefaultDpethStencilState();
		ID3D11Buffer* CreateBuffer(const D3D11_BUFFER_DESC& bufferDesc, const void* data = nullptr);
		void UpdateBuffer(ID3D11Buffer* buffer, const void* dataSrc);

	private:
		friend class Renderer;
		friend class MeshRenderer;

		GraphicsManager(HWND windowHandle, const GraphicsConfig& config);
		void InitializeD3D();
		void CreateViewPort(float screenWidth, float screenHeight);
		void CreateBlendStates();
		void CreateRenderStates();
		void CreateDepthStencilStates();

		// Handle to the window and config passed by the application
		HWND mWindowHandle;
		GraphicsConfig mConfig;

		// Core graphics resources
		ID3D11DepthStencilView* mDepthStencilView;
		ID3D11RenderTargetView* mRenderTargetView;
		IDXGISwapChain1* mSwapChain;
		ID3D11Device1* mDevice;
		ID3D11DeviceContext1* mDeviceContext;
		ID3D11Texture2D* mDepthStencilBuffer;

		std::vector<D3D11_VIEWPORT> mViewports;
		ID3D11BlendState* mAlphaBlend;
		ID3D11BlendState* mColourBlend;
		ID3D11RasterizerState* mCwCull;
		ID3D11RasterizerState* mCcwCull;
		ID3D11RasterizerState* mNoCull;
		ID3D11RasterizerState* mWireframe;
		ID3D11DepthStencilState* mFullDepth;

		static GraphicsManager* mGraphicsManager;
	};
}