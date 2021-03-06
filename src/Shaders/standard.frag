#version 450

#include "GUST_Fragment.frag"

layout(std140, set = 0, binding = 3) uniform TestData
{
	layout(offset = 0) vec2 uv;
} testData;

layout(set = 1, binding = 0) uniform sampler2D tex;
layout(set = 1, binding = 1) uniform sampler2D normal_map;
layout(set = 1, binding = 2) uniform sampler2D metallic_map;
layout(set = 1, binding = 3) uniform sampler2D ao_map;
layout(set = 1, binding = 4) uniform sampler2D roughness_map;

void main()
{
	// Color
	vec3 color = texture(tex, GUST_UV * testData.uv).rgb;
	
	// Metallic
	GUST_OUT_METALLIC = texture(metallic_map, GUST_UV * testData.uv).r;
	
	// Roughness
	GUST_OUT_ROUGHNESS = texture(roughness_map, GUST_UV * testData.uv).r;
	
	// AO
	GUST_OUT_AO = texture(ao_map, GUST_UV * testData.uv).r;
	
	// Normal
	mat3 TBN = mat3(GUST_TANGENT, GUST_BITANGENT, GUST_NORMAL);
	vec3 normal = texture(normal_map, GUST_UV * testData.uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);
	
	GUST_OUT_COLOR = vec4(color, 1.0);
	GUST_OUT_NORMAL = vec4(normal, 1.0);
	GUST_OUT_POSITION = vec4(GUST_FRAG_POS, 1.0);
}