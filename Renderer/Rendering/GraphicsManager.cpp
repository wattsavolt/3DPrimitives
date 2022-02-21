#include "GraphicsManager.h"

namespace renderer
{
	GraphicsManager* GraphicsManager::mGraphicsManager = nullptr;

	GraphicsManager* GraphicsManager::Initialize(HWND windowHandle, const GraphicsConfig& config)
	{
		if (!mGraphicsManager)
		{
			mGraphicsManager = new GraphicsManager(windowHandle, config);
		}
		return mGraphicsManager;
	}

	GraphicsManager::GraphicsManager(HWND windowHandle, const GraphicsConfig& config)
		: mWindowHandle(windowHandle), mConfig(config)
	{
		InitializeD3D();
		CreateBlendStates();
		CreateRenderStates();
		CreateDepthStencilStates();
	}

	GraphicsManager::~GraphicsManager()
	{
		mSwapChain->SetFullscreenState(false, NULL);

		SAFE_RELEASE(mAlphaBlend);
		SAFE_RELEASE(mColourBlend);
		SAFE_RELEASE(mCwCull);
		SAFE_RELEASE(mCcwCull);
		SAFE_RELEASE(mNoCull);
		SAFE_RELEASE(mWireframe);
		SAFE_RELEASE(mFullDepth);

		SAFE_RELEASE(mDepthStencilView);
		SAFE_RELEASE(mDepthStencilBuffer);
		SAFE_RELEASE(mRenderTargetView);
		SAFE_RELEASE(mDeviceContext);
		SAFE_RELEASE(mSwapChain);
		SAFE_RELEASE(mDevice);

		mGraphicsManager = nullptr;
	}

