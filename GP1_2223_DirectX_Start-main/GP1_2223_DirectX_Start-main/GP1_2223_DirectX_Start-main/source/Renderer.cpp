#include "pch.h"
#include "Renderer.h"
#include "MeshRepresentation.h"
#include "ShadingEffect.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow)
		: m_pWindow(pWindow)
		, m_Meshes()
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		m_Camera.Initialize(45.f, { 0.f,0.f,-50.f });

		//std::vector<Vertex> vertices{
		//	{{.0f, .5f, .5f }, {1.f, 0.f, 0.f}},
		//	{{.5f, -.5f, .5f }, {0.f, 0.f, 1.f}},
		//	{{-.5f, -.5f, .5f }, {0.f, 1.f, 0.f}},
		//};


		auto pShadingEffect{ std::make_unique<ShadingEffect>(m_pDevice, L"Resources/PosCol3D.fx") };

		pShadingEffect->SetDiffuseMap(std::make_unique<dae::Texture>("Resources/vehicle_diffuse.png", m_pDevice).get());
		pShadingEffect->SetNormalMap(std::make_unique<dae::Texture>("Resources/vehicle_normal.png", m_pDevice).get());
		pShadingEffect->SetSpecularMap(std::make_unique<dae::Texture>("Resources/vehicle_specular.png", m_pDevice).get());
		pShadingEffect->SetGlossinessMap(std::make_unique < dae::Texture >("Resources/vehicle_gloss.png", m_pDevice).get());

		m_Meshes.push_back(new MeshRepresentation(m_pDevice, "Resources/vehicle.obj", std::move(pShadingEffect)));

		auto pFlatEffect{ std::make_unique<Effect>(m_pDevice, L"Resources/Flat.fx") };
		pFlatEffect->SetDiffuseMap(std::make_unique<dae::Texture>("Resources/fireFX_diffuse.png", m_pDevice).get());
		m_Meshes.push_back(new MeshRepresentation(m_pDevice, "Resources/fireFX.obj", std::move(pFlatEffect)));
	}

	Renderer::~Renderer()
	{
		for (auto& m : m_Meshes)
		{
			delete m;
		}

		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);


		constexpr const float rotationSpeed{ 30.f };
		for (auto& m : m_Meshes)
		{
			if (m_EnableRotating)
			{
				m->RotateY(rotationSpeed * TO_RADIANS * pTimer->GetElapsed());
			}
			m->Update(m_Camera.GetWorldViewProjection(), m_Camera.GetInvViewMatrix());
		}

		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

		if (pKeyboardState[SDL_SCANCODE_F2])
		{
			if (!m_F2Held)
			{
				for (auto& m : m_Meshes)
				{
					m->CycleFilteringMethods();
				}
			}
			m_F2Held = true;
		}
		else m_F2Held = false;
		if (pKeyboardState[SDL_SCANCODE_F5])
		{
			if (!m_F5Held)
			{
				m_EnableRotating = !m_EnableRotating;

			}
			m_F5Held = true;
		}
		else m_F5Held = false;
	}


	void Renderer::Render() const
	{
		/*if (!m_IsInitialized)
			return;*/

			//1. CLEAR RTV & DSV
		ColorRGB clearColor = ColorRGB{ 0.f, 0.f, 0.3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		//2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		for (auto& m : m_Meshes)
		{
			m->Render(m_pDeviceContext);
		}

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);

	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		//2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version)
			SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;
		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;
		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//===== 
		// 
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer)); if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		//5. Bind RTV & DSV to Output Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;
	}
}
