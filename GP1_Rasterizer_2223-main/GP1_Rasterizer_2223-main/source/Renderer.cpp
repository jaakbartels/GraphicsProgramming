//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include <iostream>

#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include "BRDFs.h"
using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(45.f, { .0f, 0.0f,0.f });
	m_Camera.aspect = (m_Width / float(m_Height));
	//initialize Texture
	//m_pTexture = new Texture{ "Resources/uv_grid_2.png" };
	m_pTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
	m_pNormalMap = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pGlossMap = Texture::LoadFromFile("Resources/vehicle_gloss.png");
	m_pSpecularMap = Texture::LoadFromFile("Resources/vehicle_specular.png");

	m_LightMode = LightMode::Combined;

	Mesh vehicle{};
	Utils::ParseOBJ("Resources/vehicle.obj", vehicle.vertices, vehicle.indices);
	vehicle.primitiveTopology = PrimitiveTopology::TriangleList;
	m_Meshes.push_back(vehicle);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
	delete m_pNormalMap;
	delete m_pSpecularMap;
	delete m_pGlossMap;
}

void Renderer::ToggleShowDepthBuffer()
{
	ShowDepthBuffer = !ShowDepthBuffer;
}

void Renderer::Update(Timer* pTimer)
{
	if (m_Rotating)
	{
		const float rotationSpeed{ 50 * TO_RADIANS * pTimer->GetElapsed() };
		m_Angle += rotationSpeed;
		m_Meshes[0].worldMatrix = Matrix::CreateRotationY(m_Angle) * Matrix::CreateTranslation(0, 0, 50.f);
	}
	m_Camera.Update(pTimer);
}

void Renderer::Render(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY)
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	Renderer_W4_01( fromX,  fromY,  toX,  toY);

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

//Remap value v from [min, max] to [0, 1]
float Renderer::Remap(float v, float min, float max) const
{
	float result{ (v - min)/(max - min) };
	return Clamp(result, 0.f, 1.f);
}

