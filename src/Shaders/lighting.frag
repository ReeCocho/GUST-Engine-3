#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMisc;

#define GUST_POINT_LIGHT_COUNT 64
#define GUST_DIRECTIONAL_LIGHT_COUNT 8
#define GUST_SPOT_LIGHT_COUNT 16

struct PointLightData
{
	vec4 position;
	vec4 color;
	float range;
	float intensity;
};

struct DirectionalLightData
{
	vec4 direction;
	vec4 color;
	float intensity;
};

struct SpotLightData
{
	vec4 position;
	vec4 direction;
	vec4 color;
	float cutOff;
	float intensity;
	float range;
};

struct CalculationData
{
	// Light
	vec3 direction;
	vec3 lightColor;
	float attenuation;
	
	// Material
	float metallic;
	float roughness;
	vec3 albedo;
	vec3 reflectance;
	
	// Misc
	vec3 normal;
	vec3 viewDirection;
};

layout(set = 0, binding = 1) uniform sampler2D inPosition;
layout(set = 0, binding = 2) uniform sampler2D inNormal;
layout(set = 0, binding = 3) uniform sampler2D inAlbedo;
layout(set = 0, binding = 4) uniform sampler2D inMisc;

layout(std140, set = 0, binding = 0) uniform LightingData 
{
	// Point light data
	layout(offset = 0) 
	PointLightData pointLights[GUST_POINT_LIGHT_COUNT];
	
	// Point lights in use
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40)) 
	uint pointLightCount;
	
	// Directional light data
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40) + 16) 
	DirectionalLightData directionalLights[GUST_DIRECTIONAL_LIGHT_COUNT];
	
	// Directional lights in use
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40) + (GUST_DIRECTIONAL_LIGHT_COUNT * 36) + 16)
	uint directionalLightCount;
	
	// Spot light data
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40) + (GUST_DIRECTIONAL_LIGHT_COUNT * 36) + 32) 
	SpotLightData spotLights[GUST_SPOT_LIGHT_COUNT];
	
	// Spot lights in use
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40) + (GUST_DIRECTIONAL_LIGHT_COUNT * 36) + (GUST_SPOT_LIGHT_COUNT * 60) + 32)
	uint spotLightCount;
	
	// Camera position
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40) + (GUST_DIRECTIONAL_LIGHT_COUNT * 36) + (GUST_SPOT_LIGHT_COUNT * 60) + 48)
	vec4 cameraPosition;
	
	// Ambient color
	layout(offset = (GUST_POINT_LIGHT_COUNT * 40) + (GUST_DIRECTIONAL_LIGHT_COUNT * 36) + (GUST_SPOT_LIGHT_COUNT * 60) + 64)
	vec4 ambient;
	
} lightingData;

layout(location = 0) in VS_OUT
{
	vec2 uv;
} vsOut;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 lighting(CalculationData data)
{
	// Calculate per-light radiance
	vec3 L = normalize(data.direction);
	vec3 H = normalize(data.viewDirection + L);
	vec3 radiance = data.lightColor * data.attenuation;
	
	// Cook-Torrance BRDF
	float NDF = DistributionGGX(data.normal, H, data.roughness);
	float G = GeometrySmith(data.normal, data.viewDirection, L, data.roughness);
	vec3 F = fresnelSchlick(clamp(dot(H, data.viewDirection), 0.0, 1.0), data.reflectance);
	
	vec3 nominator = NDF * G * F;
	float denominator = 4.0 * max(dot(data.normal, data.viewDirection), 0.0) * max(dot(data.normal, L), 0.0);
	vec3 specular = nominator / max(denominator, 0.001);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - data.metallic;
	
	float NdotL = max(dot(data.normal, L), 0.0);
	
	return (kD * data.albedo / PI + specular) * radiance * NdotL;
}

void main() 
{
	// Position
	vec3 position = texture(inPosition, vsOut.uv).rgb;
	
	// Normal
	vec3 normal = normalize(texture(inNormal, vsOut.uv).rgb);
	
	// Albedo
	vec3 albedo = texture(inAlbedo, vsOut.uv).rgb;
	albedo.r = pow(albedo.r, 4.4);
	albedo.g = pow(albedo.g, 4.4);
	albedo.b = pow(albedo.b, 4.4);
	
	// Misc
	vec4 misc = texture(inMisc, vsOut.uv);
	
	// Rougness, metallic, and ambient occlusion
	float roughness = misc.r;
	float metallic = misc.g;
	float ao = misc.b;
	
	// View direction
	vec3 viewDir = normalize(lightingData.cameraPosition.xyz - position);
	
	// Reflectance
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	
	// Total color
	vec3 total = vec3(0.0);
	
	// Directional lights
	for(uint i = 0; i < lightingData.directionalLightCount; ++i)
	{
		CalculationData data;
		data.direction = -lightingData.directionalLights[i].direction.rgb;
		data.lightColor = lightingData.directionalLights[i].color.rgb * lightingData.directionalLights[i].intensity;
		data.attenuation = 1.0;
		data.metallic = metallic;
		data.roughness = roughness;
		data.albedo = albedo;
		data.reflectance = F0;
		data.normal = normal;
		data.viewDirection = viewDir;
	
		total += lighting(data);
	}
	
	// Point lights
	for(uint i = 0; i < lightingData.pointLightCount; ++i)
	{
		vec3 direction = normalize(lightingData.pointLights[i].position.xyz - position);
		float dist = length(lightingData.pointLights[i].position.xyz - position);
	
		CalculationData data;
		data.direction = direction;
		data.lightColor = lightingData.pointLights[i].color.rgb * lightingData.pointLights[i].intensity;
		data.attenuation = 1.0 / (dist * dist);
		data.metallic = metallic;
		data.roughness = roughness;
		data.albedo = albedo;
		data.reflectance = F0;
		data.normal = normal;
		data.viewDirection = viewDir;
	
		total += lighting(data);
	}
	
	// Ambient lighting
	vec3 ambient = albedo * lightingData.ambient.xyz * lightingData.ambient.w * ao;
	
	vec3 color = ambient + total;
	
	// HDR tonemapping
	color = color / (color + vec3(1.0));
	
	// Gama correct
	color = pow(color, vec3(1.0/2.2));
	
	outAlbedo = vec4(color, 1.0);
	outNormal = vec4(normal, 1.0);
	outPosition = vec4(position, 1.0);
	outMisc = misc;
}