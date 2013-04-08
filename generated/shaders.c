#include "shaders.h"
#pragma data_seg(".shaders")
struct shader_entry_t _shaders[NUM_SHADERS] = {
	{ "boxfilter.glsl" , "in vec2 pos;\n#extension GL_ARB_shading_language_420pack:enable\nlayout(binding=0)uniform sampler2D texture0;uniform vec2 res;out vec3 ocolor;\n#define INV_SUM 1.0/10.0\nconst float v[9]=float[9](INV_SUM,INV_SUM,INV_SUM,INV_SUM,2.*INV_SUM,INV_SUM,INV_SUM,INV_SUM,INV_SUM);void main(){float r=1.f/res.x,I=1.f/res.y;vec2 s=(pos+1.)/2.;ocolor=vec3(0.f,0.f,0.f);for(int f=0;f<9;f++){float M,o;M=f%3-1;o=floor(f/3)-1;ocolor+=texture(texture0,vec2(clamp(s.x+r*M,0,1),clamp(s.y+I*o,0,1))).xyz*v[f];}}" },
	{ "gradient.glsl" , "in vec2 pos;out vec3 color;void main(){color=vec3((pos.x+1.)/2.,(pos.y+1.)/2.,mod(time/10.,1.));}" },
	{ "common.glsl" , "#version 150\n#extension GL_ARB_explicit_attrib_location:enable\nuniform float time;" },
	{ "vertex.glsl" , "layout(location=0)in vec2 in_pos;out vec2 pos;void main(){pos=in_pos,gl_Position=vec4(in_pos,0.,1.);}" },
};
