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

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		//W2
		Texture* m_pTexture;

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunctionW1(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const;
		bool IsInTriangle(const Vector2& pixel, const Vector2& a, const Vector2& b, const Vector2& c);

		void Renderer_W1_01();
		void Renderer_W1_02();
		void Renderer_W1_03();
		void Renderer_W1_04();
		void Renderer_W1_05();

		void Renderer_W2_01();
		void Renderer_W2_02();
		void Renderer_W2_03();
	};
}
