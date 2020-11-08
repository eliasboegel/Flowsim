#pragma once

#include "Settings.h"
#include "ElementaryFlows.h"
#include "Particle.h"

#include <vector>

class Simulator {
public:
	Simulator();
	~Simulator();

	float getStreamfunct(int x, int y);
	uint32_t getColor(int x, int y);
	std::vector<Particle>* getParticlesPtr();


	void updateField();
	void simulateParticles(float dt);
	
	
	void addParticle(float x, float y);
	void addUniform(float angle, float v);
	void addSourceSink(float x, float y, float strength);
	void addVortex(float x, float y, float strength);

private:
	std::vector<Particle> particles;
	std::vector<Uniform> uniforms;
	std::vector<SourceSink> sourcesinks;
	std::vector<Vortex> vortices;

	float v_freestream2 = 0;
	float v_stag2 = FLT_MAX;
	float streamfunct_stag = 0;

	float streamfunct_cache[GRID_SIZE_W+2][GRID_SIZE_W+2];
	float velocity_cache[GRID_SIZE_W][GRID_SIZE_H][2];
	uint32_t color_cache[GRID_SIZE_W][GRID_SIZE_H];

	float calcStreamfunct(float x, float y);
	uint32_t calcColor(int x, int y);

	void updateStreamfunctFragment(int index_start_x, int index_stop_x, int index_start_y, int index_stop_y);
	void updateVelColorFragment(int index_start_x, int index_stop_x, int index_start_y, int index_stop_y);
};