#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();
		void ToggleShowDepthBuffer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY);
		float Remap(float v, float min, float max) const;

		bool SaveBufferToImage() const;
		ColorRGB PixelShading(const Vertex_Out& v);
		bool ShowDepthBuffer{ false };
		void ToggleRotation();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		bool m_Rotating{ true };

		//W2
		Texture* m_pTexture;
		Texture* m_pNormalMap;
		Texture* m_pGlossMap;
		Texture* m_pSpecularMap;

		std::vector<Mesh> m_Meshes {};

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunctionW1(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const;
		bool IsInTriangle(const Vector2& pixel, const Vector2& a, const Vector2& b, const Vector2& c);

		void Renderer_W4_01(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY);
		float Interpolate(float value0, float value1, float value2, float w0, float w1, float w2);
		Vector3 Interpolate(Vector3 value0, Vector3 value1, Vector3 value2, float w0, float w1, float w2);
	};
}
