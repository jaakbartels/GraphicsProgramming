#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{ };
		float fovAngle{90.f};
		float pitch{ 0 };
		float yaw{ 0 };

		Vector3 forward{ Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		const float moveStep = .5f;
		const float rotateStep = 0.1f;
		const int mouseThreshold = 2;

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Vector4 t{ origin, 1.f };

			Matrix cam{ forward, up  ,right,t };
			return cam;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const Uint8* pStates = SDL_GetKeyboardState(nullptr);
			if(pStates[SDL_SCANCODE_W] || pStates[SDL_SCANCODE_UP])
			{
				origin += moveStep * forward;
			}
			if (pStates[SDL_SCANCODE_S] || pStates[SDL_SCANCODE_DOWN])
			{
				origin -= moveStep * forward;
			}
			if (pStates[SDL_SCANCODE_D] || pStates[SDL_SCANCODE_RIGHT])
			{
				origin += moveStep * right;
			}
			if (pStates[SDL_SCANCODE_A] || pStates[SDL_SCANCODE_LEFT])
			{
				origin -= moveStep * right;
			}
			if (pStates[SDL_SCANCODE_B]) //move camera to back 
			{
				origin = { .0f,1.f,4.f };
				yaw = PI;
				pitch = 0.f;
			}
			if (pStates[SDL_SCANCODE_F]) //move camera to front
			{
				origin = { .0f, 1.f, -5.f };
				yaw = 0;
				pitch = 0.f;
			}
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if (mouseState & SDL_BUTTON_LMASK && mouseState & SDL_BUTTON_RMASK )
			{
				if ( mouseY < -mouseThreshold)
				{
					origin -= moveStep * up;
				}
				else if ( mouseY > mouseThreshold)
				{
					origin += moveStep * up;
				}
			}
			else if(mouseState & SDL_BUTTON_LMASK)
			{
				if(mouseY < -mouseThreshold)
				{
					origin += moveStep * forward;
				}
				else if(mouseY > mouseThreshold)
				{
					origin -= moveStep * forward;
				}
				if (mouseX < -mouseThreshold)
				{
					yaw += rotateStep;
				}
				else if (mouseX > mouseThreshold)
				{
					yaw -= rotateStep;
				}
			}
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				if (mouseX < -mouseThreshold)
				{
					yaw += rotateStep;
				}
				else if (mouseX > mouseThreshold)
				{
					yaw -= rotateStep;
				}
				if (mouseY < -mouseThreshold)
				{
					pitch += rotateStep;
				}
				else if (mouseY > mouseThreshold)
				{
					pitch -= rotateStep;
				}
			}

			Matrix pitchRotation =  Matrix::CreateRotationX(pitch);
			Matrix yawRotation = Matrix::CreateRotationY(yaw);
			Matrix finalRotation = yawRotation * pitchRotation;
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			right = finalRotation.TransformVector(Vector3::UnitX);
			right.Normalize();
			up = finalRotation.TransformVector(Vector3::UnitY);
			up.Normalize();
		}
	};
}
