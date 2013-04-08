#include "shaders.h"
#pragma data_seg(".shaders")
struct shader_entry_t _shaders[NUM_SHADERS] = {
	{ "boxfilter.glsl" , "in vec2 pos;\n#extension GL_ARB_shading_language_420pack: enable\nlayout(binding=0) uniform sampler2D texture0;\nuniform vec2 res;\nout vec3 ocolor;\n/* This must be 1.0 through the sum of the kernel (without INV_SUM applied )*/\n#define INV_SUM 1.0/10.0\nconst float kernel[9] = float[9](\n 1.0 * INV_SUM, 1.0 * INV_SUM, 1.0 * INV_SUM,\n 1.0 * INV_SUM, 2.0 * INV_SUM, 1.0 * INV_SUM,\n 1.0 * INV_SUM, 1.0 * INV_SUM, 1.0 * INV_SUM\n);\n \nvoid main() {\n float dx = 1.0f / res.x;\n float dy = 1.0f / res.y;\n vec2 uv = (pos + 1.0)/2.0;\n \n ocolor = vec3(0.0f, 0.0f, 0.0f);\n for ( int i = 0; i < 9; i++ ){\n float x,y;\n x = i%3 - 1;\n y = floor(i/3) - 1;\n ocolor += texture(texture0, vec2(clamp(uv.x + dx * x, 0, 1), clamp(uv.y + dy * y, 0, 1)) ).rgb * kernel[i];\n }\n //ocolor = texture(texture0, uv).rgb;\n}\n" },
	{ "gradient.glsl" , "in vec2 pos;\nout vec3 color;\nvoid main() {\n color = vec3((pos.x + 1.0)/2.0, (pos.y + 1.0)/2.0, mod(time/10.0, 1.0));\n}" },
	{ "common.glsl" , "#version 150\n#extension GL_ARB_explicit_attrib_location: enable\nuniform float time;\n" },
	{ "vertex.glsl" , "layout (location=0) in vec2 in_pos;\nout vec2 pos;\nvoid main(){\n pos = in_pos;\n gl_Position = vec4(in_pos, 0.0, 1.0);\n}" },
};
