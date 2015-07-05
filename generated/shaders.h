#ifndef SHADERS_H
#define SHADERS_H

#if _DEBUG

#define SHADER_TYPE const char *

#define SHADER_COMMON_GLSL "common.glsl"
#define SHADER_FFT_GLSL "fft.glsl"
#define SHADER_NOISE_GLSL "noise.glsl"
#define SHADER_OCEAN_FRAG_GLSL "ocean_frag.glsl"
#define SHADER_OCEAN_RESOLVE_GLSL "ocean_resolve.glsl"
#define SHADER_OCEAN_VERT_GLSL "ocean_vert.glsl"
#define SHADER_RAYMARCH_GLSL "raymarch.glsl"
#define SHADER_RAYMARCH_FRAG_GLSL "raymarch_frag.glsl"
#define SHADER_VERTEX_GLSL "vertex.glsl"
#define SHADER_PASSTHRU_GLSL "passthru.glsl"

#else

#define SHADER_TYPE char

#define SHADER_COMMON_GLSL 0
#define SHADER_FFT_GLSL 1
#define SHADER_NOISE_GLSL 2
#define SHADER_OCEAN_FRAG_GLSL 3
#define SHADER_OCEAN_RESOLVE_GLSL 4
#define SHADER_OCEAN_VERT_GLSL 5
#define SHADER_RAYMARCH_GLSL 6
#define SHADER_RAYMARCH_FRAG_GLSL 7
#define SHADER_VERTEX_GLSL 8
#define SHADER_PASSTHRU_GLSL 9

#endif

#endif
