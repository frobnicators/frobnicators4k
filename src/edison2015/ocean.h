#pragma once

#include "frob_math.h"
#include "shader.h"

extern struct shader_t ocean_draw;

void ocean_init();

void ocean_seed(vec2* wind, float A, float g);

void ocean_calculate();