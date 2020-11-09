#include "Simulator.h"

#include <thread>
#include <algorithm>

Simulator::Simulator() {

}

Simulator::~Simulator() {
	uniforms.clear();
	sourcesinks.clear();
	vortices.clear();
}

void Simulator::simulateParticles(float dt) {
	for (auto& particle : particles) {
		int x = static_cast<int>(particle.x);
		int y = static_cast<int>(particle.y);

		float v_x = velocity_cache[x][y][0];
		float v_y = velocity_cache[x][y][1];

		particle.x = particle.x + v_x * dt;
		particle.y = particle.y + v_y * dt;

		if (particle.x < 0) {
			particle.x = SCREEN_W;
			particle.y = static_cast<float>(rand() * SCREEN_H) / RAND_MAX;
		}
		else if (particle.x > SCREEN_W) {
			particle.x = 0;
			particle.y = static_cast<float>(rand() * SCREEN_H) / RAND_MAX;
		}
		if (particle.y < 0) {
			particle.x = static_cast<float>(rand() * SCREEN_W) / RAND_MAX;
			particle.y = SCREEN_H;
		}
		else if (particle.y > SCREEN_H) {
			particle.x = static_cast<float>(rand() * SCREEN_W) / RAND_MAX;
			particle.y = 0;
		}

		

		x = static_cast<int>(particle.x);
		y = static_cast<int>(particle.y);

		particle.col = color_cache[x][y];
	}
}

float Simulator::getStreamfunct(int x, int y) {
	return streamfunct_cache[x][y];
}

uint32_t Simulator::getColor(int x, int y) {
	return color_cache[x][y];
}

std::vector<Particle>* Simulator::getParticlesPtr() {
	return &particles;
}





void Simulator::updateField() {
	int interval_width_y = SCREEN_H / THREAD_NUM;
	std::vector<std::thread> threads;

	// Calculate streamfunction on border
	for (int i = 0; i < GRID_SIZE_W + 2; i++) {
		streamfunct_cache[i][0] = calcStreamfunct(i, 0);
		streamfunct_cache[i][GRID_SIZE_H + 2] = calcStreamfunct(i, GRID_SIZE_H + 2);
	}
	for (int i = 1; i < GRID_SIZE_H + 1; i++) {
		streamfunct_cache[0][i] = calcStreamfunct(0, i);
		streamfunct_cache[GRID_SIZE_W + 2][i] = calcStreamfunct(GRID_SIZE_W + 2, i);
	}

	// Split rest of the screen up in intervals and dispatch thread for each
	// Update Streamfunction field
	for (int i = 0; i < THREAD_NUM; i++) {
		threads.push_back(std::thread(&Simulator::updateStreamfunctFragment, this, 0, SCREEN_W, i * interval_width_y, (i + 1) * interval_width_y));
	}
	for (auto& t : threads) {
		t.join();
	}
	threads.clear();

	// Update Velocity and color field
	for (int i = 0; i < THREAD_NUM; i++) {
		threads.push_back(std::thread(&Simulator::updateVelColorFragment, this, 0, SCREEN_W, i * interval_width_y, (i + 1) * interval_width_y));
	}
	for (auto& t : threads) {
		t.join();
	}
	threads.clear();
}

void Simulator::updateStreamfunctFragment(int index_start_x, int index_stop_x, int index_start_y, int index_stop_y) {
	for (int i = index_start_x; i < index_stop_x; i++) {
		for (int j = index_start_y; j < index_stop_y; j++) {
			// Calculate stream function value and cache it
			streamfunct_cache[i + 1][j + 1] = calcStreamfunct(i, j);
		}
	}
}

