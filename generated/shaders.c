#include "shaders.h"
#pragma data_seg(".shaders")
const struct shader_entry_t _shaders[NUM_SHADERS] = {
	{ "common.glsl" , "#version 150\n#extension GL_ARB_explicit_attrib_location:enable\nuniform float t,b;const int f=200;const float x=500;const vec3 v=vec3(0),d=vec3(0,-.7,-.7);const float i=.001;float e(vec3 i,vec3 v,vec3 f){vec3 x=abs(v-i)-f;return max(x.x,max(x.y,x.z));}vec3 e(vec3 v){vec3 f,x;f.x=df(v+vec3(i,0,0),x)-df(v-vec3(i,0,0),x);f.y=df(v+vec3(0,i,0),x)-df(v-vec3(0,i,0),x);f.z=df(v+vec3(0,0,i),x)-df(v-vec3(0,0,i),x);return normalize(f);}vec3 e(vec3 v,vec3 f){vec3 x=e(v);float i=max(dot(-d,x),0.);return f*i;}vec3 s(vec3 c,vec2 s){vec3 d=vec3(0,0,0);d+=vec3(1,0,0)*s.x;d+=vec3(0,1,0)*s.y*.5625;d+=-vec3(0,0,1)*1;d=normalize(d);vec3 m;float r=0.;for(int n=0;n<f&&r<x;n++){float z=df(c,m);if(z<i)return e(c,m);r+=abs(z);c+=d*z;}return v;}" },
	{ "intro" , "in vec2 p;in vec3 c;out vec3 oc;void main(){oc=raymarch(vec3(0),p);}const vec3 v=vec3(50);float d(vec3 a,out vec3 y){return y=vec3(1,0,0),a.x=mod(abs(a.x),v.x)-v.x*.5,a.y=mod(abs(a.y),v.y)-v.y*.5,a.z=mod(abs(a.z),v.z)-v.z*.5,_db(a,vec3(25),vec3(5));}" },
	{ "vertex.glsl" , "layout(location=0)in vec2 ip;out vec2 p;out vec3 c;void main(){p=ip,gl_Position=vec4(ip,0.,1.);}" },
};
