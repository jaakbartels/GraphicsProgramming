#include "pch.h"
#include "Renderer.h"

#include "BRDFs.h"
#include "Mesh.h"
#include "ShadingEffect.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow)
		: m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Set initial render function (can be changed while running)
		m_pCurrentRenderFunc = [this] {RenderDirectX(); };
		m_IsUsingDirectX = true;

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsDirectXInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		m_Camera.Initialize(45.f, { 0.f,0.f,-50.f });

		//Initialize Soft pipeline (create buffers)
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		auto pShadingEffect{ std::make_unique<ShadingEffect>(m_pDevice, L"Resources/PosCol3D.fx") };
		auto diffuseTexture = new Texture("Resources/vehicle_diffuse.png", m_pDevice);
		auto normalMap = new Texture("Resources/vehicle_normal.png", m_pDevice);
		auto specularMap = new Texture("Resources/vehicle_specular.png", m_pDevice);
		auto glossMap = new Texture("Resources/vehicle_gloss.png", m_pDevice);

		pShadingEffect->SetDiffuseMap(diffuseTexture);
		pShadingEffect->SetNormalMap(normalMap);
		pShadingEffect->SetSpecularMap(specularMap);
		pShadingEffect->SetGlossinessMap(glossMap);

		auto vehicle = new Mesh(m_pDevice, "Resources/vehicle.obj", std::move(pShadingEffect));
		vehicle->SetDiffuseTexture(diffuseTexture);
		vehicle->SetNormalTexture(normalMap);
		vehicle->SetGlossinessTexture(glossMap);
		vehicle->SetSpecularTexture(specularMap);
		vehicle->primitiveTopology = PrimitiveTopology::TriangleList;

		m_Meshes.push_back(vehicle);

		auto pFlatEffect{ std::make_unique<Effect>(m_pDevice, L"Resources/Flat.fx") };
		auto fireTexture = new Texture("Resources/fireFX_diffuse.png", m_pDevice);
		pFlatEffect->SetDiffuseMap(fireTexture);

		auto fireFX = new Mesh(m_pDevice, "Resources/fireFX.obj", std::move(pFlatEffect), /*supportsSoftwareRenderer = */ false);
		fireFX->SetDiffuseTexture(fireTexture);
		m_Meshes.push_back(fireFX);
	}

	Renderer::~Renderer()
	{
		for (auto& m : m_Meshes)
		{
			delete m;
		}

		delete[] m_pDepthBufferPixels;

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
			if (m_Rotating)
			{
				m->RotateY(rotationSpeed * TO_RADIANS * pTimer->GetElapsed());
			}
			m->Update(m_Camera.GetWorldViewProjection(), m_Camera.GetInvViewMatrix());
		}

		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	}


	void Renderer::Render() 
	{
		m_pCurrentRenderFunc();
	}

	void Renderer::RenderDirectX()
	{
		//1. CLEAR RTV & DSV
		ColorRGB clearColor = m_UniformColor ? ColorRGB{.1f, .1f, .1f} : ColorRGB{ 0.39f, 0.59f, 0.93f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		for (auto& m : m_Meshes)
		{
			if (m->SupportSoftwareRenderer || m_ShowFireFX)
			{
				m->Render(m_pDeviceContext);
			}
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

	void Renderer::CycleFilteringMethods()
	{
		for (Mesh* pMesh : m_Meshes)
		{
			pMesh->CycleFilteringMethods();
		}
	}

	void Renderer::ToggleRotation()
	{
		m_Rotating = !m_Rotating;
		std::cout << "Rotation " << (m_Rotating ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleUniformColor()
	{
		m_UniformColor = !m_UniformColor;
		std::cout << "Uniform Color " << (m_UniformColor ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleShowFireFX()
	{
		m_ShowFireFX = !m_ShowFireFX;
		std::cout << "Show Fire FX " << (m_ShowFireFX ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleShowDepthBufer()
	{
		m_ShowDepthBuffer = !m_ShowDepthBuffer;
		std::cout << "Show Depth buffer " << (m_ShowDepthBuffer ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleShowBoundingBoxes()
	{
		m_ShowBoundingBoxes = !m_ShowBoundingBoxes;
		std::cout << "Show Bounding Boxes " << (m_ShowBoundingBoxes ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleUseNormalMap()
	{
		m_UseNormalMap = !m_UseNormalMap;
		std::cout << "Use Normal map " << (m_UseNormalMap ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleRenderFunction()
	{
		if (m_IsUsingDirectX)
		{
			m_IsUsingDirectX = false;
			m_pCurrentRenderFunc = [this] {RenderSoftware(); };
			std::cout << "Software Rasterizer" << "\n";
		}
		else
		{
			m_IsUsingDirectX = true;
			m_pCurrentRenderFunc = [this] {RenderDirectX(); };
			std::cout << "DirectX Rasterizer" << "\n";
		}
	}

	void Renderer::CycleShadingMode()
	{
		if (int(m_ShadingMode) < 3)
		{
			m_ShadingMode = ShadingMode(int(m_ShadingMode) + 1);
		}
		else
		{
			m_ShadingMode = ShadingMode(0);
		}

		const char* names[] = {"ObservedArea",	"Diffuse", "Specular",	"Combined"};

		std::cout << "ShadingMode set to " <<  names[int(m_ShadingMode)] << "\n";
	}

	//Soft

	void Renderer::RenderSoftware() 
	{
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);


		uint32_t fromX = 0;
		uint32_t fromY = 0;
		uint32_t toX = m_Width;
		uint32_t toY = m_Height;

		//Clear backBuffer
		ColorRGB clearColor = (m_UniformColor ? ColorRGB{.1f, .1f, .1f } : ColorRGB{.39f,.39f,.39f }) * 255;

		uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 8 | (uint32_t)clearColor.g << 16 | (uint32_t)clearColor.r;
		SDL_FillRect(m_pBackBuffer, NULL, hexColor);
		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
		VertexTransformationFunction(m_Meshes);
		for (auto& pMesh : m_Meshes)
		{
			if (!pMesh->SupportSoftwareRenderer)
			{
				continue;
			}

			for (int i{ 0 }; i < pMesh->indices.size() - 2; i += 3)
			{
				//Points of the Triangle
				const int index0{ int(pMesh->indices[i]) };
				int index1{ int(pMesh->indices[i + 1]) };
				int index2{ int(pMesh->indices[i + 2]) };

				if (pMesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
				{
					if (i % 2 != 0)
					{
						std::swap(index1, index2);
					}
				}

				const auto V0{ pMesh->vertices_out[index0] };
				const auto V1{ pMesh->vertices_out[index1] };
				const auto V2{ pMesh->vertices_out[index2] };

				const Vector4 pos0{ V0.position };
				const Vector4 pos1{ V1.position };
				const Vector4 pos2{ V2.position };

				float topLeftX = std::min(pos0.x, std::min(pos1.x, pos2.x));
				float topLeftY = std::max(pos0.y, std::max(pos1.y, pos2.y));
				float bottomRightX = std::max(pos0.x, std::max(pos1.x, pos2.x));
				float bottomRightY = std::min(pos0.y, std::min(pos1.y, pos2.y));


				topLeftX = Clamp(topLeftX, fromX, toX - 1);
				topLeftY = Clamp(topLeftY, fromY, toY - 1);
				bottomRightX = Clamp(bottomRightX, fromX, toX - 1);
				bottomRightY = Clamp(bottomRightY, fromY, toY - 1);

				// Define the edges of the screen triangle
				const Vector2 V01{ pos0.GetXY(), pos1.GetXY() };
				const Vector2 V12{ pos1.GetXY(), pos2.GetXY() };
				const Vector2 V20{ pos2.GetXY(), pos0.GetXY() };

				//RENDER LOGIC
				for (int px{ int(topLeftX) }; px < bottomRightX; ++px)
				{
					for (int py{ int(bottomRightY) }; py < topLeftY; ++py)
					{
						if (m_ShowBoundingBoxes)
						{
							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(255),
								static_cast<uint8_t>(255),
								static_cast<uint8_t>(255));

							continue;
						}

						Vector2 pixel{ float(px), float(py) };
						ColorRGB finalColor{ 0.0f, 0.0f, 0.0f };

						const float signedArea2{ Vector2::Cross(V01, Vector2{ pos0.GetXY(), pixel}) };
						const float signedArea0{ Vector2::Cross(V12, Vector2{ pos1.GetXY(), pixel}) };
						const float signedArea1{ Vector2::Cross(V20, Vector2{ pos2.GetXY(), pixel}) };
						const float doubleOfTriangleArea = signedArea2 + signedArea0 + signedArea1;

						if (signedArea2 > 0 && signedArea0 > 0 && signedArea1 > 0)
						{
							const float w2{ signedArea2 / doubleOfTriangleArea };
							const float w0{ signedArea0 / doubleOfTriangleArea };
							const float w1{ signedArea1 / doubleOfTriangleArea };

							const float interpolatedZ{ 1 / ((1 / pos0.z) * w0 + (1 / pos1.z) * w1 + (1 / pos2.z) * w2) };

							if (interpolatedZ >= 0.f && interpolatedZ <= 1.f && interpolatedZ < m_pDepthBufferPixels[px + (py * m_Width)])
							{
								m_pDepthBufferPixels[px + (py * m_Width)] = interpolatedZ;

								const float interpolatedW{ 1 / ((1 / pos0.w) * w0 + (1 / pos1.w) * w1 + (1 / pos2.w) * w2) };

								Vector2 interpolatedUV{
									pMesh->vertices[index0].uv / pos0.w * w0
									+ pMesh->vertices[index1].uv / pos1.w * w1
									+ pMesh->vertices[index2].uv / pos2.w * w2 };

								interpolatedUV *= interpolatedW;

								if (interpolatedUV.x < 0.f) interpolatedUV.x = 0.f;
								if (interpolatedUV.y < 0.f) interpolatedUV.y = 0.f;
								if (interpolatedUV.x > 1.f) interpolatedUV.x = 1.f;
								if (interpolatedUV.y > 1.f) interpolatedUV.y = 1.f;

								if (m_ShowDepthBuffer)
								{
									float v = Remap(interpolatedZ, 0.995f, 1.f);
									finalColor = { v,v,v };
								}
								else
								{
									Vertex_Out v{};
									v.uv = interpolatedUV;

									float vw0 = pos0.w;
									float vw1 = pos1.w;
									float vw2 = pos2.w;

									v.normal = Interpolate(V0.normal, V1.normal, V2.normal, w0, w1, w2, vw0, vw1, vw2, interpolatedW);
									v.normal.Normalize();
									v.tangent = Interpolate(V0.tangent, V1.tangent, V2.tangent, w0, w1, w2, vw0, vw1, vw2, interpolatedW);
									v.tangent.Normalize();
									v.viewDirection = Interpolate(V0.viewDirection, V1.viewDirection, V2.viewDirection, w0, w1, w2, vw0, vw1, vw2, interpolatedW);
									v.viewDirection.Normalize();


									finalColor = PixelShading(pMesh, v);
								}

								//Update Color in Buffer
								finalColor.MaxToOne();

								m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(finalColor.r * 255),
									static_cast<uint8_t>(finalColor.g * 255),
									static_cast<uint8_t>(finalColor.b * 255));
							}
						}

					}
				}
			}
		}

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	//Remap value v from [min, max] to [0, 1]
	float Renderer::Remap(float v, float min, float max) 
	{
		float result{ (v - min) / (max - min) };
		return Clamp(result, 0.f, 1.f);
	}

	void Renderer::Renderer_W4_01(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY) 
	{
		//Clear backBuffer
		ColorRGB clearColor{ 100, 100, 100 };
		uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 8 | (uint32_t)clearColor.g << 16 | (uint32_t)clearColor.r;
		SDL_FillRect(m_pBackBuffer, NULL, hexColor);

		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
		VertexTransformationFunction(m_Meshes);

		for (auto& pMesh : m_Meshes)
		{
			for (int i{ 0 }; i < pMesh->indices.size() - 2; i += 3)
			{
				//Points of the Triangle
				const int index0{ int(pMesh->indices[i]) };
				int index1{ int(pMesh->indices[i + 1]) };
				int index2{ int(pMesh->indices[i + 2]) };

				if (pMesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
				{
					if (i % 2 != 0)
					{
						std::swap(index1, index2);
					}
				}

				const auto V0{ pMesh->vertices_out[index0] };
				const auto V1{ pMesh->vertices_out[index1] };
				const auto V2{ pMesh->vertices_out[index2] };

				const Vector4 pos0{ V0.position };
				const Vector4 pos1{ V1.position };
				const Vector4 pos2{ V2.position };

				//if (toIdx - fromIdx <= 4)
				//{
				//	std::cout << "V0 : " << V0.x << ", " << V0.y << ", " << V0.z << ", " << V0.w << "\n";
				//	std::cout << "V1 : " << V1.x << ", " << V1.y << ", " << V1.z << ", " << V1.w << "\n";
				//	std::cout << "V2 : " << V2.x << ", " << V2.y << ", " << V2.z << ", " << V2.w << "\n";
				//}

				float topLeftX = std::min(pos0.x, std::min(pos1.x, pos2.x));
				float topLeftY = std::max(pos0.y, std::max(pos1.y, pos2.y));
				float bottomRightX = std::max(pos0.x, std::max(pos1.x, pos2.x));
				float bottomRightY = std::min(pos0.y, std::min(pos1.y, pos2.y));


				topLeftX = Clamp(topLeftX, fromX, toX - 1);
				topLeftY = Clamp(topLeftY, fromY, toY - 1);
				bottomRightX = Clamp(bottomRightX, fromX, toX - 1);
				bottomRightY = Clamp(bottomRightY, fromY, toY - 1);

				// Define the edges of the screen triangle
				const Vector2 V01{ pos0.GetXY(), pos1.GetXY() };
				const Vector2 V12{ pos1.GetXY(), pos2.GetXY() };
				const Vector2 V20{ pos2.GetXY(), pos0.GetXY() };

				//RENDER LOGIC
				for (int px{ int(topLeftX) }; px < bottomRightX; ++px)
				{
					for (int py{ int(bottomRightY) }; py < topLeftY; ++py)
					{
						Vector2 pixel{ float(px), float(py) };
						ColorRGB finalColor{ 0.0f, 0.0f, 0.0f };

						const float signedArea2{ Vector2::Cross(V01, Vector2{ pos0.GetXY(), pixel}) };
						const float signedArea0{ Vector2::Cross(V12, Vector2{ pos1.GetXY(), pixel}) };
						const float signedArea1{ Vector2::Cross(V20, Vector2{ pos2.GetXY(), pixel}) };
						const float doubleOfTriangleArea = signedArea2 + signedArea0 + signedArea1;

						if (signedArea2 > 0 && signedArea0 > 0 && signedArea1 > 0)
						{
							const float w2{ signedArea2 / doubleOfTriangleArea };
							const float w0{ signedArea0 / doubleOfTriangleArea };
							const float w1{ signedArea1 / doubleOfTriangleArea };

							const float interpolatedZ{ 1 / ((1 / pos0.z) * w0 + (1 / pos1.z) * w1 + (1 / pos2.z) * w2) };

							if (interpolatedZ >= 0.f && interpolatedZ <= 1.f && interpolatedZ < m_pDepthBufferPixels[px + (py * m_Width)])
							{
								m_pDepthBufferPixels[px + (py * m_Width)] = interpolatedZ;

								const float interpolatedW{ 1 / ((1 / pos0.w) * w0 + (1 / pos1.w) * w1 + (1 / pos2.w) * w2) };

								Vector2 interpolatedUV{
									pMesh->vertices[index0].uv / pos0.w * w0
									+ pMesh->vertices[index1].uv / pos1.w * w1
									+ pMesh->vertices[index2].uv / pos2.w * w2 };

								interpolatedUV *= interpolatedW;

								if (interpolatedUV.x < 0.f) interpolatedUV.x = 0.f;
								if (interpolatedUV.y < 0.f) interpolatedUV.y = 0.f;
								if (interpolatedUV.x > 1.f) interpolatedUV.x = 1.f;
								if (interpolatedUV.y > 1.f) interpolatedUV.y = 1.f;

								if (m_ShowDepthBuffer)
								{
									float v = Remap(interpolatedZ, 0.995f, 1.f);
									finalColor = { v,v,v };
								}
								else
								{
									Vertex_Out v{};
									v.uv = interpolatedUV;

									float vw0 = pos0.w;
									float vw1 = pos1.w;
									float vw2 = pos2.w;

									v.normal = Interpolate(V0.normal, V1.normal, V2.normal, w0, w1, w2, vw0, vw1, vw2, interpolatedW);
									v.normal.Normalize();
									v.tangent = Interpolate(V0.tangent, V1.tangent, V2.tangent, w0, w1, w2, vw0, vw1, vw2, interpolatedW);
									v.tangent.Normalize();
									v.viewDirection = Interpolate(V0.viewDirection, V1.viewDirection, V2.viewDirection, w0, w1, w2, vw0, vw1, vw2, interpolatedW);
									v.viewDirection.Normalize();


									finalColor = PixelShading(pMesh, v);
								}

								//Update Color in Buffer
								finalColor.MaxToOne();

								m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(finalColor.r * 255),
									static_cast<uint8_t>(finalColor.g * 255),
									static_cast<uint8_t>(finalColor.b * 255));
							}
						}

					}
				}
			}
		}
	}

	float Renderer::Interpolate(float value0, float value1, float value2, float w0, float w1, float w2, float interpolatedW)
	{
		//return 1 / ((1 / value0) * w0 + (1 / value1) * w1 + (1 / value2) * w2);
		return value0 * w0 + value1 * w1 + value2 * w2 * interpolatedW;
	}

	Vector3 Renderer::Interpolate(Vector3 value0, Vector3 value1, Vector3 value2, float w0, float w1, float w2, float vw0, float vw1, float vw2, float interpolatedW)
	{
		return ((value0 / vw0) * w0 + (value1 / vw1) * w1 + (value2 / vw2) * w2) * interpolatedW;

		return
		{
			//Interpolate(value0.x, value1.x, value2.x, w0,w1,w2,interpolatedW) ,
			//Interpolate(value0.y, value1.y, value2.y, w0,w1,w2,interpolatedW),
			//Interpolate(value0.z, value1.z, value2.z, w0,w1,w2,interpolatedW)
		};
	}

	void Renderer::VertexTransformationFunction(std::vector<Mesh*>&  meshes) 
	{

		for (auto& mesh : meshes)
		{
			auto postionMatrix = mesh->WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix;

			mesh->vertices_out.clear();
			mesh->vertices_out.reserve(mesh->vertices.size());

			for (const auto& vertex : mesh->vertices)
			{
				Vector4 screenSpaceVertex = postionMatrix.TransformPoint(Vector4{ vertex.position, 1.f });

				//Perspective Divide (perspective distortion)
				screenSpaceVertex.x /= screenSpaceVertex.w;
				screenSpaceVertex.y /= screenSpaceVertex.w;
				screenSpaceVertex.z /= screenSpaceVertex.w;

				screenSpaceVertex.x = ((screenSpaceVertex.x + 1) / 2.f) * float(m_Width);
				screenSpaceVertex.y = ((1 - screenSpaceVertex.y) / 2.f) * float(m_Height);

				auto vertex_out = Vertex_Out{ screenSpaceVertex };

				vertex_out.normal = mesh->WorldMatrix.TransformVector(vertex.normal).Normalized();
				vertex_out.tangent = mesh->WorldMatrix.TransformVector(vertex.tangent).Normalized();
				vertex_out.viewDirection = mesh->WorldMatrix.TransformPoint(vertex.position) - m_Camera.origin;
				mesh->vertices_out.emplace_back(vertex_out);
			}
		}
	}

	bool Renderer::SaveBufferToImage() const
	{
		return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
	}

	ColorRGB Renderer::PixelShading(const Mesh* pMesh, const Vertex_Out& v)
	{
		const Vector3 lightDirection{ .577f, -.577f, .577f };
		const ColorRGB ambient{ .025f, .025f, .025f };
		const float shininess{ 25.f };

		auto color = pMesh->GetDiffuseTexture()->Sample(v.uv);

		auto normalFromMap = pMesh->GetNormalTexture()->SampleNormal(v.uv);;

		Vector3 binormal{ Vector3::Cross(v.normal, v.tangent) };
		Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3::Zero };

		auto normal = tangentSpaceAxis.TransformVector( m_UseNormalMap ? normalFromMap : Vector3::UnitZ);

		const auto observedArea = Vector3::Dot(normal, -lightDirection);

		if (observedArea < 0)
		{
			return ColorRGB{};
		}

		auto glossiness = pMesh->GetGlossinessTexture()->SampleFloat(v.uv);
		auto specular = pMesh->GetSpecularTexture()->Sample(v.uv);

		auto phong = BRDF::Phong(2.f, glossiness * shininess, lightDirection, -v.viewDirection, normal);
		ColorRGB phongClamped{ Clamp(phong.r, 0.f, 1.f), Clamp(phong.g, 0.f, 1.f) , Clamp(phong.b, 0.f, 1.f) };
		auto lambert = BRDF::Lambert(7.f, color);


		ColorRGB finalColor{};
		switch (m_ShadingMode)
		{
		case Renderer::ShadingMode::ObservedArea:
			finalColor = { observedArea, observedArea, observedArea };
			break;
		case Renderer::ShadingMode::Diffuse:
			finalColor = lambert * observedArea;
			break;
		case Renderer::ShadingMode::Specular:
			finalColor = phongClamped * specular;
			break;
		case Renderer::ShadingMode::Combined:
			finalColor = (lambert + phongClamped * specular + ambient) * observedArea;
			break;
		}
		return finalColor;
	}

}
