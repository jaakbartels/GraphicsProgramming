#pragma once
#include "Texture.h"
class dae::Texture;

class Effect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~Effect();

	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	ID3DX11Effect* GetEffect();
	ID3DX11EffectTechnique* GetTechnique();

	enum class FilteringMethod
	{
		Point, Linear, Anisotropic, END
	};

	void CycleFilteringMethods();

	void SetWorldMatrix(const dae::Matrix& matrix);
	void SetWorldViewProjectionMatrix(const dae::Matrix& matrix);
	void SetInverseViewMatrix(const dae::Matrix& matrix);

	void SetSpecularMap(dae::Texture* pSpecularTexture);
	void SetNormalMap(dae::Texture* pNormalTexture);
	void SetGlossinessMap(dae::Texture* pGlossinessTexture);
	void SetDiffuseMap(dae::Texture* pDiffuseTexture);
private:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;


	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	ID3DX11EffectMatrixVariable* m_pMatrixWorldViewProjVariable;
	ID3DX11EffectMatrixVariable* m_pViewInvVariable{};
	ID3DX11EffectMatrixVariable* m_pWorldVariable{};

	FilteringMethod m_FilteringMethod;
	

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};

