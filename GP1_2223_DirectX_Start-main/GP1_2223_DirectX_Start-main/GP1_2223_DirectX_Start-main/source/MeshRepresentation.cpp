#include "pch.h"
#include "MeshRepresentation.h"
#include "Texture.h"
#include <cassert>
#include "Camera.h"
#include "ShadingEffect.h"
#include "Utils.h"


MeshRepresentation::MeshRepresentation(ID3D11Device* pDevice, const std::string& name, std::unique_ptr<Effect> pEffect)
	: m_pEffect{ std::move(pEffect) }
{
	//Create Vertex Input
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{}; 
	vertexDesc[0].SemanticName = "POSITION"; 
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; 
	vertexDesc[0].AlignedByteOffset = 0; 
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA; 

	vertexDesc[1].SemanticName = "UV"; 
	vertexDesc[1]. Format = DXGI_FORMAT_R32G32B32_FLOAT; 
	vertexDesc[1].AlignedByteOffset = 12; 
	vertexDesc[1]. InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	if (!Utils::ParseOBJ(name, vertices, indices))
	{
		std::cout << "Can't read and parse OBJ file " << name << '\n';
	}

	//create vertx buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE; 
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size()); 
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; 
	bd.CPUAccessFlags = 0; bd.MiscFlags = 0; 
	D3D11_SUBRESOURCE_DATA initData = {}; 
		initData.pSysMem = vertices.data(); 
	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer); 
	if (FAILED(result)) return;

	//Create Input Layout 
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	const HRESULT resultInput = pDevice->CreateInputLayout( 
		vertexDesc, 
		numElements, 
		passDesc.pIAInputSignature,
		passDesc. IAInputSignatureSize, 
		&m_pInputLayout); 
	if (FAILED(resultInput)) return; //or return

	//Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0; bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(resultInput)) return;
}

MeshRepresentation::~MeshRepresentation()
{
	m_pIndexBuffer->Release();
	m_pVertexBuffer->Release();
	m_pInputLayout->Release();

}

void MeshRepresentation::Render(ID3D11DeviceContext* pDeviceContext)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);
	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
	
}

void MeshRepresentation::CycleFilteringMethods()
{
	m_pEffect->CycleFilteringMethods();
}

void MeshRepresentation::Update(const Matrix& viewProjectionMatrix, const Matrix& inverseViewMatrix)
{
	Matrix world{ m_ScaleMatrix * m_RotationMatrix * m_TranslationMatrix };
	m_pEffect->SetWorldViewProjectionMatrix(world * viewProjectionMatrix);
	m_pEffect->SetInverseViewMatrix(inverseViewMatrix);
	m_pEffect->SetWorldMatrix(world);
}

void MeshRepresentation::RotateX(float angle)
{
	m_RotationMatrix = Matrix::CreateRotationX(angle) * m_RotationMatrix;
}
void MeshRepresentation::RotateY(float angle)
{
	m_RotationMatrix = Matrix::CreateRotationY(angle) * m_RotationMatrix;
}
void MeshRepresentation::RotateZ(float angle)
{
	m_RotationMatrix = Matrix::CreateRotationZ(angle) * m_RotationMatrix;
}