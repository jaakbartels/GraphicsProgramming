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

void ShowKeyBindings()
{
	std::cout << "[Key Bindings - SHARED]\n";
	std::cout << "  [F1] Toggle Rasterizer Mode(HARDWARE / SOFTWARE)\n";
	std::cout << "  [F2]  Toggle Vehicle Rotation(ON / OFF)\n";
	std::cout << "  [F10] Toggle Uniform ClearColor(ON / OFF)\n";
	std::cout << "  [F11] Toggle Print FPS(ON / OFF)\n";
	std::cout << "\n";
	std::cout << "[Key Bindings - HARDWARE]\n";
	std::cout << "  [F3] Toggle FireFX(ON / OFF)\n";
	std::cout << "  [F4] Cycle Sampler State(POINT / LINEAR / ANISOTROPIC)\n";
	std::cout << "\n";
	std::cout << "[Key Bindings - SOFTWARE]\n";
	std::cout << "  [F5] Cycle Shading Mode(COMBINED / OBSERVED_AREA / DIFFUSE / SPECULAR)\n";
	std::cout << "  [F6] Toggle NormalMap(ON / OFF)\n";
	std::cout << "  [F7] Toggle DepthBuffer Visualization(ON / OFF)\n";
	std::cout << "  [F8] Toggle BoundingBox Visualization(ON / OFF)\n";
	std::cout << "\n";
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	ShowKeyBindings();

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