#include <SDL.h>

struct Particle {
	float x = 0;
	float y = 0;
	uint32_t col;

	Particle(float x_pos, float y_pos) {
		x = x_pos;
		y = y_pos;

		col = UINT32_MAX;
	};
};

