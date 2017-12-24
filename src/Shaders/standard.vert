#version 450

#include "GUST_Vertex.vert"

void main()
{
	gl_Position = GUST_DATA.MVP * vec4(IN_POSITION, 1.0f);
	
	// Calculate normal and tangent
	vec3 normal = mat3(transpose(inverse(GUST_DATA.MODEL))) * normalize(IN_NORMAL);
	vec3 tangent = vec3(GUST_DATA.MODEL * vec4(normalize(IN_TANGENT), 0.0)).xyz;
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	
	GUST_NORMAL = normal;
	GUST_FRAG_POS = vec3(GUST_DATA.MODEL * vec4(IN_POSITION, 1.0));
	GUST_UV = IN_UV;
	GUST_TANGENT = tangent;
	GUST_BITANGENT = cross(normal, tangent);
}