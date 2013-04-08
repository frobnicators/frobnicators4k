#include "shaders.h"
#pragma data_seg(".shaders")
struct shader_entry_t _shaders[NUM_SHADERS] = {
	{ "gradient.glsl" , "in vec2 pos;\nout vec3 color;\nvoid main() {\n	color = vec3((pos.x + 1.0)/2.0, (pos.y + 1.0)/2.0, mod(time/10.0, 1.0));\n}" },
	{ "common.glsl" , "#version 150\n#extension GL_ARB_explicit_attrib_location: enable\nuniform float time;\n" },
	{ "vertex.glsl" , "layout (location=0) in vec2 in_pos;\nout vec2 pos;\nvoid main(){\n	pos = in_pos;\n	gl_Position = vec4(in_pos, 0.0, 1.0);\n}" },
};
