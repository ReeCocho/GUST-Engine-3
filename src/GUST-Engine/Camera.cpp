#include "Camera.hpp"

namespace gust
{
	Handle<Camera> Camera::mainCamera = Handle<Camera>::nullHandle();



	Camera::Camera(Entity entity, Handle<Camera> handle) : Component<Camera>(entity, handle)
	{

	}

	Camera::~Camera()
	{

	}

	void Camera::generateProjectionMatrix()
	{
		float width = static_cast<float>(gust::graphics.getWidth());
		float height = static_cast<float>(gust::graphics.getHeight());

		m_projection = glm::perspective
		(
			glm::radians(m_fieldOfView), 
			width / height, 
			m_nearClippingPlane, 
			m_farClippingPlane
		);
	}

	void Camera::generateViewMatrix()
	{
		m_view = glm::lookAt
		(
			m_transform->getPosition(), 
			m_transform->getPosition() + m_transform->getForward(), 
			m_transform->getUp()
		);
	}



	CameraSystem::CameraSystem(Scene* scene) : System(scene)
	{
		initialize<Camera>();
	}

	CameraSystem::~CameraSystem()
	{

	}

	void CameraSystem::onBegin()
	{
		auto camera = getComponent<Camera>();
		camera->m_transform = camera->getEntity().getComponent<Transform>();
		camera->m_virtualCamera = gust::renderer.createCamera();

		gust::renderer.setMainCamera(camera->m_virtualCamera);
	}

	void CameraSystem::onPreRender(float deltaTime)
	{
		auto camera = getComponent<Camera>();
		camera->generateProjectionMatrix();
		camera->generateViewMatrix();		

		camera->m_virtualCamera->view = camera->m_view;
		camera->m_virtualCamera->projection = camera->m_projection;
		camera->m_virtualCamera->viewPosition = camera->m_transform->getPosition();
	}

	void CameraSystem::onEnd()
	{
		auto camera = getComponent<Camera>();
		gust::renderer.destroyCamera(camera->m_virtualCamera);
	}
}