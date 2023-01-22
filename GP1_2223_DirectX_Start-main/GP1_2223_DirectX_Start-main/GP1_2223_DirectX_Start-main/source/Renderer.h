#pragma once
#include <functional>

#include "Camera.h"
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() ;
		void RenderSoftware() ;
		void RenderDirectX() ;

		void ToggleRotation();
		void ToggleUniformColor();
		void ToggleShowFireFX();
		void CycleShadingMode();
		void ToggleShowDepthBufer();
		void ToggleShowBoundingBoxes();
		void ToggleRenderFunction();
		void ToggleUseNormalMap();
		void CycleFilteringMethods();

	private:

		//Common (DirectX + Soft)
		Camera m_Camera;
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};


		std::vector<Mesh*> m_Meshes;

		std::function<void()> m_pCurrentRenderFunc;
		bool m_IsUsingDirectX{ true };
		
		bool m_F2Held{ false };// CycleFilteringMethod
		bool m_F5Held{ false };// ToggleRotation

		//Software Renderer

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		enum class ShadingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined
		};

		ShadingMode m_ShadingMode{ShadingMode::Combined};
		bool m_Rotating{ true };
		bool m_UniformColor{ false };
		bool m_ShowDepthBuffer{ false };
		bool m_ShowBoundingBoxes{ false };
		bool m_ShowFireFX{true};
		bool m_UseNormalMap{true};

		float m_Angle{};

		void Renderer_W4_01(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY);
		float Remap(float v, float min, float max) ;
		void Renderer_W4_01(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY) const;
		void VertexTransformationFunctionW1(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version
		void VertexTransformationFunction( std::vector<Mesh*>& meshes) ;
		bool SaveBufferToImage() const;
		ColorRGB PixelShading(const Mesh* pMesh, const Vertex_Out& v);
		bool IsInTriangle(const Vector2& pixel, const Vector2& a, const Vector2& b, const Vector2& c);

		float Interpolate(float value0, float value1, float value2, float w0, float w1, float w2, float interpolatedW);
		Vector3 Interpolate(Vector3 value0, Vector3 value1, Vector3 value2, float w0, float w1, float w2, float vw0, float vw1, float vw2, float interpolatedW);

		//DIRECTX Renderer
		bool m_IsDirectXInitialized{ false };

		HRESULT InitializeDirectX();
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;
	};
}
