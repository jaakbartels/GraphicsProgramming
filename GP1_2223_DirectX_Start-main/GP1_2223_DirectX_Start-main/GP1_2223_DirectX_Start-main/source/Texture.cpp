#include "pch.h"
#include "Texture.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(const std::string& path, ID3D11Device* pDevice)
	{
		// Make SDL_Surface, release at the end
		m_pSurface = IMG_Load(path.c_str());

		// Texture description
		const DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		// InitData SDL_Surface
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

		// ShaderResourceView description
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);
	}
	Texture::~Texture()
	{
		m_pResource->Release();
		m_pShaderResourceView->Release();
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}

	}
	ID3D11Texture2D* Texture::GetResource() const
	{
		return m_pResource;
	}
	ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
	{
		return m_pShaderResourceView;
	}


	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		if (this == nullptr) return { 0,0,0 };
		//TODO
		//Sample the correct texel for the given uv
		const int textureWidth{ m_pSurface->w };
		const int textureHeight{ m_pSurface->h };

		const Vector2 convertedUV{ static_cast<float> (static_cast<int> (uv.x * textureWidth)),static_cast<float>(static_cast<int> (uv.y * textureHeight)) };
		auto m_pSurfacePixels{ (uint32_t*)m_pSurface->pixels };
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

		Vector3 value{ 2.f * valueAsColor.r - 1.f , 2.f * valueAsColor.g - 1.f, 2.f * valueAsColor.b - 1.f };
		return value;
	}

	float Texture::SampleFloat(const Vector2& uv) const
	{
		auto valueAsColor = Sample(uv);

		return valueAsColor.r;
	}
}

