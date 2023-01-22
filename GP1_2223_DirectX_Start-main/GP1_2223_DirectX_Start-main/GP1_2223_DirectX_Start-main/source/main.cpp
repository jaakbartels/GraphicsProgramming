#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRasterizer - Xander Bartels, 2DAE07",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool printFPS = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				//if (e.key.keysym.scancode == SDL_SCANCODE_X)
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					if (pRenderer) pRenderer->ToggleRenderFunction();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					if (pRenderer) pRenderer->ToggleRotation();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					if (pRenderer) pRenderer->ToggleShowFireFX();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					if (pRenderer) pRenderer->CycleFilteringMethods();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					if (pRenderer) pRenderer->CycleShadingMode();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					if (pRenderer) pRenderer->ToggleUseNormalMap();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					if (pRenderer) pRenderer->ToggleShowDepthBufer();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					if (pRenderer) pRenderer->ToggleShowBoundingBoxes();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					if (pRenderer) pRenderer->ToggleUniformColor();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					printFPS = !printFPS;
					std::cout << "Print FPS " << (printFPS ? "ON" : "OFF") << "\n";
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printFPS && printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}