#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

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
        SDL_Surface* surface{};
        surface = IMG_Load(path.c_str());
        //Create & Return a new Texture Object (using SDL_Surface)
        Texture* texture{ new Texture{surface} };

        return texture;
    }

    ColorRGB Texture::Sample(const Vector2& uv) const
    {
        //TODO
        //Sample the correct texel for the given uv
        const float u{ uv.x * m_pSurface->w };
        const float v{ uv.y * m_pSurface->h };
        const int pixel{ int(uv.x + (uv.y * m_pSurface->w)) };
        Uint8 r{}, g{}, b{};
        SDL_GetRGB(m_pSurfacePixels[pixel], m_pSurface->format, &r, &g, &b);
        ColorRGB color{};
        color.r = r / 255.f;
        color.g = g / 255.f;
        color.b = b / 255.f;

        return color;
    }
}