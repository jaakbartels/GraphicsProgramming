#include "pch.h"

#include "Effect.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <sstream>

#include "Matrix.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

	//m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	m_pTechnique = m_pEffect->GetTechniqueByName("PointFilteringTechnique");
	if (!m_pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";

	m_pMatrixWorldViewProjVariable = m_pEffect->GetVariableByName("WorldViewProj")->AsMatrix();
	if (!m_pMatrixWorldViewProjVariable->IsValid())
	{
		std::wcout << L"m_pMatrixWorldViewProjVariable not valid! \n";
	}

	m_pViewInvVariable = m_pEffect->GetVariableByName("ViewInverseMatrix")->AsMatrix();
	if (!m_pViewInvVariable->IsValid())
	{
		std::wcout << L"m_pViewInvVariable not valid!\n";
	}

	m_pWorldVariable = m_pEffect->GetVariableByName("WorldMatrix")->AsMatrix();
	if (!m_pWorldVariable->IsValid())
	{
		std::wcout << L"m_pWorldVariable not valid!\n";
	}

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("DiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable not valid!\n";
	}

}


Effect::~Effect()
{
	m_pTechnique->Release();
	m_pEffect->Release();
}

void Effect::SetWorldViewProjectionMatrix(const dae::Matrix& matrix)
{
	m_pMatrixWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void Effect::SetInverseViewMatrix(const dae::Matrix& matrix)
{
	m_pViewInvVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void Effect::SetWorldMatrix(const dae::Matrix& matrix)
{
	m_pWorldVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void Effect::SetDiffuseMap(dae::Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
	}
}


ID3DX11Effect* Effect::GetEffect()
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechnique()
{
	return m_pTechnique;
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;
	DWORD shaderFlags = 0;



#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);
	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}


void Effect::CycleFilteringMethods()
{

	m_FilteringMethod = static_cast<FilteringMethod>((static_cast<int>(m_FilteringMethod) + 1) % (static_cast<int>(FilteringMethod::END)));

	std::cout << "filter method ";
	switch (m_FilteringMethod)
	{
	case FilteringMethod::Point:
		m_pTechnique = m_pEffect->GetTechniqueByName("PointFilteringTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"PointTechnique not valid\n";
		std::cout << "Point\n";
		break;
	case FilteringMethod::Linear:
		m_pTechnique = m_pEffect->GetTechniqueByName("LinearFilteringTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"LinearTechnique not valid\n";
		std::cout << "Linear\n";
		break;
	case FilteringMethod::Anisotropic:
		m_pTechnique = m_pEffect->GetTechniqueByName("AnisotropicFilteringTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"AnisotropicTechnique not valid\n";
		std::cout << "Anisotropic\n";
		break;
	}
}