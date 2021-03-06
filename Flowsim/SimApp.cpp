#include "SimApp.h"
#include <iostream>
#include <math.h>
#include <stdlib.h> 
#include <cmath>
#include <algorithm>

SimApp::SimApp() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error("Could not initialize SDL2!");
	}

	SDL_CreateWindowAndRenderer(SCREEN_W, SCREEN_H, 0, &window, &renderer);
	if (window == NULL || renderer == NULL) {
		throw std::runtime_error("Renderer or Window could not be created!");
	}

	createParticles();
	createFlowField();

	sim.updateField();
}

SimApp::~SimApp() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}


void SimApp::createFlowField() {
	
	sim.addUniform(0, 500);
	//sim.addSourceSink(630, 360, 180000);
	//sim.addSourceSink(650, 360, -180000);
	//sim.addVortex(640, 360, 50000);*/


	
	// Some airfoil based on wikipedia NACA 5 digit camber line function
	int num_steps = 33;
	float lift_mult = 50000;
	float tip_pos_x = 300;
	float tip_pos_y = 360;
	float chord = 400;
	float m = 0.2025;
	float k = 16;
	float stepsize = chord / num_steps;
	for (int i = 0; i < num_steps; i++) {
		float z = 0;
		float x = float(i) / num_steps;
		if (x < m) {
			z = k / 6 * (x*x*x-3*m*x*x+m*m*(3-m)*x);
		}
		else {
			z = k * m * m * m / 6 * (1 - x);
		}

		float thickness = -0.000888888888889 * x * x + 0.2666666666666667 * x;

		z = 3 * z;

		sim.addVortex(tip_pos_x + x * chord, tip_pos_y - z * chord + 0.5 * thickness, lift_mult / float(num_steps));
		sim.addVortex(tip_pos_x + x * chord, tip_pos_y - z * chord - 0.5 * thickness, lift_mult / float(num_steps));
	}
}


void SimApp::createParticles() {
	srand(1);
	int particle_count = 100;

	
	// Random particle start
	for (int j = 0; j < particle_count; j++) {
		float rnd_x = static_cast<float>(rand() * SCREEN_W) / RAND_MAX;
		float rnd_y = static_cast<float>(rand() * SCREEN_H) / RAND_MAX;

		sim.addParticle(rnd_x, rnd_y);
	}
}					   			



void SimApp::render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	const uint32_t bitmask_r = 255 << 24;
	const uint32_t bitmask_g = 255 << 16;
	const uint32_t bitmask_b = 255 << 8;

	auto particles = *sim.getParticlesPtr();

	for (auto& particle : particles) {
		uint8_t r = (particle.col >> 24) & 255;
		uint8_t g = (particle.col >> 16) & 255;
		uint8_t b = (particle.col >> 8) & 255;

		SDL_SetRenderDrawColor(renderer, r, g, b, 255);
		SDL_RenderDrawPoint(renderer, int(particle.x), int(particle.y));
	}

	SDL_RenderPresent(renderer);
	SDL_UpdateWindowSurface(window);
}

void SimApp::renderNoParticles() {
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);

	int32_t pitch = 0;

	uint32_t* pPixelBuffer = nullptr;
	if (!SDL_LockTexture(texture, NULL, (void**)&pPixelBuffer, &pitch))
	{
		pitch /= sizeof(uint32_t);
		
		for (uint32_t i = 0; i < SCREEN_W * SCREEN_H; ++i) {

			int x = i % SCREEN_W + 1;
			int y = i / SCREEN_W + 1;

			pPixelBuffer[i] = sim.getColor(x, y);
		}
		
		SDL_UnlockTexture(texture);

		
		SDL_RenderCopy(renderer, texture, NULL, NULL);

		SDL_RenderPresent(renderer);

		SDL_UpdateWindowSurface(window);
	}
}

void SimApp::mainLoop() {

	//renderNoParticles();

	while (running) {
		uint32_t currentTime = SDL_GetTicks();
		float dt = float((currentTime - time)) / 1000;

		int mouse_x, mouse_y;

		SDL_PumpEvents();
		if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			for (int i = 0; i < 8; i++) {
				sim.addParticle(mouse_x+pow(-1,i) * (rand()%5), mouse_y + pow(-1, i) * (rand() % 5));
			}
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		

		
		if (dt > (1 / FPS)) {
			#if DRAW_MODE == 3
			sim.simulateParticles(dt/10);
			render();
			#else
			renderNoParticles();
			#endif

			time = currentTime;
		}
	}
}