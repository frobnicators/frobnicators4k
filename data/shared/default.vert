#extension GL_ARB_explicit_attrib_location: enable

layout (location=0) in vec4 in_pos;
layout (location=1) in vec2 in_uv;

out vec2 uv;

void main(){
	uv = in_uv;
	gl_Position = projectionViewMatrix *  in_pos;
}