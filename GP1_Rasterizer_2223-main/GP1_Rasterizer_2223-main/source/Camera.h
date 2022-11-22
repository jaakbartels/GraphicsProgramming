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

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			invViewMatrix = Matrix{ {right, 0}, {up, 0},
				{forward, 0}, {origin, 1} };


			//Inverse(ONB) => ViewMatrix
			//viewMatrix = Matrix::Inverse(invViewMatrix);
			/*viewMatrix = Matrix{
				{right.x, right.y, right.z, 0 },
				{up.x, up.y, up.z, 0 },
				{forward.x, forward.y, forward.z, 0 },
				{origin.x, origin.y, origin.z, 1 }
			};*/

			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);

			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			const float movementSpeed{ 10 * deltaTime };
			//Keyboard Input
			//const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			const Uint8* pStates{ SDL_GetKeyboardState(nullptr) };
			if (pStates[SDL_SCANCODE_W] || pStates[SDL_SCANCODE_UP])
			{
				origin.z += movementSpeed;
			}
			else if (pStates[SDL_SCANCODE_S] || pStates[SDL_SCANCODE_DOWN])
			{
				origin.z -= movementSpeed;
			}
			else if (pStates[SDL_SCANCODE_D] || pStates[SDL_SCANCODE_RIGHT])
			{
				origin.x += movementSpeed;
			}
			else if (pStates[SDL_SCANCODE_A] || pStates[SDL_SCANCODE_LEFT])
			{
				origin.x -= movementSpeed;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			const float rotationSpeed{ 0.1f * deltaTime };

			if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)))
			{
				origin.y += -mouseY * movementSpeed;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				totalPitch += -mouseY * rotationSpeed;
				totalYaw += mouseX * rotationSpeed;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				totalYaw += mouseX * rotationSpeed;

				origin.z += -mouseY * movementSpeed;
			}

			const Matrix finalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
