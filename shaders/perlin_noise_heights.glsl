in vec2 p; /* screen position (-1, 1) */
in vec3 c; /* camera position*/

out vec3 oc;

void main() {
	oc = raymarch(vec3(0, 0, 0), p);
}

const vec3 i_field_size = vec3(50);

float distance_sphere(vec3 eye, vec3 pos, float r){
	return length(eye-pos) - r;
}

vec3 permute(vec3 x) {
  return mod(((x*34.0)+1.0)*x, 289);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211, 0.366, -0.577, 0.024);
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = pow(m,vec3(4));

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79 - 0.85 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float scene(vec3 p, out vec3 color) {
	float h = snoise(vec2(p.x, p.z)/100.0)*10.0
		+ snoise(vec2(p.x, p.z)/50.0)*5.0
		+ snoise(vec2(p.x, p.z)/20.0)*1
		+ snoise(vec2(p.x, p.z)/5.0)*0.1;
	float floor_dist = p.y - h + 50.0;

//	if(p.y < -5.0) {
		color = vec3(0.5);
//	}

	return floor_dist;
}

vec3 lpos() { /* Shall return light position */
//	return vec3(0);
	return vec3(sin(t*2.)*2.0, 0, t - 2.0)*10.;
}