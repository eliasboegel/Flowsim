#pragma once

#include <SDL.h>
#include "Settings.h"
#include "Simulator.h"

#include <vector>


class SimApp {
public:
	SimApp();
	~SimApp();

	void mainLoop();

private:
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL; 
	SDL_Renderer* renderer = NULL;
	SDL_Surface* surface = NULL;
	SDL_Texture* texture = NULL;
	uint32_t time = 0;
	bool running = true;

	Simulator sim;

	void render();
	void renderNoParticles();

	void createFlowField();
	void createParticles();
};
