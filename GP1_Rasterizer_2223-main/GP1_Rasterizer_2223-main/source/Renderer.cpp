//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Triangle.h"
#include "Utils.h"

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

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

Triangle Renderer::NdcToScreenSpace(Triangle triangle_ndc) const
{
	Triangle result;
	for (int i=0; i<3; ++i)
	{
		result[i].x = (triangle_ndc[i].x + 1.f) / 2.f * m_Width;
		result[i].y = (1.f-triangle_ndc[i].y ) / 2.f * m_Height;
		result[i].z = triangle_ndc[i].z;
	}
	return result;
}

void Renderer::Render_W1_Part1()
{
	Triangle const triangle_ndc
	{
		{0.f, .5f, 1.f},
		{.5f, -.5f, 1.f},
		{-.5f, -.5f, 1.f}
	};

	std::vector<Triangle> triangles{ triangle_ndc };

	auto numTriangles = triangles.size();

	//RENDER LOGIC
	for (size_t t = 0; t < numTriangles; ++t) 
	{
		Triangle triangle_screen = NdcToScreenSpace(triangles[t]);

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				ColorRGB finalColor;

				if (triangle_screen.contains({float(px),float(py)}))
				{
					finalColor = { 1.f,1.f,1.f };
				}
				else
				{
					finalColor = { 0.f, 0.f, 0.f };
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

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	Render_W1_Part1();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
