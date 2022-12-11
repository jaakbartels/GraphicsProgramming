#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"
#include "Vector3.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;
		Vector3 SampleNormal(const Vector2& uv) const;
		float SampleFloat(const Vector2& uv) const;

	private:
		Texture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}
