#include "Lights.hpp"

namespace gust
{
	PointLight::PointLight(Entity entity, Handle<PointLight> handle) : Component<PointLight>(entity, handle)
	{

	}

	PointLight::~PointLight()
	{

	}

	PointLightSystem::PointLightSystem(Scene* scene) : System(scene)
	{
		initialize<PointLight>();
	}

	PointLightSystem::~PointLightSystem()
	{

	}

	void PointLightSystem::onBegin()
	{
		auto pointLight = getComponent<PointLight>();
		pointLight->m_transform = pointLight->getEntity().getComponent<Transform>();
	}

	void PointLightSystem::onPreRender(float deltaTime)
	{
		auto pointLight = getComponent<PointLight>();

		PointLightData data = {};
		data.color = { pointLight->getColor(), 1 };
		data.intensity = pointLight->getIntensity();
		data.range = pointLight->m_range;
		data.position = { pointLight->m_transform->getPosition(), 1 };

		gust::renderer.draw(data);
	}



	DirectionalLight::DirectionalLight(Entity entity, Handle<DirectionalLight> handle) : Component<DirectionalLight>(entity, handle)
	{

	}

	DirectionalLight::~DirectionalLight()
	{

	}

	DirectionalLightSystem::DirectionalLightSystem(Scene* scene) : System(scene)
	{
		initialize<DirectionalLight>();
	}

	DirectionalLightSystem::~DirectionalLightSystem()
	{

	}

	void DirectionalLightSystem::onBegin()
	{
		auto directionalLight = getComponent<DirectionalLight>();
		directionalLight->m_transform = directionalLight->getEntity().getComponent<Transform>();
	}

	void DirectionalLightSystem::onPreRender(float deltaTime)
	{
		auto directionalLight = getComponent<DirectionalLight>();

		DirectionalLightData data = {};
		data.color = { directionalLight->getColor(), 1 };
		data.intensity = directionalLight->getIntensity();
		data.direction = { directionalLight->m_transform->getForward(), 1 };

		gust::renderer.draw(data);
	}



	SpotLight::SpotLight(Entity entity, Handle<SpotLight> handle) : Component<SpotLight>(entity, handle)
	{

	}

	SpotLight::~SpotLight()
	{

	}

	SpotLightSystem::SpotLightSystem(Scene* scene) : System(scene)
	{
		initialize<SpotLight>();
	}

	SpotLightSystem::~SpotLightSystem()
	{

	}

	void SpotLightSystem::onBegin()
	{
		auto spotLight = getComponent<SpotLight>();
		spotLight->m_transform = spotLight->getEntity().getComponent<Transform>();
	}

	void SpotLightSystem::onPreRender(float deltaTime)
	{
		auto spotLight = getComponent<SpotLight>();

		SpotLightData data = {};
		data.color = { spotLight->getColor(), 1 };
		data.intensity = spotLight->getIntensity();
		data.direction = { spotLight->m_transform->getForward(), 1 };
		data.cutOff = glm::cos(glm::radians(spotLight->m_angle));
		data.range = spotLight->m_range;
		data.position = { spotLight->m_transform->getPosition(), 1 };

		gust::renderer.draw(data);
	}
}