#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 IN_POSITION;
layout(location = 1) in vec2 IN_UV;
layout(location = 2) in vec3 IN_NORMAL;
layout(location = 3) in vec3 IN_TANGENT;

layout(std140, set = 0, binding = 0) uniform GUST_VERT_DATA
{
	layout(offset = 0) mat4 MVP;
	layout(offset = 64) mat4 MODEL;
} GUST_DATA;

// layout(std140, set = 0, binding = 1) uniform CUSTOM_VERT_DATA
// {
// 	layout(offset = 0) float test;
// } CUSTOM_DATA;

layout(location = 0) out vec3 GUST_NORMAL;
layout(location = 1) out vec3 GUST_FRAG_POS;
layout(location = 2) out vec2 GUST_UV;
layout(location = 3) out vec3 GUST_TANGENT;
layout(location = 4) out vec3 GUST_BITANGENT;