void Simulator::updateVelColorFragment(int index_start_x, int index_stop_x, int index_start_y, int index_stop_y) {
	for (int i = index_start_x; i < index_stop_x; i++) {
		for (int j = index_start_y; j < index_stop_y; j++) {
			// Find u from streamfunction cache and cache it
			velocity_cache[i][j][0] = streamfunct_cache[i][j] - streamfunct_cache[i][j + 2];

			// Find v from streamfunction cache and cache it
			velocity_cache[i][j][1] = streamfunct_cache[i + 2][j] - streamfunct_cache[i][j];

			float v_x = velocity_cache[i][j][0];
			float v_y = velocity_cache[i][j][1];
			float v2 = v_x * v_x + v_y * v_y;

			if (v2 < v_stag2) {
				streamfunct_stag = streamfunct_cache[i + 1][j + 1];
				v_stag2 = v2;
			}
		}
	}

	for (int i = index_start_x; i < index_stop_x; i++) {
		for (int j = index_start_y; j < index_stop_y; j++) {
			// Find color from velocity field and cache it

			#if DRAW_MODE == 1
				color_cache[i][j] = calcColor(i, j);
			#endif

			#if DRAW_MODE == 2
			if (abs(fmod(streamfunct_cache[i + 1][j + 1], 5000)) < 500) {
				color_cache[i][j] = (MATERIAL_R << 24) | (MATERIAL_G << 16) | (MATERIAL_B << 8) | 255;
			}
			else {
				color_cache[i][j] = calcColor(i, j);
			}
			#endif

			#if DRAW_MODE == 3
			color_cache[i][j] = calcColor(i, j);
			#endif

		}
	}
}

float Simulator::calcStreamfunct(float x, float y) {
	float phi = 0;

	for (auto& uniform : uniforms) {
		phi += uniform.calcStreamfunct(x, y);
	}
	for (auto& sourcesink : sourcesinks) {
		phi += sourcesink.calcStreamfunct(x, y);
	}
	for (auto& vortex : vortices) {
		phi += vortex.calcStreamfunct(x, y);
	}

	return phi;
}

uint32_t Simulator::calcColor(int x, int y) {
	float v_x = velocity_cache[x][y][0];
	float v_y = velocity_cache[x][y][1];

	float diff2 = (v_x * v_x + v_y * v_y) - v_freestream2;
	float diff2norm = diff2 / (v_freestream2)+1;

	float p_mix = atanf(0.71633527 * diff2norm) * 4.96 / (exp(diff2norm) + exp(-diff2norm));
	
	uint8_t r, g, b;
	uint8_t a = 255;

	if (signbit(diff2)) {
		// High press
		r = p_mix * MID_P_R + (1 - p_mix) * HIGH_P_R;
		g = p_mix * MID_P_G + (1 - p_mix) * HIGH_P_G;
		b = p_mix * MID_P_B + (1 - p_mix) * HIGH_P_B;
	}
	else {
		// Low press
		r = p_mix * MID_P_R + (1 - p_mix) * LOW_P_R;
		g = p_mix * MID_P_G + (1 - p_mix) * LOW_P_G;
		b = p_mix * MID_P_B + (1 - p_mix) * LOW_P_B;
	}

	return (r << 24) | (g << 16) | (b << 8) | a;
}





void Simulator::addParticle(float x, float y) {
	Particle new_particle(x, y);
	particles.push_back(new_particle);
}

void Simulator::addUniform(float angle, float v) {
	Uniform new_uniform(angle, v);
	uniforms.push_back(new_uniform);

	float v_freestream_x = 0;
	float v_freestream_y = 0;
	
	for (auto& uniform : uniforms) {
		v_freestream_x += uniform.calcU(0,0);
		v_freestream_x += uniform.calcU(0,0);
	}

	v_freestream2 = v_freestream_x * v_freestream_x + v_freestream_y * v_freestream_y;
}

void Simulator::addSourceSink(float x, float y, float strength) {
	SourceSink new_sourcesink(x, y, strength);
	sourcesinks.push_back(new_sourcesink);
}

void Simulator::addVortex(float x, float y, float strength) {
	Vortex new_vortex(x, y, strength);
	vortices.push_back(new_vortex);
}