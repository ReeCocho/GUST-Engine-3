#version 450

#include "GUST_Fragment.frag"

layout(std140, set = 0, binding = 3) uniform TestData
{
	layout(offset = 0) vec2 uv;
} testData;

layout(set = 1, binding = 0) uniform sampler2D tex;
layout(set = 1, binding = 1) uniform sampler2D normal_map;

void main()
{
	// Color
	vec3 color = texture(tex, GUST_UV * testData.uv).rgb;
	
	// Normal
	mat3 TBN = mat3(GUST_TANGENT, GUST_BITANGENT, GUST_NORMAL);
	vec3 normal = texture(normal_map, GUST_UV * testData.uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);
	
	GUST_OUT_COLOR = vec4(color, 1.0);
	GUST_OUT_NORMAL = vec4(normal, 1.0);
	GUST_OUT_POSITION = vec4(GUST_FRAG_POS, 1.0);
}