	void GraphicsManager::InitializeD3D()
	{
		ID3D11Device* tempDevice = nullptr;
		ID3D11DeviceContext* tempDevCon = nullptr;

		D3D_FEATURE_LEVEL dxFeatureLevel = D3D_FEATURE_LEVEL_11_1;

		HRESULT hr;

		//Create the Direct3D 11 Device and SwapChain
		if (FAILED(hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &tempDevice, &dxFeatureLevel, &tempDevCon)))
		{
			MessageBox(NULL, L"D3D11CreateDevice() failed.",
				L"Error", MB_OK | MB_ICONERROR);
			return;
		}
		if (FAILED(hr = tempDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&mDevice))))
		{
			SAFE_RELEASE(tempDevice);
			SAFE_RELEASE(tempDevCon);
			MessageBox(NULL, L"ID3D11Device::QueryInterface() failed.",
				L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		if (FAILED(hr = tempDevCon->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&mDeviceContext))))
		{
			SAFE_RELEASE(tempDevice);
			SAFE_RELEASE(tempDevCon);
			MessageBox(NULL, L"ID3D11DeviceContect::QueryInterface() failed.",
				L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		SAFE_RELEASE(tempDevice);
		SAFE_RELEASE(tempDevCon);

		IDXGIDevice* dxgiDevice = nullptr;
		if (FAILED(hr = mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice))))
		{
			MessageBox(NULL, L"ID3D11Device::QueryInterface() failed.",
				L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		IDXGIAdapter* dxgiAdapter = nullptr;
		if (FAILED(hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter))))
		{
			SAFE_RELEASE(dxgiDevice);
			MessageBox(NULL, L"IDXGIDevice::GetParent() failed retrieving adapter.",
				L"Error", MB_OK | MB_ICONERROR);
			return;

		}

		IDXGIFactory2* dxgiFactory = nullptr;
		if (FAILED(hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory))))
		{
			SAFE_RELEASE(dxgiDevice);
			SAFE_RELEASE(dxgiAdapter);
			MessageBox(NULL, L"IDXGIAdapter::GetParent() failed retrieving factory.",
				L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		//Create swapchain description
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
		swapChainDesc.Width = mConfig.screenWidth;
		swapChainDesc.Height = mConfig.screenHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		//Multisampling quality level
		UINT qualityLevel;
		if (mConfig.multiSamplingenabled)
		{
			mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, mConfig.multiSamplingCount, &qualityLevel);
			if (qualityLevel == 0)
			{
				SAFE_RELEASE(dxgiDevice);
				SAFE_RELEASE(dxgiAdapter);
				SAFE_RELEASE(dxgiFactory);

				MessageBox(NULL, L"Unsupported multisampling quality level.",
					L"Error", MB_OK | MB_ICONERROR);
				return;
			}
			swapChainDesc.SampleDesc.Count = mConfig.multiSamplingCount;
			swapChainDesc.SampleDesc.Quality = qualityLevel - 1;
		}
		else
		{
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
		}

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
		ZeroMemory(&fullScreenDesc, sizeof(fullScreenDesc));
		fullScreenDesc.RefreshRate.Denominator = 1;
		fullScreenDesc.RefreshRate.Numerator = mConfig.refreshRate;
		fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullScreenDesc.Windowed = mConfig.windowed;

		if (FAILED(hr = dxgiFactory->CreateSwapChainForHwnd(dxgiDevice, mWindowHandle, &swapChainDesc, &fullScreenDesc, NULL, &mSwapChain)))
		{
			SAFE_RELEASE(dxgiDevice);
			SAFE_RELEASE(dxgiAdapter);
			SAFE_RELEASE(dxgiFactory);
			MessageBox(NULL, L"IDXGIDevice::CreateSwapChainForHwnd() failed.",
				L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		// Release temporary resources
		SAFE_RELEASE(dxgiDevice);
		SAFE_RELEASE(dxgiAdapter);
		SAFE_RELEASE(dxgiFactory);

		ID3D11Texture2D* backBuffer;
		//Create back buffer and render target
		hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		hr = mDevice->CreateRenderTargetView(backBuffer, NULL, &mRenderTargetView);

		SAFE_RELEASE(backBuffer);

		//Describe the Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC depthStencilDesc;

		depthStencilDesc.Width = mConfig.screenWidth;
		depthStencilDesc.Height = mConfig.screenHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //24 bits for the depth and 8 bits for the stencil.
		depthStencilDesc.SampleDesc.Count = swapChainDesc.SampleDesc.Count;
		depthStencilDesc.SampleDesc.Quality = swapChainDesc.SampleDesc.Quality;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;
		mDevice->CreateTexture2D(&depthStencilDesc, NULL, &mDepthStencilBuffer);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		mDevice->CreateDepthStencilView(mDepthStencilBuffer, &depthStencilViewDesc, &mDepthStencilView);

		CreateViewPort(mConfig.screenWidth, mConfig.screenHeight);
		SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
	}

	void GraphicsManager::CreateViewPort(float screenWidth, float screenHeight)
	{
		//Create the Viewport
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = screenWidth;
		viewport.Height = screenHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//Set the Viewport
		SetViewports(1, &viewport);
	}

	void GraphicsManager::SetViewports(int numViewPorts, const D3D11_VIEWPORT* mViewports)
	{
		this->mViewports.clear();
		for (int i = 0; i < numViewPorts; i++) {
			const D3D11_VIEWPORT viewport = mViewports[i];
			this->mViewports.push_back(viewport);
		}

		mDeviceContext->RSSetViewports(numViewPorts, mViewports);
	}

	D3D11_VIEWPORT* GraphicsManager::GetViewport(int slot)
	{
		return &this->mViewports.at(slot);
	}

	void GraphicsManager::CreateBlendStates()
	{
		D3D11_BLEND_DESC blendDesc;

		//Alpha blend state
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = true;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		ID3D11BlendState* blendState;
		mDevice->CreateBlendState(&blendDesc, &mAlphaBlend);

		//Colour blend state
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = true;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		mDevice->CreateBlendState(&blendDesc, &mColourBlend);
	}

	void GraphicsManager::CreateRenderStates()
	{
		D3D11_RASTERIZER_DESC rastDesc;

		//Clock-wise culling
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.FrontCounterClockwise = FALSE;
		rastDesc.DepthBias = 0;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.SlopeScaledDepthBias = 0.0f;
		rastDesc.DepthClipEnable = TRUE;
		rastDesc.ScissorEnable = FALSE;
		rastDesc.MultisampleEnable = TRUE;
		rastDesc.AntialiasedLineEnable = TRUE;
		mDevice->CreateRasterizerState(&rastDesc, &mCwCull);

		//Counter-clockwise culling
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.FrontCounterClockwise = TRUE;
		rastDesc.DepthBias = 0;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.SlopeScaledDepthBias = 0.0f;
		rastDesc.DepthClipEnable = TRUE;
		rastDesc.ScissorEnable = FALSE;
		rastDesc.MultisampleEnable = TRUE;
		rastDesc.AntialiasedLineEnable = TRUE;
		mDevice->CreateRasterizerState(&rastDesc, &mCcwCull);

		//No culling
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FrontCounterClockwise = FALSE;
		rastDesc.DepthBias = 0;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.SlopeScaledDepthBias = 0.0f;
		rastDesc.DepthClipEnable = TRUE;
		rastDesc.ScissorEnable = FALSE;
		rastDesc.MultisampleEnable = TRUE;
		rastDesc.AntialiasedLineEnable = TRUE;
		mDevice->CreateRasterizerState(&rastDesc, &mNoCull);

		//Wireframe
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FrontCounterClockwise = FALSE;
		rastDesc.DepthBias = 0;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.SlopeScaledDepthBias = 0.0f;
		rastDesc.DepthClipEnable = TRUE;
		rastDesc.ScissorEnable = FALSE;
		rastDesc.MultisampleEnable = TRUE;
		rastDesc.AntialiasedLineEnable = TRUE;
		mDevice->CreateRasterizerState(&rastDesc, &mWireframe);
	}

	void GraphicsManager::CreateDepthStencilStates()
	{
		D3D11_DEPTH_STENCIL_DESC dssDesc;

		//Create depth stencil state used for sky.
		ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dssDesc.DepthEnable = true;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dssDesc.StencilEnable = FALSE;
		dssDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		dssDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

		mDevice->CreateDepthStencilState(&dssDesc, &mFullDepth);
	}

	void GraphicsManager::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology)
	{
		mDeviceContext->IASetPrimitiveTopology(topology);
	}

	//Cull counterclockwise polygons.
	void GraphicsManager::EnableClockwiseCulling()
	{
		mDeviceContext->RSSetState(mCwCull);
	}

	//Cull clockwise polygons.
	void GraphicsManager::EnableCounterClockwiseCulling()
	{
		mDeviceContext->RSSetState(mCcwCull);
	}

	void GraphicsManager::DisableCulling()
	{
		mDeviceContext->RSSetState(mNoCull);
	}

	void GraphicsManager::EnableWireframeRendering()
	{
		mDeviceContext->RSSetState(mWireframe);
	}

	void GraphicsManager::EnableAplhaBlending()
	{
		mDeviceContext->OMSetBlendState(mAlphaBlend, 0, 0xffffffff);
	}

	void GraphicsManager::EnableColourBlending(float redFactor, float greenFactor, float blueFactor, float alphaFactor)
	{
		float blendFactor[] = { redFactor, greenFactor, blueFactor, alphaFactor };
		mDeviceContext->OMSetBlendState(mColourBlend, blendFactor, 0xffffffff);
	}

	void GraphicsManager::DisableBlending()
	{
		mDeviceContext->OMSetBlendState(0, 0, 0xffffffff);
	}

	void GraphicsManager::EnableFullDepth()
	{
		mDeviceContext->OMSetDepthStencilState(mFullDepth, 0);
	}

	void GraphicsManager::UseDefaultDpethStencilState()
	{
		mDeviceContext->OMSetDepthStencilState(NULL, 0);
	}

	// Creates a graphics buffer
	ID3D11Buffer* GraphicsManager::CreateBuffer(const D3D11_BUFFER_DESC& bufferDesc, const void* data)
	{
		//Create buffer using description and the address of the above buffer declaration.
		ID3D11Buffer* newBuffer;
		HRESULT hr;
		if (data)
		{
			D3D11_SUBRESOURCE_DATA pInitialData;
			pInitialData.pSysMem = data;
			pInitialData.SysMemPitch = 0;
			pInitialData.SysMemSlicePitch = 0;	

			if (FAILED(hr = mDevice->CreateBuffer(&bufferDesc, &pInitialData, &newBuffer)))
			{
				MessageBox(NULL, L"ID3D11Device::CreateBuffer() failed.",
					L"Error", MB_OK | MB_ICONERROR);
				return nullptr;
			}
		}
		else
		{
			if (FAILED(hr = mDevice->CreateBuffer(&bufferDesc, NULL, &newBuffer)))
			{
				MessageBox(NULL, L"ID3D11Device::CreateBuffer() failed.",
					L"Error", MB_OK | MB_ICONERROR);
				return nullptr;
			}
		}

		return newBuffer;
	}

	//Updates a graphics buffer
	void GraphicsManager::UpdateBuffer(ID3D11Buffer* buffer, const void* data)
	{
		D3D11_BUFFER_DESC desc;
		buffer->GetDesc(&desc);
		if (desc.Usage == D3D11_USAGE_DYNAMIC)
		{
			D3D11_MAPPED_SUBRESOURCE mappedBuff;
			HRESULT hr = mDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuff);
			// Copy the data into the buffer.
			memcpy(mappedBuff.pData, data, desc.ByteWidth);
			mDeviceContext->Unmap(buffer, 0);
		}
		else
		{
			mDeviceContext->UpdateSubresource(buffer, 0, NULL, data, 0, 0);
		}
	}
}
	
	

	