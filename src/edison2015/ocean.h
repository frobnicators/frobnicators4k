#pragma once

#include "frob_math.h"
#include "shader.h"

extern vec2 ocean_wind_speed;
extern float ocean_A;
extern float ocean_g;
extern struct shader_t ocean_draw;

void ocean_init();

void ocean_calculate();