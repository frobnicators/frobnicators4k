#ifndef SHADERS_H
#define SHADERS_H

#if _DEBUG

#define SHADER_TYPE const char *

#define SHADER_COMMON_GLSL "common.glsl"
#define SHADER_COMPUTE_GLSL "compute.glsl"
#define SHADER_INTRO_GLSL "intro.glsl"
#define SHADER_VERTEX_GLSL "vertex.glsl"

#else

#define SHADER_TYPE char

#define SHADER_COMMON_GLSL 0
#define SHADER_COMPUTE_GLSL 1
#define SHADER_INTRO_GLSL 2
#define SHADER_VERTEX_GLSL 3

#endif

#endif
