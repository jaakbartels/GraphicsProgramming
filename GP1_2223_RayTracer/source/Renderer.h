#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;
		void ToggleShadows() { m_RenderShadows = !m_RenderShadows; }
		void CycleLightingMode();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		void Render(Scene* pScene, int fromX, int toX, int fromY, int toY) const;
		bool SaveBufferToImage() const;

	private:
		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_currentLightingMode{ LightingMode::Combined };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};
		bool m_RenderShadows = true;
		int m_Width{};
		int m_Height{};
	};
}