void Renderer::Renderer_W4_01(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY)
{
	//Clear backBuffer
	ColorRGB clearColor{ 100, 100, 100 };
	uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 8 | (uint32_t)clearColor.g << 16 | (uint32_t)clearColor.r;
	SDL_FillRect(m_pBackBuffer, NULL, hexColor);

	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	VertexTransformationFunction(m_Meshes);

	for (auto mesh : m_Meshes)
	{
		for (int i{ 0 }; i < mesh.indices.size() - 2; i += 3)
		{
			//Points of the Triangle
			const int index0{ int(mesh.indices[i]) };
			int index1{ int(mesh.indices[i + 1]) };
			int index2{ int(mesh.indices[i + 2]) };

			if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
			{
				if (i % 2 != 0)
				{
					std::swap(index1, index2);
				}
			}

			const auto V0{ mesh.vertices_out[index0] };
			const auto V1{ mesh.vertices_out[index1] };
			const auto V2{ mesh.vertices_out[index2] };

			const Vector4 pos0{ V0.position };
			const Vector4 pos1{ V1.position };
			const Vector4 pos2{ V2.position };

			//if (toIdx - fromIdx <= 4)
			//{
			//	std::cout << "V0 : " << V0.x << ", " << V0.y << ", " << V0.z << ", " << V0.w << "\n";
			//	std::cout << "V1 : " << V1.x << ", " << V1.y << ", " << V1.z << ", " << V1.w << "\n";
			//	std::cout << "V2 : " << V2.x << ", " << V2.y << ", " << V2.z << ", " << V2.w << "\n";
			//}

			float topLeftX = std::min(pos0.x, std::min(pos1.x, pos2.x));
			float topLeftY = std::max(pos0.y, std::max(pos1.y, pos2.y));
			float bottomRightX = std::max(pos0.x, std::max(pos1.x, pos2.x));
			float bottomRightY = std::min(pos0.y, std::min(pos1.y, pos2.y));


			topLeftX = Clamp(topLeftX, fromX, toX - 1);
			topLeftY = Clamp(topLeftY, fromY, toY-1);
			bottomRightX = Clamp(bottomRightX, fromX, toX - 1);
			bottomRightY = Clamp(bottomRightY, fromY, toY - 1);

			// Define the edges of the screen triangle
			const Vector2 V01{ pos0.GetXY(), pos1.GetXY() };
			const Vector2 V12{ pos1.GetXY(), pos2.GetXY() };
			const Vector2 V20{ pos2.GetXY(), pos0.GetXY() };

			//RENDER LOGIC
			for (int px{ int(topLeftX) }; px < bottomRightX; ++px)
			{
				for (int py{ int(bottomRightY) }; py < topLeftY; ++py)
				{
					Vector2 pixel{ float(px), float(py) };
					ColorRGB finalColor{ 0.0f, 0.0f, 0.0f };

					const float signedArea2{ Vector2::Cross(V01, Vector2{ pos0.GetXY(), pixel}) };
					const float signedArea0{ Vector2::Cross(V12, Vector2{ pos1.GetXY(), pixel}) };
					const float signedArea1{ Vector2::Cross(V20, Vector2{ pos2.GetXY(), pixel}) };
					const float doubleOfTriangleArea = signedArea2 + signedArea0 + signedArea1;

					if (signedArea2 > 0 && signedArea0 > 0 && signedArea1 > 0)
					{
						const float w2{ signedArea2 / doubleOfTriangleArea };
						const float w0{ signedArea0 / doubleOfTriangleArea };
						const float w1{ signedArea1 / doubleOfTriangleArea };

						const float interpolatedZ{ 1 / ((1 / pos0.z) * w0 + (1 / pos1.z) * w1 + (1 / pos2.z) * w2) };

						if (interpolatedZ >= 0.f && interpolatedZ <= 1.f && interpolatedZ < m_pDepthBufferPixels[px + (py * m_Width)])
						{
							m_pDepthBufferPixels[px + (py * m_Width)] = interpolatedZ;

							const float interpolatedW{ 1 / ((1 / pos0.w) * w0 + (1 / pos1.w) * w1 + (1 / pos2.w) * w2) };

							Vector2 interpolatedUV{
								mesh.vertices[index0].uv / pos0.w * w0
								+ mesh.vertices[index1].uv / pos1.w * w1
								+ mesh.vertices[index2].uv / pos2.w * w2 };

							interpolatedUV *= interpolatedW;

							if (interpolatedUV.x < 0.f) interpolatedUV.x = 0.f;
							if (interpolatedUV.y < 0.f) interpolatedUV.y = 0.f;
							if (interpolatedUV.x > 1.f) interpolatedUV.x = 1.f;
							if (interpolatedUV.y > 1.f) interpolatedUV.y = 1.f;

							if (ShowDepthBuffer)
							{
								float v = Remap(interpolatedZ, 0.995f, 1.f);
								finalColor = { v,v,v};
							}
							else
							{
								Vertex_Out v{};
								v.uv = interpolatedUV;

								v.normal = Interpolate(V0.normal, V1.normal, V2.normal , w0, w1,  w2);
								v.normal.Normalize();
								v.tangent = Interpolate(V0.tangent, V1.tangent, V2.tangent, w0 , w1, w2);
								v.tangent.Normalize();
								v.viewDirection = Interpolate(V0.viewDirection, V1.viewDirection, V2.viewDirection, w0, w1, w2);
								v.viewDirection.Normalize();


								finalColor = PixelShading(v);
							}

							//Update Color in Buffer
							finalColor.MaxToOne();

							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255),
								static_cast<uint8_t>(finalColor.g * 255),
								static_cast<uint8_t>(finalColor.b * 255));
						}
					}

				}
			}
		}
	}
}

float Renderer::Interpolate(float value0, float value1, float value2, float w0, float w1, float w2)
{
	//return 1 / ((1 / value0) * w0 + (1 / value1) * w1 + (1 / value2) * w2);
	return value0 * w0 + value1 * w1 + value2 * w2;
}

