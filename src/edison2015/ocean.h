#pragma once

#include "frob_math.h"
#include "fbo.h"

extern fbo_t ocean_fbo;

void ocean_init();

void ocean_seed(vec2* wind, float A, float g);

void ocean_calculate();
void ocean_render();