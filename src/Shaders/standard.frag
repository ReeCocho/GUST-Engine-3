#version 450

#include "GUST_Fragment.frag"

// layout(set = 3, binding = 0) uniform sampler2D tex;

void main()
{
	// Color
	vec3 color = vec3(1, 1, 1); // texture(tex, GUST_UV).rgb;
	
	// Normal
	vec3 normal = normalize(GUST_NORMAL);
	
	GUST_OUT_COLOR = vec4(color, 1.0);
	GUST_OUT_NORMAL = vec4(normal, 1.0);
	GUST_OUT_POSITION = vec4(GUST_FRAG_POS, 1.0);
}