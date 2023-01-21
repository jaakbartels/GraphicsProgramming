#pragma once
#include "Effect.h"
#include "Texture.h"
class dae::Texture;

class ShadingEffect final : public Effect
{
public:
	ShadingEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ShadingEffect(const ShadingEffect&) = delete;
	ShadingEffect(ShadingEffect&&) noexcept = delete;
	ShadingEffect& operator=(const ShadingEffect&) = delete;
	ShadingEffect& operator=(ShadingEffect&&) noexcept = delete;

	enum class FilteringMethod
	{
		Point, Linear, Anisotropic, END
	};

	void CycleFilteringMethods();

	void SetSpecularMap(dae::Texture* pSpecularTexture);
	void SetNormalMap(dae::Texture* pNormalTexture);
	void SetGlossinessMap(dae::Texture* pGlossinessTexture);
private:


	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	FilteringMethod m_FilteringMethod;

};

