// h~
layout(std430, binding = 0) buffer b1{
	vec2 ht[];
};

//h~slopex
layout(std430, binding = 1) buffer b2{
	vec2 htsx[];
};

//h~slopey
layout(std430, binding = 2) buffer b3{
	vec2 htsz[];
};

//h~dy
layout(std430, binding = 3) buffer b4{
	vec2 htdx[];
};

//h~dy
layout(std430, binding = 4) buffer b5{
	vec2 htdz[];
};

uniform int N;
