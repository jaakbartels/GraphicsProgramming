#include "pch.h"
#include "ShadingEffect.h"

#include "Effect.h"

ShadingEffect::ShadingEffect(ID3D11Device* pDevice, const std::wstring& assetFile) : ::Effect(pDevice, assetFile)
{

	m_pNormalMapVariable = m_pEffect->GetVariableByName("NormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable not valid!\n";
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("GlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"m_pGlossinessMapVariable not valid!\n";
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("SpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable not valid!\n";
	}
}

void ShadingEffect::SetSpecularMap(dae::Texture* pSpecularTexture)
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetShaderResourceView());
	}
}

void ShadingEffect::SetNormalMap(dae::Texture* pNormalTexture)
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->SetResource(pNormalTexture->GetShaderResourceView());
	}
}


void ShadingEffect::SetGlossinessMap(dae::Texture* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetShaderResourceView());
	}
}

void ShadingEffect::CycleFilteringMethods()
{

	m_FilteringMethod = static_cast<FilteringMethod>((static_cast<int>(m_FilteringMethod) + 1) % (static_cast<int>(FilteringMethod::END)));

	std::cout << "filter method ";
	switch (m_FilteringMethod)
	{
	case ShadingEffect::FilteringMethod::Point:
		m_pTechnique = m_pEffect->GetTechniqueByName("PointFilteringTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"PointTechnique not valid\n";
		std::cout << "Point\n";
		break;
	case ShadingEffect::FilteringMethod::Linear:
		m_pTechnique = m_pEffect->GetTechniqueByName("LinearFilteringTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"LinearTechnique not valid\n";
		std::cout << "Linear\n";
		break;
	case ShadingEffect::FilteringMethod::Anisotropic:
		m_pTechnique = m_pEffect->GetTechniqueByName("AnisotropicFilteringTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"AnisotropicTechnique not valid\n";
		std::cout << "Anisotropic\n";
		break;
	}
}