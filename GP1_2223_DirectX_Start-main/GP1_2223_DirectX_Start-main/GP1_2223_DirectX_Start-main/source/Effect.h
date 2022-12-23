#pragma once
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
	ID3D11InputLayout* GetInputLayout();

private:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3D11InputLayout* m_pInputLayout;


	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};

