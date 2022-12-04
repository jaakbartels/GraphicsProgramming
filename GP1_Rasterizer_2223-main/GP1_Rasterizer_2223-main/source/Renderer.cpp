//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>

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
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
	m_Camera.aspect = (m_Width / float(m_Height));
	//initialize Texture
	//m_pTexture = new Texture{ "Resources/uv_grid_2.png" };
	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");
	
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//Renderer_W1_01();
	//Renderer_W1_02();	
	//Renderer_W1_03();
	//Renderer_W1_04();
	//Renderer_W1_05();

	//Renderer_W2_01();
	//Renderer_W2_02();
	//Renderer_W2_03();

	Renderer_W3_01();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::Renderer_W3_01()
{
	std::vector<Mesh> meshes_world
	{
		Mesh{
				{
				Vertex{{-3, 3, -2}, {1,1,1}, {0,0}},
				Vertex{{0,3,-2}, {1,1,1}, {0.5,0}},
				Vertex{{3,3,-2}, {1,1,1}, {1,0}},
				Vertex{{-3,0,-2}, {1,1,1}, {0,0.5}},
				Vertex{{0,0,-2}, {1,1,1}, {0.5,0.5}},
				Vertex{{3,0,-2}, {1,1,1}, {1,0.5}},
				Vertex{{-3,-3,-2}, {1,1,1}, {0,1}},
				Vertex{{0,-3,-2}, {1,1,1}, {0.5,1}},
				Vertex{{3,-3,-2}, {1,1,1}, {1,1}}
		},
			{
				3,0,4,1,5,2,
				2,6,
				6,3,7,4,8,5
			},
			PrimitiveTopology::TriangleStrip
		}
	};

	//Clear backBuffer
	ColorRGB clearColor{ 100, 100, 100 };
	uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 8 | (uint32_t)clearColor.g << 16 | (uint32_t)clearColor.r;
	SDL_FillRect(m_pBackBuffer, NULL, hexColor);

	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	VertexTransformationFunction(meshes_world);

	for (int i{}; i < meshes_world[0].indices.size() - 2; ++i)
	{
		//Points of the Triangle
		const int index0{ int(meshes_world[0].indices[i]) };
		int index1{ int(meshes_world[0].indices[i + 1]) };
		int index2{ int(meshes_world[0].indices[i + 2]) };

		if (meshes_world[0].primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			if (i % 2 != 0)
			{
				std::swap(index1, index2);
			}
		}

		const Vector3 V0{ meshes_world[0].vertices_out[index0].position };
		const Vector3 V1{ meshes_world[0].vertices_out[index1].position };
		const Vector3 V2{ meshes_world[0].vertices_out[index2].position };

		float topLeftX = std::min(V0.x, std::min(V1.x, V2.x));
		float topLeftY = std::max(V0.y, std::max(V1.y, V2.y));
		float bottomRightX = std::max(V0.x, std::max(V1.x, V2.x));
		float bottomRightY = std::min(V0.y, std::min(V1.y, V2.y));


		topLeftX = Clamp(topLeftX, 1.f, float(m_Width - 1));
		topLeftY = Clamp(topLeftY, 1.f, float(m_Height - 1));
		bottomRightX = Clamp(bottomRightX, 1.f, float(m_Width - 1));
		bottomRightY = Clamp(bottomRightY, 1.f, float(m_Height - 1));

		//RENDER LOGIC
		for (int px{ int(topLeftX) }; px < bottomRightX; ++px)
		{
			for (int py{ int(bottomRightY) }; py < topLeftY; ++py)
			{
				Vector2 pixel{ float(px), float(py) };
				ColorRGB finalColor{ 0.0f, 0.0f, 0.0f };

				// Define the edges of the screen triangle
				const Vector2 V01{ V0.GetXY(), V1.GetXY() };
				const Vector2 V12{ V1.GetXY(), V2.GetXY() };
				const Vector2 V20{ V2.GetXY(), V0.GetXY() };

				const float signedArea2{ Vector2::Cross(V01, Vector2{ V0.GetXY(), pixel}) };
				const float signedArea0{ Vector2::Cross(V12, Vector2{ V1.GetXY(), pixel}) };
				const float signedArea1{ Vector2::Cross(V20, Vector2{ V2.GetXY(), pixel}) };
				const float doubleOfTriangleArea = signedArea2 + signedArea0 + signedArea1;


				if (signedArea2 > 0 && signedArea0 > 0 && signedArea1 > 0)
				{
					const float w2{ signedArea2 / doubleOfTriangleArea };
					const float w0{ signedArea0 / doubleOfTriangleArea };
					const float w1{ signedArea1 / doubleOfTriangleArea };

					const float interpolatedZ{ 1 / ((1 / V0.z) * w0 + (1 / V1.z) * w1 + (1 / V2.z) * w2) };
					//const float interpolatedW{ 1 / ((1 / V0.w) * w0 + (1 / V1.z) * w1 + (1 / V2.z) * w2) };

					Vector2 interpolatedUV{
						meshes_world[0].vertices[index0].uv / V0.z * w0
						+ meshes_world[0].vertices[index1].uv / V1.z * w1
						+ meshes_world[0].vertices[index2].uv / V2.z * w2 };

					interpolatedUV *= interpolatedZ;

					finalColor = { m_pTexture->Sample(interpolatedUV) };
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

void Renderer::VertexTransformationFunctionW1(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage

	for (int i{}; i < vertices_in.size(); ++i)
	{
		Vector3 p{};

		//world to view space
		p = m_Camera.viewMatrix.TransformPoint(vertices_in[i].position);

		//camera settings
		const float aspectRatio{ float(m_Width) / m_Height };
		p.x /= (aspectRatio * m_Camera.fov);
		p.y /= m_Camera.fov;

		//perspective divide
		if (p.z != 0)
		{
			p.x /= p.z;
			p.y /= p.z;
		}
		p.z = p.z;

		//to screen space
		p.x = (p.x + 1) / 2.f * m_Width;
		p.y = (1 - p.y) / 2.f * m_Height;
		p.z = p.z;

		//std::cout << p.x << ' ' << p.y << ' ' << p.z << '\n';
		vertices_out.push_back({ p, vertices_in[i].color });
	}
}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes) const
{
	auto matrix = m_Camera.viewMatrix * m_Camera.projectionMatrix;

	for (auto& mesh : meshes)
	{
		for (const auto& vertex : mesh.vertices)
		{
			Vector4 screenSpaceVertex =  matrix.TransformPoint(Vector4{ vertex.position, 1.f });

			//Perspective Divide (perspective distortion)
			screenSpaceVertex.x /= screenSpaceVertex.w;
			screenSpaceVertex.y /= screenSpaceVertex.w;
			screenSpaceVertex.z /= screenSpaceVertex.w;

			
			screenSpaceVertex.x = ((screenSpaceVertex.x + 1) / 2.f) * float(m_Width);
			screenSpaceVertex.y = ((1 - screenSpaceVertex.y) / 2.f) * float(m_Height);

			mesh.vertices_out.emplace_back(screenSpaceVertex);
		}
	}
}

bool Renderer::IsInTriangle(const Vector2& pixel, const Vector2& a, const Vector2& b, const Vector2& c)
{
	const Vector2 AP{ a - pixel };
	const Vector2 BP{ b - pixel };
	const Vector2 CP{ c - pixel };
	const Vector2 AB{ a - b };
	const Vector2 BC{ b - c };
	const Vector2 CA{ c - a };

	const float cross1{ Vector2::Cross(AP, AB) };
	const float cross2{ Vector2::Cross(BP, BC) };
	const float cross3{ Vector2::Cross(CP, CA) };

	if (cross1 < 0 && cross2 < 0 && cross3 < 0) return true;

	return false;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
