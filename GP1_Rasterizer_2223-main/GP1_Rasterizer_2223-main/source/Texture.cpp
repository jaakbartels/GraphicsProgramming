#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <assert.h>

#include "Vector3.h"

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		const auto loadedImage{ IMG_Load(path.c_str()) };
		assert(loadedImage != nullptr && "There was no file found");

		Texture* imageTexture = new Texture{ loadedImage };
		return imageTexture;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		const int textureWidth{ m_pSurface->w };
		const int textureHeight{ m_pSurface->h };

		const Vector2 convertedUV{ static_cast<float> (static_cast<int> (uv.x * textureWidth)),static_cast<float>(static_cast<int> (uv.y * textureHeight)) };
		const auto desiredPixel{ m_pSurfacePixels[static_cast<size_t>(convertedUV.y * textureWidth + convertedUV.x)] };

		Uint8 redValue{};
		Uint8 blueValue{};
		Uint8 greenValue{};

		SDL_GetRGB(desiredPixel, m_pSurface->format, &redValue, &blueValue, &greenValue);

		ColorRGB desiredColor{ static_cast<float>(redValue), static_cast<float>(blueValue), static_cast<float>(greenValue) };
		desiredColor.r /= 255.f;
		desiredColor.g /= 255.f;
		desiredColor.b /= 255.f;

		return desiredColor;
	}

	Vector3 Texture::SampleNormal(const Vector2& uv) const
	{
		auto valueAsColor = Sample(uv);

		Vector3 value{ 2.f * valueAsColor.r - 1.f , 2.f*valueAsColor.g - 1.f, 2.f*valueAsColor.b - 1.f };
		return value;
	}

	float Texture::SampleFloat(const Vector2& uv) const
	{
		auto valueAsColor = Sample(uv);

		return valueAsColor.r;
	}
}
