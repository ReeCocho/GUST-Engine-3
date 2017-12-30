#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 GUST_OUT_POSITION;
layout(location = 1) out vec4 GUST_OUT_NORMAL;
layout(location = 2) out vec4 GUST_OUT_COLOR;
layout(location = 3) out vec4 GUST_OUT_MISC;

layout(set = 0, binding = 1) uniform samplerCube inAlbedo;

layout(location = 0) in vec3 uvw;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;

void main() 
{
	GUST_OUT_COLOR = texture(inAlbedo, uvw);
	GUST_OUT_POSITION = vec4(position, 1.0);
	GUST_OUT_NORMAL = vec4(normal, 1.0);
	GUST_OUT_MISC = vec4(0.0);
}