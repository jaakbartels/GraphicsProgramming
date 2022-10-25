//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <iostream>

#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <future> //async
#include <ppl.h>
using namespace dae;


#define ASYNC
//#define PARALLEL_FOR

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Render(pScene, 0, m_Width, 0, m_Height);
}

void Renderer::Render(Scene* pScene, const int fromX, const int toX, const int fromY, const int toY) const
{
	Camera& camera = pScene->GetCamera();
	camera.CalculateCameraToWorld();
	float ar{ float(m_Width * 1.f / m_Height) };
	float FOV = tanf(camera.fovAngle / 2 * TO_RADIANS);

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t numPix = m_Width * m_Height;
#if defined(ASYNC)
	//async exe

	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPielsPertask = numPix / numCores;
	uint32_t numUnassignedPixels = numPix % numCores;
	uint32_t currentPix{ 0 };

	for (uint32_t coreId{0}; coreId < numCores; ++coreId)
	{
		uint32_t taskSize = numPielsPertask;
		if(numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}
		async_futures.push_back(
			std::async(std::launch::async,[=,this]
			{
					const uint32_t pixelIndexEnd = currentPix + taskSize;
				for(uint32_t pixelindex{currentPix}; pixelindex< pixelIndexEnd;++pixelindex)
				{
					RenderPixel(pScene, pixelindex, FOV, ar, camera, lights, materials);
				}
			})
		);
		currentPix += taskSize;
	}
	for(const std::future<void>& f : async_futures)
	{
		f.wait();
	}
#elif defined(PARALLEL_FOR)
	
	concurrency::parallel_for(0u, numPix, [=, this](int i)
		{
			RenderPixel(pScene, i, FOV, ar, camera, lights, materials);
		});
#else
	//synchronous
	for (uint32_t i{}; i< numPix;++i)
	{
		RenderPixel(pScene, i, FOV, ar, camera, lights, materials);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pscene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera,
	const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	const float rx = px + 0.5f;
	const float ry = py + 0.5f;

	const float cX{ 2 * (rx / float(m_Width) - 1) * aspectRatio * fov };
	const float cY{ 1 - (2 * (ry / float(m_Height))) * fov };

	
	Vector3 rayDirection{ cX, cY, 1 };

	rayDirection = cX * camera.right + cY * camera.up + 1.0f * camera.forward;
	Vector3 normalRayDir{ rayDirection.Normalized() };


	Ray viewRay{ camera.origin, normalRayDir };

	ColorRGB finalColor{};

	HitRecord closestHit{};

	pscene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		auto material{ materials[closestHit.materialIndex] };
		for (unsigned long i{}; i < lights.size(); ++i)
		{
			Vector3 directionToLight = LightUtils::GetDirectionToLight(lights[i], closestHit.origin);
			float mag{ directionToLight.Magnitude() };
			directionToLight.Normalize();
			float observedArea = Vector3::Dot(closestHit.normal, directionToLight);
			Ray rayToLight = Ray{ closestHit.origin,directionToLight,0.0001f,mag };
			if (observedArea >= 0.f && (!m_RenderShadows || !pscene->DoesHit(rayToLight)))
			{
				ColorRGB radiance = LightUtils::GetRadiance(lights[i], closestHit.origin);
				ColorRGB BRDF = material->Shade(closestHit, directionToLight, -viewRay.direction);

				switch (m_currentLightingMode)
				{
				case LightingMode::ObservedArea:
					finalColor += ColorRGB(1.f, 1.f, 1.f) * observedArea;
					break;
				case LightingMode::Radiance:
					finalColor += radiance;
					break;
				case LightingMode::BRDF:
					finalColor += BRDF;
					break;
				case LightingMode::Combined:
					finalColor += radiance * observedArea * BRDF;
					break;
				}
			}
		}
	}

	finalColor.MaxToOne();
	//Update Color in Buffer
	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	switch (m_currentLightingMode)
	{
		case LightingMode::ObservedArea:
			m_currentLightingMode = LightingMode::Radiance;
			break;
		case LightingMode::Radiance: 
			m_currentLightingMode = LightingMode::BRDF;
			break;
		case LightingMode::BRDF: 
			m_currentLightingMode = LightingMode::Combined;
			break;
		case LightingMode::Combined: 
			m_currentLightingMode = LightingMode::ObservedArea;
			break;
		default: ;
	}
}

