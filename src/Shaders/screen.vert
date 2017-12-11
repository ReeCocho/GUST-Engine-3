#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

out gl_PerVertex 
{
    vec4 gl_Position;
};

layout(location = 0) out VS_OUT
{
	vec2 uv;
} vsOut;

void main()
{
    gl_Position = vec4(inPosition, 1.0f);
	vsOut.uv = inUV;
}