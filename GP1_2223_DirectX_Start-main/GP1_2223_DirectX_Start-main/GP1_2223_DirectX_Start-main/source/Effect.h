#pragma once
#include <d3d11.h>
#include <d3dx11effect.h>

#include "Matrix.h"
#include "Texture.h"
class dae::Texture;

class Effect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect();

	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	ID3DX11Effect* GetEffect();
	ID3DX11EffectTechnique* GetTechnique();

	void SetWorldMatrix(const dae::Matrix& matrix);
	void SetDiffuseMap(dae::Texture* pDiffuseTexture);
	void SetWorldViewProjectionMatrix(const dae::Matrix& matrix);
	void SetInverseViewMatrix(const dae::Matrix& matrix);

	void CycleFilteringMethods();

	enum class FilteringMethod
	{
		Point, Linear, Anisotropic, END
	};

protected:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectMatrixVariable* m_pMatrixWorldViewProjVariable;
	ID3DX11EffectMatrixVariable* m_pViewInvVariable{};
	ID3DX11EffectMatrixVariable* m_pWorldVariable{};

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	FilteringMethod m_FilteringMethod;

};
