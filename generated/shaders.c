#include "shaders.h"
#pragma data_seg(".shaders")
const struct shader_entry_t _shaders[NUM_SHADERS] = {
	{ "common.glsl" , "#version 150\n#extension GL_ARB_explicit_attrib_location:enable\nuniform float t,b;\nfloat scene(vec3 p, out vec3 c);vec3 lpos();float dbox(vec3 v,vec3 i,vec3 n){vec3 f=abs(i-v)-n;return max(f.x,max(f.y,f.z));}vec3 rotateY(vec3 v,float i){float f,r;f=sin(i);r=cos(i);return vec3(v.x*r+v.z*f,v.y,v.z*r-v.x*f);}int icoolfFunc3d2(in int v){return v=v<<13^v,v*(v*v*15731+789221)+1376312589&2147483647;}float coolfFunc3d2(in int v){return float(icoolfFunc3d2(v));}float noise3f(in vec3 v){ivec3 f=ivec3(floor(v));vec3 n=fract(v);n=n*n*(3.-2.*n);int i=f.x+f.y*57+f.z*113;float s=mix(mix(mix(coolfFunc3d2(i+0),coolfFunc3d2(i+1),n.x),mix(coolfFunc3d2(i+57),coolfFunc3d2(i+58),n.x),n.y),mix(mix(coolfFunc3d2(i+113),coolfFunc3d2(i+114),n.x),mix(coolfFunc3d2(i+170),coolfFunc3d2(i+171),n.x),n.y),n.z);return 1.-s*9.31323e-10;}float noise2f(vec2 v){return noise3f(vec3(v,0));}float fbm(in vec3 v){float f=.5*noise3f(v)+.25*noise3f(v*2.)+.125*noise3f(v*4.)+.0625*noise3f(v*8.);return f;}vec3 n_at(vec3 v){vec3 f,n;f.x=scene(v+vec3(.001,0,0),n)-scene(v-vec3(.001,0,0),n);f.y=scene(v+vec3(0,.001,0),n)-scene(v-vec3(0,.001,0),n);f.z=scene(v+vec3(0,0,.001),n)-scene(v-vec3(0,0,.001),n);return normalize(f);}float brdr(vec3 v,vec3 i,vec3 n,float f){float r=0;vec3 s=cross(i,n),x;float z=1.;for(int c=0;c<5;++c){float l=pow(float(c+1.),2.)*.07;r+=abs(scene(v+s*l,x)-f)*z;r+=abs(scene(v+s*-l,x)-f)*z;z*=.5;}return step(.1,r);}vec3 light(vec3 v,vec3 f,float i,vec3 n){vec3 r=v-lpos();float s=length(r);r=normalize(r);vec3 x=n_at(v);float z=max(dot(-r,x),0.);vec3 c=n*z*vec3(.8);float m=0;return mix(c+n*vec3(.2),vec3(0),m);}vec3 raymarch(vec3 v,vec2 n){vec3 f=vec3(0,0,0);f+=vec3(1,0,0)*n.x;f+=vec3(0,1,0)*n.y*.5625;f+=vec3(0,0,1)*1;f=normalize(f);vec3 x;float r=0.;for(int i=0;i<100&&r<500;i++){float c=scene(v,x);if(c<.001)return light(v,f,c,x);r+=abs(c);v+=f*c;}return vec3(0);}" },
	{ "intro" , "in vec2 p;in vec3 c;out vec3 oc;void main(){oc=raymarch(vec3(0,0,t*10.),p);}float distance_sphere(vec3 v,vec3 l,float o){return length(v-l)-o;}float scene(vec3 v,out vec3 l){return l=vec3(.7,0,.7),dbox(mod(v,vec3(20)),vec3(10),vec3(6));}vec3 lpos(){return vec3(sin(t*2.)*20.,0,t-2.)*10.;}" },
	{ "s2" , "in vec2 p;in vec3 c;out vec3 oc;void main(){oc=raymarch(vec3(0,0,-10),p);}float pillar(vec3 v){return dbox(rotateY(v,v.y*.3),vec3(0.),vec3(1,5,1))+.2*clamp(fbm(v),0,1);}float scene(vec3 v,out vec3 o){float m=v.y+3.;o=vec3(.5);float l=pillar(v);return min(m,l);}vec3 lpos(){return vec3(-10);}" },
	{ "vertex.glsl" , "layout(location=0)in vec2 ip;out vec2 p;out vec3 c;void main(){p=ip,gl_Position=vec4(ip,0.,1.);}" },
};
