#define NEAR_Z 0.0f
#define FAR_Z 250.0f
#define STEP_SIZE 0.03f
#define STEP_SIZE2 0.03f
#define TEXTURE_SCALE 0.001f

#define CLOUD_OFFSET 52.0

float hash(float n) { return fract(sin(n)*753.5453123); }
float noise(in vec3 x)
{
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f*f*(3.0 - 2.0*f);

	float n = p.x + p.y*157.0 + 113.0*p.z;
	return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
		mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
		mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
		mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

float map5(in vec3 p)
{
	vec3 q = p - vec3(0.0, 0.1, 1.0)*time;
	float f;
	f = 0.50000*noise(q); q = q*2.02;
	f += 0.25000*noise(q); q = q*2.03;
	f += 0.12500*noise(q); q = q*2.01;
	f += 0.06250*noise(q); q = q*2.02;
	f += 0.03125*noise(q);
	return clamp(1.5 - abs(p.y - CLOUD_OFFSET) - 2.0 + 1.75*f, 0.0, 1.0);
}

float map4(in vec3 p)
{
	vec3 q = p - vec3(0.0, 0.1, 1.0)*time;
	float f;
	f = 0.50000*noise(q); q = q*2.02;
	f += 0.25000*noise(q); q = q*2.03;
	f += 0.12500*noise(q); q = q*2.01;
	f += 0.06250*noise(q);
	return clamp(1.5 - abs(p.y - CLOUD_OFFSET + 0.5) - 2.0 + 1.75*f, 0.0, 1.0);
}
float map3(in vec3 p)
{
	vec3 q = p - vec3(0.0, 0.1, 1.0)*time;
	float f;
	f = 0.50000*noise(q); q = q*2.02;
	f += 0.25000*noise(q); q = q*2.03;
	f += 0.12500*noise(q);
	return clamp(1.5 - abs(p.y - CLOUD_OFFSET - 0.5) - 2.0 + 1.75*f, 0.0, 1.0);
}
float map2(in vec3 p)
{
	vec3 q = p - vec3(0.0, 0.1, 1.0)*time;
	float f;
	f = 0.50000*noise(q); q = q*2.02;
	f += 0.25000*noise(q);;
	return clamp(1.5 - abs(p.y - CLOUD_OFFSET) - 2.0 + 1.75*f, 0.0, 1.0);
}

vec3 sundir = normalize(vec3(-1.0, 0.0, -1.0));

vec4 integrate(in vec4 sum, in float dif, in float den, in vec3 bgcol, in float t)
{
	// lighting
	vec3 lin = vec3(0.65, 0.68, 0.7)*1.3 + 0.5*vec3(0.7, 0.5, 0.3)*dif;
	vec4 col = vec4(mix(1.15*vec3(1.0, 0.95, 0.8), vec3(0.65), den), den);
	col.xyz *= lin;
	col.xyz = mix(col.xyz, bgcol, 1.0 - exp(-0.003*t*t));
	// front to back blending
	col.a *= 0.4;
	col.rgb *= col.a;
	return sum + col*(1.0 - sum.a);
}

#define MARCH(STEPS,MAPLOD) for(int i=0; i<STEPS; i++) { vec3  pos = ro + t*rd; if( pos.y > (CLOUD_OFFSET+15.0) || pos.y < (CLOUD_OFFSET-15.0) || sum.a > 0.99 ) break; float den = MAPLOD( pos ); if( den>0.01 ) { float dif =  clamp((den - MAPLOD(pos+0.3*sundir))/0.6, 0.0, 1.0 ); sum = integrate( sum, dif, den, bgcol, t ); } t += max(0.1,0.02*t); }

vec4 raymarch(in vec3 ro, in vec3 rd, in vec3 bgcol)
{
	vec4 sum = vec4(0.0);

	float t = 0.0;

	MARCH(30,map5);
	MARCH(30, map4);
	MARCH(30,map3);
	MARCH(30,map2);

	return clamp(sum, 0.0, 1.0);
}

vec3 sky_color(vec3 ro, vec3 rd){
	// background sky
	float sun = clamp(dot(sundir, rd), 0.0, 1.0);
	vec3 col = vec3(0.6, 0.71, 0.75) - rd.y*0.2*vec3(1.0, 0.5, 1.0) + 0.15*0.5;
	col += 0.2*vec3(1.0, .6, 0.1)*pow(sun, 8.0);

	// clouds
	vec4 res = raymarch(ro, rd, col);
	col = col*(1.0 - res.w) + res.xyz;

	// sun glare
	col += 0.1*vec3(1.0, 0.4, 0.2)*pow(sun, 3.0);

	return col;
}
