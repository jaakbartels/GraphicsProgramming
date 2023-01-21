#pragma once
#include "DataTypes.h"
#include "Effect.h"

#include "Texture.h"


class ShadingEffect;

class MeshRepresentation
{
public:
	MeshRepresentation(ID3D11Device* pDevice, const std::string& name, std::unique_ptr<Effect> pEffect);
	~MeshRepresentation();
	void Render(ID3D11DeviceContext* pDeviceContext);

	void CycleFilteringMethods();

	void Update(const Matrix& viewProjectionMatrix, const Matrix& inverseViewMatrix);



	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);

private:

	Matrix m_ScaleMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_TranslationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_RotationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;
	uint32_t m_NumIndices;

	std::unique_ptr<Effect> m_pEffect{};
	

	std::unique_ptr<dae::Texture> m_pDiffuseTexture{};
	std::unique_ptr<dae::Texture> m_pNormalTexture{};
	std::unique_ptr<dae::Texture> m_pSpecularTexture{};
	std::unique_ptr<dae::Texture> m_pGlossinessTexture{};
};
