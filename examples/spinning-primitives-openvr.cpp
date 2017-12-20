#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/VR.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class SpinningPrimitivesOpenVRExample {
public:
    SpinningPrimitivesOpenVRExample(){
        r.init();

        camera.setPerspectiveProjection(60,0.1,100);
        material = Shader::getUnlit()->createMaterial();
        material->setTexture(Texture::create().withFile("examples_data/test.png").withGenerateMipmaps(true).build());
        mesh[0] = Mesh::create()
                .withQuad()
                .build();
        mesh[1] = Mesh::create()
                .withSphere()
                .build();
        mesh[2] = Mesh::create()
                .withCube()
                .build();
        mesh[3] = Mesh::create()
                .withTorus()
                .build();

		r.keyEvent = [&](SDL_Event& event)
		{
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_r)
				{
					if (speed != 0)
					{
						speed = 0;
					} else
					{
						speed = 0.5f;
					}
				}
			}
		};
        r.frameRender = [&](){
            render();
        };
		if (Renderer::instance->getVR() == nullptr){
			LOG_ERROR("Cannot initialize VR");
			return;
		}
		Renderer::instance->getVR()->renderVR = [&](RenderPass& renderPass, bool left)
		{
			render(renderPass);
		};
		Renderer::instance->getVR()->lookAt(eye, at, up);
        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{1, 0, 0, 1})
                .build();

		render(renderPass);
		static bool lookAt = true;
		ImGui::Checkbox("LookAt", &lookAt);
		if (!lookAt){
			ImGui::DragFloat3("Position", &position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &rotation.x,0.05f);
			Renderer::instance->getVR()->setViewTransform(glm::inverse( glm::translate(position)*glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z)));
		} else {
			ImGui::DragFloat3("Eye", &eye.x);
			ImGui::DragFloat3("At", &at.x);
			Renderer::instance->getVR()->lookAt(eye, at, up);
		}
		Renderer::instance->getVR()->debugGUI();
        i++;
    }

	void render(RenderPass& renderPass)
    {
		int index = 0;
		for (int x = -5; x<=5; x=x+2) {
			for (int y = -5; y<=5; y=y+2) {
				for (int z = -5; z <= 50; z=z+2) {
					glm::mat4 modelTransform = glm::translate(glm::vec3(-1.5 + x * 3, -1.5 + y * 3, z)) *
						glm::eulerAngleY(glm::radians(i * speed));
					renderPass.draw(mesh[index%3], modelTransform, material);
					index++;
				}
			}
		}
    }
private:
	glm::vec3 position = { 0,0,0 };
	glm::vec3 rotation = { 0,0,0 };
	
	glm::vec3 eye = { 0,0,6 };
	glm::vec3 at  = { 0,0,0 };
	glm::vec3 up  = { 0,1,0 };
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Material> material;
    std::shared_ptr<Mesh> mesh[4];
    int i=0;
	float speed = .5f;
};

int main() {
    new SpinningPrimitivesOpenVRExample();

    return 0;
}

