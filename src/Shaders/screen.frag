#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D inAlbedo;

layout(location = 0) in VS_OUT
{
	vec2 uv;
} vsOut;

void main() 
{
	outColor = vec4(texture(inAlbedo, vsOut.uv).rgb, 1.0);
}