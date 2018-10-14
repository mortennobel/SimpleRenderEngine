#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/Inspector.hpp>

using namespace sre;

class SpinningPrimitivesTexExample {
public:
	SpinningPrimitivesTexExample(){
		r.init();

		camera.lookAt({0,0,12},{0,0,0},{0,1,0});
		camera.setPerspectiveProjection(60,0.1,100);
		material = Shader::getUnlit()->createMaterial();
		material->setTexture(Texture::create().withFile("examples_data/test.png").withGenerateMipmaps(true).build());
		materialBlack = Shader::getUnlit()->createMaterial();
		materialBlack->setColor(Color(0,0,0));
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

		mesh[4] = Mesh::create()
				.withWireCube()
				.build();

		r.frameRender = [&](){
			render();
		};
		r.mouseEvent = [&](SDL_Event& event){
			if (event.button.button==SDL_BUTTON_RIGHT){
				showInspector = true;
			}
		};
		r.startEventLoop();
	}

	void render(){
		auto renderPass = RenderPass::create()
				.withCamera(camera)
				.withClearColor(true,{1, 0, 0, 1})
				.build();

		const float speed = .5f;
		int index = 0;
		for (int x=0;x<5;x++){

				glm::mat4 modelTransform = glm::translate(glm::vec3(-6+x*3,0,0)) *
										   glm::eulerAngleY(glm::radians( i * speed));
				renderPass.draw(mesh[index], modelTransform, x<4?material:materialBlack);
				index++;

		}
		static Inspector inspector;
		inspector.update();
		if (showInspector){
			inspector.gui();
		}
		i++;
	}
private:
	SDLRenderer r;
	Camera camera;
	std::shared_ptr<Material> material;
	std::shared_ptr<Material> materialBlack;
	std::shared_ptr<Mesh> mesh[5];
	int i=0;
	bool showInspector = false;
};

int main() {
	 
	std::make_unique<SpinningPrimitivesTexExample>();
	return 0;
}
