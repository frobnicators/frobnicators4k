in vec2 pos;

#extension GL_ARB_shading_language_420pack: enable

layout(binding=0) uniform sampler2D texture0;
uniform vec2 res;

out vec3 ocolor;

/* This must be 1.0 through the sum of the kernel (without INV_SUM applied )*/
#define INV_SUM 1.0/10.0

const float kernel[9] = float[9](
	1.0 * INV_SUM, 1.0 * INV_SUM, 1.0 * INV_SUM,
	1.0 * INV_SUM, 2.0 * INV_SUM, 1.0 * INV_SUM,
	1.0 * INV_SUM, 1.0 * INV_SUM, 1.0 * INV_SUM
);
	
void main() {
	float dx = 1.0f / res.x;
	float dy = 1.0f / res.y;
	vec2 uv = (pos + 1.0)/2.0;
  
	ocolor = vec3(0.0f, 0.0f, 0.0f);
	for ( int i = 0; i < 9; i++ ){
		float x,y;
		x = i%3 - 1;
		y = floor(i/3) - 1;
		ocolor += texture(texture0, vec2(clamp(uv.x + dx * x, 0, 1), clamp(uv.y + dy * y, 0, 1)) ).rgb * kernel[i];
	}
	//ocolor = texture(texture0, uv).rgb;
}
