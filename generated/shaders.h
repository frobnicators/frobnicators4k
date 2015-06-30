#ifndef SHADERS_H
#define SHADERS_H

#if _DEBUG

#define SHADER_TYPE const char *

#define SHADER_COMMON_GLSL "common.glsl"
#define SHADER_FFT_GLSL "fft.glsl"
#define SHADER_OCEAN_COMPUTE_GLSL "ocean_compute.glsl"
#define SHADER_OCEAN_DRAW_GLSL "ocean_draw.glsl"
#define SHADER_OCEAN_RASTERIZE_FRAG_GLSL "ocean_rasterize_frag.glsl"
#define SHADER_OCEAN_RASTERIZE_VERT_GLSL "ocean_rasterize_vert.glsl"
#define SHADER_VERTEX_GLSL "vertex.glsl"

#else

#define SHADER_TYPE char

#define SHADER_COMMON_GLSL 0
#define SHADER_FFT_GLSL 1
#define SHADER_OCEAN_COMPUTE_GLSL 2
#define SHADER_OCEAN_DRAW_GLSL 3
#define SHADER_OCEAN_RASTERIZE_FRAG_GLSL 4
#define SHADER_OCEAN_RASTERIZE_VERT_GLSL 5
#define SHADER_VERTEX_GLSL 6

#endif

#endif
