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

layout(location = 0) out vec3 uvw;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 position;

layout(std140, set = 0, binding = 0) uniform UBO
{
	layout(offset = 0) mat4 MVP;
	layout(offset = 64) mat4 Model;
} ubo;

void main()
{
	normal = inNormal;
	position = vec3(ubo.Model * vec4(inPosition, 1.0));

	uvw = inPosition;
	uvw.x *= -1.0;
  
	gl_Position = ubo.MVP * vec4(inPosition, 1.0);
}