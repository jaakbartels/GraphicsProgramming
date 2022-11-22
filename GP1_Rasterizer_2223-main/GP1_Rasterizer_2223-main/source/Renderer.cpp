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
	Renderer_W2_03();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::Renderer_W1_01()
{
	//Initialize triangles
	std::vector<Vector3> vertices_ndc
	{
		{0.f, .5f, 1.f },
		{.5f, -.5f, 1.f},
		{-.5f, -.5f, 1.f},
	};

	std::vector<Vector2> vertices_screen{};
	for (auto p : vertices_ndc)
	{
		//to screen space
		vertices_screen.emplace_back(Vector2{ (p.x + 1) / 2.f * m_Width,
		(1 - p.y) / 2.f * m_Height });
	}


	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

			ColorRGB finalColor{};

			const Vector2 pixel{ float(px), float(py) };

			if (IsInTriangle(pixel, vertices_screen[0], vertices_screen[1], vertices_screen[2]))
			{
				finalColor = ColorRGB{ 1,1,1 };
				//CalculateColor(finalColor, pixel, m_PointsTris[0], m_PointsTris[1], m_PointsTris[2]);
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

void Renderer::Renderer_W1_02()
{
	//Initialize triangles
	std::vector<Vertex> vertices_world
	{
		{{0.f, 2.f, 0.f }},
		{{1.f, 0.f, 0.f} },
		{{ -1.f, 0.f, 0.f }},
	};

	std::vector<Vertex> vertices_screen{};
	VertexTransformationFunctionW1(vertices_world, vertices_screen);

	std::vector<Vector2> vertices_Triangle{
		{vertices_screen[0].position.x, vertices_screen[0].position.y},
	{vertices_screen[1].position.x, vertices_screen[1].position.y},
	{vertices_screen[2].position.x, vertices_screen[2].position.y} };

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

			ColorRGB finalColor{};

			const Vector2 pixel{ float(px), float(py) };

			if (IsInTriangle(pixel, vertices_Triangle[0], vertices_Triangle[1], vertices_Triangle[2]))
			{
				finalColor = ColorRGB{ 1,1,1 };
				//CalculateColor(finalColor, pixel, m_PointsTris[0], m_PointsTris[1], m_PointsTris[2]);
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

void Renderer::Renderer_W1_03()
{
	//Initialize triangles
	std::vector<Vertex> vertices_world
	{
		{{0.f, 4.f, 2.f }, {1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0} },
		{{-3.f, -2.f, 2.f}, {0, 0, 1}}
	};

	std::vector<Vertex> vertices_screen{};
	VertexTransformationFunctionW1(vertices_world, vertices_screen);

	std::vector<Vector2> vertices_Triangle{
		{vertices_screen[0].position.x, vertices_screen[0].position.y},
	{vertices_screen[1].position.x, vertices_screen[1].position.y},
	{vertices_screen[2].position.x, vertices_screen[2].position.y} };

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

			Vector2 pixel{ float(px), float(py) };

			// Define the edges of the screen triangle
			const Vector2 AB{ vertices_Triangle[0], vertices_Triangle[1] };
			const Vector2 BC{ vertices_Triangle[1], vertices_Triangle[2] };
			const Vector2 CA{ vertices_Triangle[2], vertices_Triangle[0] };

			const float signedAreaAB{ Vector2::Cross(AB, Vector2{ vertices_Triangle[0], pixel}) };
			const float signedAreaParallelogramBC{ Vector2::Cross(BC, Vector2{ vertices_Triangle[1], pixel}) };
			const float signedAreaParallelogramCA{ Vector2::Cross(CA, Vector2{ vertices_Triangle[2], pixel}) };
			const float triangleArea = signedAreaAB + signedAreaParallelogramBC + signedAreaParallelogramCA;

			ColorRGB finalColor{ 0.0f, 0.0f, 0.0f };
			if (signedAreaAB > 0 || signedAreaParallelogramBC > 0 || signedAreaParallelogramCA > 0)
			{
				const float weightA{ signedAreaAB / triangleArea };
				const float weightB{ signedAreaParallelogramBC / triangleArea };
				const float weightC{ signedAreaParallelogramCA / triangleArea };

				ColorRGB interpolatedColor{ vertices_screen[2].color * weightA + vertices_screen[0].color * weightB + vertices_screen[1].color * weightC };

				finalColor = { interpolatedColor };

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

void Renderer::Renderer_W1_04()
{
	//Initialize triangles
	std::vector<Vertex> vertices_world
	{
		//Triangle 0
		{{0.f, 2.f, 0.f }, {1, 0, 0}},
		{{1.5f, -1.f, 0.f}, {1, 0, 0} },
		{{-1.5f, -1.f, 0.f}, {1, 0, 0}},

		//Triangle 1
		{{0.f, 4.f, 2.f }, {1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0} },
		{{-3.f, -2.f, 2.f}, {0, 0, 1}}
	};

	std::vector<Vertex> vertices_screen{};
	VertexTransformationFunctionW1(vertices_world, vertices_screen);

	std::vector<Vector2> vertices_Triangle{};
	for (int i{}; i < vertices_screen.size(); ++i)
	{
		vertices_Triangle.push_back({ vertices_screen[i].position.x, vertices_screen[i].position.y });
	}

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			m_pDepthBufferPixels[px + py] = INFINITY;

			Vector2 pixel{ float(px), float(py) };
			ColorRGB finalColor{ 0.0f, 0.0f, 0.0f };

			for (int i{}; i < vertices_screen.size(); i += 3)
			{
				// Define the edges of the screen triangle
				const Vector2 AB{ vertices_Triangle[i], vertices_Triangle[i + 1] };
				const Vector2 BC{ vertices_Triangle[i + 1], vertices_Triangle[i + 2] };
				const Vector2 CA{ vertices_Triangle[i + 2], vertices_Triangle[i] };

				const float signedAreaAB{ Vector2::Cross(AB, Vector2{ vertices_Triangle[i], pixel}) };
				const float signedAreaParallelogramBC{ Vector2::Cross(BC, Vector2{ vertices_Triangle[i + 1], pixel}) };
				const float signedAreaParallelogramCA{ Vector2::Cross(CA, Vector2{ vertices_Triangle[i + 2], pixel}) };
				const float triangleArea = signedAreaAB + signedAreaParallelogramBC + signedAreaParallelogramCA;

				if (signedAreaAB > 0 && signedAreaParallelogramBC > 0 && signedAreaParallelogramCA > 0)
				{
					if (m_pDepthBufferPixels[px + py] > vertices_screen[i].position.z)
					{
						m_pDepthBufferPixels[px + py] = vertices_screen[i].position.z;

						const float W0{ signedAreaAB / triangleArea };
						const float W1{ signedAreaParallelogramBC / triangleArea };
						const float W2{ signedAreaParallelogramCA / triangleArea };

						ColorRGB interpolatedColor{ vertices_screen[i + 2].color * W0 + vertices_screen[i].color * W1 + vertices_screen[i + 1].color * W2 };

						finalColor = { interpolatedColor };
					}
				}
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

void Renderer::Renderer_W1_05()
{
	//Clear backBuffer
	ColorRGB clearColor{ 100, 100, 100 };
	uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 8 | (uint32_t)clearColor.g << 16 | (uint32_t)clearColor.r;
	SDL_FillRect(m_pBackBuffer, NULL, hexColor);

	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	//Initialize triangles
	std::vector<Vertex> vertices_world
	{
		//Triangle 0
		{{0.f, 2.f, 0.f }, {1, 0, 0}},
		{{1.5f, -1.f, 0.f}, {1, 0, 0} },
		{{-1.5f, -1.f, 0.f}, {1, 0, 0}},

		//Triangle 1
		{{0.f, 4.f, 2.f }, {1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0} },
		{{-3.f, -2.f, 2.f}, {0, 0, 1}}
	};

	std::vector<Vertex> vertices_screen{};
	VertexTransformationFunctionW1(vertices_world, vertices_screen);

	for (int j{}; j < vertices_screen.size(); j += 3)
	{
		//Points of the Triangle
		const Vector3 A{ vertices_screen[j + 0].position };
		const Vector3 B{ vertices_screen[j + 1].position };
		const Vector3 C{ vertices_screen[j + 2].position };

		float topLeftX = std::min(A.x, std::min(B.x, C.x));
		float topLeftY = std::max(A.y, std::max(B.y, C.y));
		float bottomRightX = std::max(A.x, std::max(B.x, C.x));
		float bottomRightY = std::min(A.y, std::min(B.y, C.y));

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
				const Vector2 AB{ A.GetXY(), B.GetXY() };
				const Vector2 BC{ B.GetXY(), C.GetXY() };
				const Vector2 CA{ C.GetXY(), A.GetXY() };

				const float signedAreaAB{ Vector2::Cross(AB, Vector2{ A.GetXY(), pixel}) };
				const float signedAreaParallelogramBC{ Vector2::Cross(BC, Vector2{ B.GetXY(), pixel}) };
				const float signedAreaParallelogramCA{ Vector2::Cross(CA, Vector2{ C.GetXY(), pixel}) };
				const float triangleArea = signedAreaAB + signedAreaParallelogramBC + signedAreaParallelogramCA;


				if (signedAreaAB > 0 && signedAreaParallelogramBC > 0 && signedAreaParallelogramCA > 0)
				{
					const float W0{ signedAreaAB / triangleArea };
					const float W1{ signedAreaParallelogramBC / triangleArea };
					const float W2{ signedAreaParallelogramCA / triangleArea };

					float currentDepth = (A.z * W0) + (B.z * W1) + (C.z * W2);

					// Check the depth buffer
					if (currentDepth > m_pDepthBufferPixels[px + (py * m_Width)])
						continue;

					m_pDepthBufferPixels[px + (py * m_Width)] = currentDepth;

					ColorRGB interpolatedColor{ vertices_screen[j].color * W0 + vertices_screen[j + 1].color * W1 + vertices_screen[j + 2].color * W2 };

					finalColor = { interpolatedColor };
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

void Renderer::Renderer_W2_01()
{
	std::vector<Mesh> meshes_world
	{
		Mesh{
				{
				Vertex{{-3, 3, -2}},
				Vertex{{0,3,-2}},
				Vertex{{3,3,-2}},
				Vertex{{-3,0,-2}},
				Vertex{{0,0,-2}},
				Vertex{{3,0,-2}},
				Vertex{{-3,-3,-2}},
				Vertex{{0,-3,-2}},
				Vertex{{3,-3,-2}}
		},
			{
				3,0,1,	1,4,3,	4,1,2,
				2,5,4,	6,3,4,	4,7,6,
				7,4,5,	5,8,7
			},
			PrimitiveTopology::TriangeList
		}
	};

	//Clear backBuffer
	ColorRGB clearColor{ 100, 100, 100 };
	uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 8 | (uint32_t)clearColor.g << 16 | (uint32_t)clearColor.r;
	SDL_FillRect(m_pBackBuffer, NULL, hexColor);

	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	
	VertexTransformationFunction(meshes_world);

	for (int i{}; i < meshes_world[0].indices.size(); i += 3)
	{
		//Points of the Triangle
		const int indexA{ int(meshes_world[0].indices[i]) };
		const int indexB{ int(meshes_world[0].indices[i + 1]) };
		const int indexC{ int(meshes_world[0].indices[i + 2]) };
		const Vector3 A{ meshes_world[0].vertices_out[indexA].position};
		const Vector3 B{ meshes_world[0].vertices_out[indexB].position };
		const Vector3 C{ meshes_world[0].vertices_out[indexC].position };

		float topLeftX = std::min(A.x, std::min(B.x, C.x));
		float topLeftY = std::max(A.y, std::max(B.y, C.y));
		float bottomRightX = std::max(A.x, std::max(B.x, C.x));
		float bottomRightY = std::min(A.y, std::min(B.y, C.y));

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
				const Vector2 AB{ A.GetXY(), B.GetXY() };
				const Vector2 BC{ B.GetXY(), C.GetXY() };
				const Vector2 CA{ C.GetXY(), A.GetXY() };

				const float signedAreaAB{ Vector2::Cross(AB, Vector2{ A.GetXY(), pixel}) };
				const float signedAreaParallelogramBC{ Vector2::Cross(BC, Vector2{ B.GetXY(), pixel}) };
				const float signedAreaParallelogramCA{ Vector2::Cross(CA, Vector2{ C.GetXY(), pixel}) };
				const float triangleArea = signedAreaAB + signedAreaParallelogramBC + signedAreaParallelogramCA;


				if (signedAreaAB > 0 && signedAreaParallelogramBC > 0 && signedAreaParallelogramCA > 0)
				{
					const float W0{ signedAreaAB / triangleArea };
					const float W1{ signedAreaParallelogramBC / triangleArea };
					const float W2{ signedAreaParallelogramCA / triangleArea };

					//float currentDepth = (A.z * W0) + (B.z * W1) + (C.z * W2);

					// Check the depth buffer
					/*if (currentDepth > m_pDepthBufferPixels[px + (py * m_Width)])
						continue;*/

						/*m_pDepthBufferPixels[px + (py * m_Width)] = currentDepth;*/

					ColorRGB interpolatedColor{ meshes_world[0].vertices[indexA].color * W0 +
						meshes_world[0].vertices[indexB].color* W1 + meshes_world[0].vertices[indexC].color * W2 };

					finalColor = interpolatedColor;
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

void Renderer::Renderer_W2_02()
{
	std::vector<Mesh> meshes_world
	{
		Mesh{
				{
				Vertex{{-3, 3, -2}},
				Vertex{{0,3,-2}},
				Vertex{{3,3,-2}},
				Vertex{{-3,0,-2}},
				Vertex{{0,0,-2}},
				Vertex{{3,0,-2}},
				Vertex{{-3,-3,-2}},
				Vertex{{0,-3,-2}},
				Vertex{{3,-3,-2}}
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
		const int indexA{ int(meshes_world[0].indices[i]) };
		int indexB		{ int(meshes_world[0].indices[i + 1]) };
		int indexC		{ int(meshes_world[0].indices[i + 2]) };

		if (meshes_world[0].primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			if (i % 2 != 0)
			{
				std::swap(indexB, indexC);
			}
		}

		const Vector3 A{ meshes_world[0].vertices_out[indexA].position };
		const Vector3 B{ meshes_world[0].vertices_out[indexB].position };
		const Vector3 C{ meshes_world[0].vertices_out[indexC].position };

		float topLeftX = std::min(A.x, std::min(B.x, C.x));
		float topLeftY = std::max(A.y, std::max(B.y, C.y));
		float bottomRightX = std::max(A.x, std::max(B.x, C.x));
		float bottomRightY = std::min(A.y, std::min(B.y, C.y));

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
				const Vector2 AB{ A.GetXY(), B.GetXY() };
				const Vector2 BC{ B.GetXY(), C.GetXY() };
				const Vector2 CA{ C.GetXY(), A.GetXY() };

				const float signedAreaAB{ Vector2::Cross(AB, Vector2{ A.GetXY(), pixel}) };
				const float signedAreaParallelogramBC{ Vector2::Cross(BC, Vector2{ B.GetXY(), pixel}) };
				const float signedAreaParallelogramCA{ Vector2::Cross(CA, Vector2{ C.GetXY(), pixel}) };
				const float triangleArea = signedAreaAB + signedAreaParallelogramBC + signedAreaParallelogramCA;


				if (signedAreaAB > 0 && signedAreaParallelogramBC > 0 && signedAreaParallelogramCA > 0)
				{
					const float W0{ signedAreaAB / triangleArea };
					const float W1{ signedAreaParallelogramBC / triangleArea };
					const float W2{ signedAreaParallelogramCA / triangleArea };

					//float currentDepth = (A.z * W0) + (B.z * W1) + (C.z * W2);

					// Check the depth buffer
					/*if (currentDepth > m_pDepthBufferPixels[px + (py * m_Width)])
						continue;*/

						/*m_pDepthBufferPixels[px + (py * m_Width)] = currentDepth;*/

					ColorRGB interpolatedColor{ meshes_world[0].vertices[indexA].color * W0 +
						meshes_world[0].vertices[indexB].color * W1 + meshes_world[0].vertices[indexC].color * W2 };

					finalColor = { 1,1,1 };
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

void Renderer::Renderer_W2_03()
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
		const int indexA{ int(meshes_world[0].indices[i]) };
		int indexB{ int(meshes_world[0].indices[i + 1]) };
		int indexC{ int(meshes_world[0].indices[i + 2]) };

		if (meshes_world[0].primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			if (i % 2 != 0)
			{
				std::swap(indexB, indexC);
			}
		}

		const Vector3 A{meshes_world[0].vertices_out[indexA].position };
		const Vector3 B{meshes_world[0].vertices_out[indexB].position};
		const Vector3 C{meshes_world[0].vertices_out[indexC].position};

		float topLeftX = std::min(A.x, std::min(B.x, C.x));
		float topLeftY = std::max(A.y, std::max(B.y, C.y));
		float bottomRightX = std::max(A.x, std::max(B.x, C.x));
		float bottomRightY = std::min(A.y, std::min(B.y, C.y));

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
				const Vector2 AB{ A.GetXY(), B.GetXY() };
				const Vector2 BC{ B.GetXY(), C.GetXY() };
				const Vector2 CA{ C.GetXY(), A.GetXY() };

				const float signedAreaAB{ Vector2::Cross(AB, Vector2{ A.GetXY(), pixel}) };
				const float signedAreaParallelogramBC{ Vector2::Cross(BC, Vector2{ B.GetXY(), pixel}) };
				const float signedAreaParallelogramCA{ Vector2::Cross(CA, Vector2{ C.GetXY(), pixel}) };
				const float triangleArea = signedAreaAB + signedAreaParallelogramBC + signedAreaParallelogramCA;


				if (signedAreaAB > 0 && signedAreaParallelogramBC > 0 && signedAreaParallelogramCA > 0)
				{
					const float W0{ signedAreaAB / triangleArea };
					const float W1{ signedAreaParallelogramBC / triangleArea };
					const float W2{ signedAreaParallelogramCA / triangleArea };

					Vector2 interpolatedUV{ meshes_world[0].vertices[indexA].uv * W0 +
						 meshes_world[0].vertices[indexB].uv * W1 + meshes_world[0].vertices[indexC].uv * W2 };
					const float finalDepth{ 1 / ((1 / A.z) * W0 + (1 / B.z) * W1 + (1 / C.z) * W2) };

					
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
	for (auto& mesh : meshes)
	{
		for (const auto& vertex : mesh.vertices)
		{
			//World Space to View Space
			Vector3 viewSpaceVertex = m_Camera.viewMatrix.TransformPoint(vertex.position);

			//Conversion to NDC - Perspective Devide (perspective distorion)
			Vector3 projectedVertex{};
			projectedVertex.x = viewSpaceVertex.x / viewSpaceVertex.z;
			projectedVertex.y = viewSpaceVertex.y / viewSpaceVertex.z;
			projectedVertex.z = viewSpaceVertex.z;

			//With camera setting (screens aren't always squares)
			const auto aspectRatio = m_Width / float(m_Height);
			projectedVertex.x = projectedVertex.x / (aspectRatio * m_Camera.fov);
			projectedVertex.y = projectedVertex.y / m_Camera.fov;

			//Convert to Screen Space
			Vertex_Out screenSpaceVertex{};
			screenSpaceVertex.position.x = ((projectedVertex.x + 1) / 2.f) * float(m_Width);
			screenSpaceVertex.position.y = ((1 - projectedVertex.y) / 2.f) * float(m_Height);

			screenSpaceVertex.color = vertex.color; //Copy the color

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