Vector3 Renderer::Interpolate(Vector3 value0, Vector3 value1, Vector3 value2, float w0, float w1, float w2)
{
	return
	{
		Interpolate(value0.x, value1.x, value2.x, w0,w1,w2),
		Interpolate(value0.y, value1.y, value2.y, w0,w1,w2),
		Interpolate(value0.z, value1.z, value2.z, w0,w1,w2)
	};
}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes) const
{

	for (auto& mesh : meshes)
	{
		auto postionMatrix = mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix;

		mesh.vertices_out.clear();
		mesh.vertices_out.reserve(mesh.vertices.size());

		for (const auto& vertex : mesh.vertices)
		{
			Vector4 screenSpaceVertex =  postionMatrix.TransformPoint(Vector4{ vertex.position, 1.f });

			//Perspective Divide (perspective distortion)
			screenSpaceVertex.x /= screenSpaceVertex.w;
			screenSpaceVertex.y /= screenSpaceVertex.w;
			screenSpaceVertex.z /= screenSpaceVertex.w;
						
			screenSpaceVertex.x = ((screenSpaceVertex.x + 1) / 2.f) * float(m_Width);
			screenSpaceVertex.y = ((1 - screenSpaceVertex.y) / 2.f) * float(m_Height);

			auto vertex_out = Vertex_Out{ screenSpaceVertex };

			vertex_out.normal = mesh.worldMatrix.TransformVector(vertex.normal).Normalized();
			vertex_out.tangent = mesh.worldMatrix.TransformVector(vertex.tangent).Normalized();
			vertex_out.viewDirection = mesh.worldMatrix.TransformPoint(vertex.position) - m_Camera.origin;
			mesh.vertices_out.emplace_back(vertex_out);
		}
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

ColorRGB Renderer::PixelShading(const Vertex_Out& v)
{
	const Vector3 lightDirection{.577f, -.577f, .577f };
	const ColorRGB ambient{ .025f, .025f, .025f };
	const float shininess{ 25.f };

	auto color = m_pTexture->Sample(v.uv);
	//auto color = ColorRGB{1,1,1}; 

	auto normalFromMap = m_pNormalMap->SampleNormal(v.uv);;

	Vector3 binormal{ Vector3::Cross(v.normal, v.tangent) };
	Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3::Zero };

	auto normal = tangentSpaceAxis.TransformVector(normalFromMap);

	const auto observedArea = Vector3::Dot(normal, -lightDirection);

	if (observedArea < 0)
	{
		return ColorRGB{};
	}

	auto glossiness =  m_pGlossMap->SampleFloat(v.uv);
	auto specular = m_pSpecularMap->Sample(v.uv);

	auto phong = BRDF::Phong(2.f, glossiness * shininess, lightDirection, -v.viewDirection, normal);
	ColorRGB phongClamped{ Clamp(phong.r, 0.f, 1.f), Clamp(phong.g, 0.f, 1.f) , Clamp(phong.b, 0.f, 1.f) };
	auto lambert = BRDF::Lambert(7.f, color);


	ColorRGB finalColor{};
	switch (m_LightMode)
	{
	case Renderer::LightMode::ObservedArea:
		finalColor = { observedArea, observedArea, observedArea };
		break;
	case Renderer::LightMode::Diffuse:
		finalColor = lambert * observedArea;
		break;
	case Renderer::LightMode::Specular:
		finalColor = phong;
		break;
	case Renderer::LightMode::Combined:
		finalColor = (lambert + phong + ambient) * observedArea;
		break;
	}
	return finalColor;
}
void Renderer::ToggleRotation()
{
	m_Rotating = !m_Rotating;
}
void Renderer::ToggleLightMode()
{
	if(int(m_LightMode) < 3)
	{
		m_LightMode = LightMode(int(m_LightMode) + 1);
	}
	else
	{
		m_LightMode = LightMode(0);
	}

}