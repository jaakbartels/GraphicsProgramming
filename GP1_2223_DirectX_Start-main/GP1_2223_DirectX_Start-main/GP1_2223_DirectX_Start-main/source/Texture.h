#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		Texture(const std::string& path, ID3D11Device* pDevice);
		~Texture();
		
		ID3D11Texture2D* GetResource() const;
		ID3D11ShaderResourceView* GetShaderResourceView() const;
		ColorRGB Sample(const Vector2& uv) const;
		Vector3 SampleNormal(const Vector2& uv) const;
		float SampleFloat(const Vector2& uv) const;

	private:
		SDL_Surface* m_pSurface{ nullptr };

		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pShaderResourceView{};
	};
}