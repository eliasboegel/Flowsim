#pragma once
#include <iostream>
#include <math.h>

#define PI 3.14159265358979323846264

struct SourceSink {
	int centerx;
	int centery;
	float source_mag;

	SourceSink(int cx, int cy, float mag) {
		centerx = cx;
		centery = cy;
		source_mag = mag;
	}

	float calcStreamfunct(float x, float y) {
		float theta = atan((centerx - x) / (centery - y));
		return source_mag / (2 * PI) * theta;
	}
};

struct Vortex {
	int centerx;
	int centery;
	float vortex_mag;

	Vortex(int cx, int cy, float mag) {
	centerx = cx;
	centery = cy;
	vortex_mag = mag;
	}

	float calcStreamfunct(float x, float y) {
		float diff_x = centerx - x;
		float diff_y = centery - y;
		float r = sqrt(diff_x * diff_x + diff_y * diff_y);

		return vortex_mag / (2 * PI) * log(r);
	}
};

struct Uniform {
	float angle;
	float V;

	Uniform(float angle1, int mag) {
		angle = PI/float(180) * angle1;
		V = mag;
	}

	float calcStreamfunct(float x, float y) {
		return V * (x * sin(angle) - y * cos(angle));
	}

	float calcU(float x, float y) {
		return V * cos(angle);
	}

	float calcV(float x, float y) {
		return V * sin(angle);
	}
};