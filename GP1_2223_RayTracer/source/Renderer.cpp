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

using namespace dae;

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
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

            float ar{ float(m_Width * 1.f / m_Height) };
            float FOV = tanf(camera.fovAngle / 2 * TO_RADIANS);

            
            float cX{ (2.f * ((px + 0.5f) / m_Width) - 1.f) * ar * FOV };
            float cY{ (1.f - ((2.f * (py + 0.5f)) / m_Height)) * FOV };


            const Matrix cameraToWorld = camera.CalculateCameraToWorld();
            Vector3 rayDirection{ cX, cY, 1 };

            rayDirection = cX * camera.right + cY * camera.up + 1.0f * camera.forward;
			Vector3 normalRayDir{ rayDirection.Normalized() };


            Ray viewRay{ camera.origin, normalRayDir };

            ColorRGB finalColor{};

            HitRecord closestHit{};

            pScene->GetClosestHit(viewRay, closestHit);

            if (closestHit.didHit)
            {
				auto material{ materials[closestHit.materialIndex] };
	            for(unsigned long i{}; i < lights.size();++i)
	            {
					Vector3 directionToLight =  LightUtils::GetDirectionToLight(lights[i], closestHit.origin);
					float mag{ directionToLight.Magnitude() };
					directionToLight.Normalize();
					float lambertVal = Vector3::Dot(closestHit.normal, directionToLight);
					if(lambertVal >0.f)
					{
						finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * lambertVal * material->Shade(closestHit,-directionToLight,viewRay.direction);
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
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
