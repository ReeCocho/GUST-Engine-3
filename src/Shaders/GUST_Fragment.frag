#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 GUST_NORMAL;
layout(location = 1) in vec3 GUST_FRAG_POS;
layout(location = 2) in vec2 GUST_UV;
layout(location = 3) in vec3 GUST_TANGENT;
layout(location = 4) in vec3 GUST_BITANGENT;

layout(location = 0) out vec4 GUST_OUT_POSITION;
layout(location = 1) out vec4 GUST_OUT_NORMAL;
layout(location = 2) out vec4 GUST_OUT_COLOR;
layout(location = 3) out vec4 GUST_OUT_MISC;

#define GUST_OUT_ROUGHNESS GUST_OUT_MISC.r
#define GUST_OUT_METALLIC GUST_OUT_MISC.g
#define GUST_OUT_AO GUST_OUT_MISC.b

layout(std140, set = 0, binding = 2) uniform GUST_FRAG_DATA
{
	layout(offset = 0) vec4 VIEW_POSITION;
} GUST_DATA;