#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

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

layout(set = 0, binding = 1) uniform sampler2D inPosition;
layout(set = 0, binding = 2) uniform sampler2D inNormal;
layout(set = 0, binding = 3) uniform sampler2D inAlbedo;

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



/**
 * @brief Calculates the fragment color for a directional light.
 * @param Directional light to use.
 * @param Direction from the camera to the fragment.
 * @param Vector normal to the fragment.
 * @param Original color of the fragment.
 * @param Specularity of the fragment.
 */
vec3 calculateDirectionalLight(DirectionalLightData directionalLight, vec3 viewDirection, vec3 normal, vec3 color, float specularity)
{
	// // Ambient
	// vec3 ambient = ambientStrength * directionalLight.color.xyz * color;
	vec3 ambient = lightingData.ambient.xyz * lightingData.ambient.w;
	ambient *= directionalLight.color.xyz * color;
	
	// Diffuse
	vec3 lightDir = normalize(-directionalLight.direction.xyz);  
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * directionalLight.color.xyz * color;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);  
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 32);
	vec3 specular = 1.0 * spec * directionalLight.color.xyz * color * specularity ;

	// Add to lighting total
	return (ambient + diffuse + specular) * directionalLight.intensity;
}

/**
 * @brief Calculates fragment the color for a point light.
 * @param Point light to use.
 * @param Direction from the camera to the fragment.
 * @param Position of the fragment in world coordinates.
 * @param Vector normal to the fragment.
 * @param Original color of the fragment.
 * @param Specularity of the fragment.
 */
vec3 calculatePointLight(PointLightData pointLight, vec3 viewDirection, vec3 position, vec3 normal, vec3 color, float specularity)
{
	// Distance from light source to fragment
	float dist = length(pointLight.position.xyz - position);

	if(dist > pointLight.range)
		return vec3(0, 0, 0);

	// // Ambient
	// vec3 ambient = ambientStrength * pointLight.color.xyz * color;
	vec3 ambient = lightingData.ambient.xyz * lightingData.ambient.w;
	ambient *= pointLight.color.xyz * color;
	
	// Diffuse
	vec3 lightDir = normalize(pointLight.position.xyz - position);  
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * pointLight.color.xyz * color;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);  
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 32);
	vec3 specular = 1.0 * spec * pointLight.color.xyz * color * specularity; 

	// Attenuation
	float intensity = pointLight.intensity;
	float range = pointLight.range;
	float attenuation = intensity / (1.0f + ( (intensity * 128.0f * (dist * dist) ) / (range * range) ) );
	
	// Add to lighting total
	return (ambient + diffuse + specular) * attenuation;
}

/**
 * @brief Calculates fragment the color for a spot light.
 * @param Spot light to use.
 * @param Direction from the camera to the fragment.
 * @param Position of the fragment in world coordinates.
 * @param Vector normal to the fragment.
 * @param Original color of the fragment.
 * @param Specularity of the fragment.
 */
vec3 calculateSpotLight(SpotLightData spotLight, vec3 viewDirection, vec3 position, vec3 normal, vec3 color, float specularity)
{
	// Distance from light source to fragment
	float dist = length((spotLight.position.xyz) - position);

	if(dist > spotLight.range)
		return vec3(0, 0, 0);

	// Calculate theta based off spot light direction and direction to fragment
	vec3 lightDir = normalize(spotLight.position.xyz - position);  
	float theta = dot(lightDir, normalize(-spotLight.direction.xyz));
	
	// Calculate intensity
	float outerCutoff = spotLight.cutOff * 1.05f;
	float epsilon   = spotLight.cutOff - outerCutoff;
	float intensity = (1.0f - clamp((theta - outerCutoff) / epsilon, 0.0, 1.0)) * spotLight.intensity;    

	// Attenuation
	float range = spotLight.range;
	float attenuation = intensity / (1.0f + ( (intensity * 128.0f * (dist * dist) ) / (range * range) ) );
	intensity *= attenuation;

	if(theta > spotLight.cutOff) 
	{       
		// // Ambient
		// vec3 ambient = ambientStrength * spotLight.color.xyz * color;
		vec3 ambient = lightingData.ambient.xyz * lightingData.ambient.w;
		ambient *= spotLight.color.xyz * color;
		
		// Diffuse
		float diff = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = diff * spotLight.color.xyz * color;
	
		// Specular
		vec3 reflectDir = reflect(-lightDir, normal);  
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 32);
		vec3 specular = 1.0 * spec * spotLight.color.xyz * color * specularity;   
	
		// Add to lighting total
		return (ambient + diffuse + specular) * intensity;
	}
	
	return vec3(0, 0, 0);
}	

void main() 
{
	// Position
	vec3 position = texture(inPosition, vsOut.uv).rgb;
	
	// Normal
	vec3 normal = texture(inNormal, vsOut.uv).rgb;
	
	// Color
	vec3 color = texture(inAlbedo, vsOut.uv).rgb;
	
	// View direction
	vec3 viewDir = normalize(lightingData.cameraPosition.xyz - position);
	
	// Total color
	vec3 total = vec3(0, 0, 0);
	
	// Directional lights
	for(uint i = 0; i < lightingData.directionalLightCount; i++)
		total += calculateDirectionalLight(lightingData.directionalLights[i], viewDir, normal, color, 1.0f);
	
	// Point lights
	for(uint i = 0; i < lightingData.pointLightCount; i++)
		total += calculatePointLight(lightingData.pointLights[i], viewDir, position, normal, color, 0.0f);
		
	// Spot lights
	for(uint i = 0; i < lightingData.spotLightCount; i++)
		total += calculateSpotLight(lightingData.spotLights[i], viewDir, position, normal, color, 0.0f);
	
	outAlbedo = vec4(total, 1.0);
	outNormal = vec4(normal, 1.0);
	outPosition = vec4(position, 1.0);
}