#pragma once
#include "DataTypes.h"
#include "Effect.h"

#include "Texture.h"


class ShadingEffect;

class Mesh
{
public:
	Mesh(ID3D11Device* pDevice, const std::string& name, std::unique_ptr<Effect> pEffect, bool supportsSoftwareRenderer = true);
	~Mesh();

	void Render(ID3D11DeviceContext* pDeviceContext);

	void CycleFilteringMethods();

	void Update(const Matrix& viewProjectionMatrix, const Matrix& inverseViewMatrix);

	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);
	Matrix WorldMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };
	std::vector<Vertex_Out> vertices_out{};

	void SetDiffuseTexture(Texture* pTexture);
	void SetNormalTexture(Texture* pTexture);
	void SetSpecularTexture(Texture* pTexture);
	void SetGlossinessTexture(Texture* pTexture);

	Texture* GetDiffuseTexture() const;
	Texture* GetNormalTexture() const;
	Texture* GetSpecularTexture() const;
	Texture* GetGlossinessTexture() const;

	bool SupportSoftwareRenderer{ true };
private:

	Matrix m_ScaleMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_TranslationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_RotationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;
	uint32_t m_NumIndices;

	std::unique_ptr<Effect> m_pEffect{};
	
	Texture* m_pDiffuseTexture{};
	Texture* m_pNormalTexture{};
	Texture* m_pSpecularTexture{};
	Texture* m_pGlossinessTexture{};
};
