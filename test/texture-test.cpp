//
// Created by Morten Nobel-Jørgensen on 18/07/2017.
//
// Test based on http://www.schaik.com/pngsuite/pngsuite_bas_png.html

#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sre/SpriteAtlas.hpp>
#include <sre/Inspector.hpp>


using namespace sre;

class TextureTestExample {
public:
	TextureTestExample()
    {
        r.init();

        camera.setOrthographicProjection(3,-2,2);

		filenames = {
			"basn0g01.png",
			"basn0g02.png",
			"basn0g04.png",
			"basn0g08.png",
			"basn0g16.png",
			"basn2c08.png",
			"basn2c16.png",
			"basn3p01.png",
			"basn3p02.png",
			"basn3p04.png",
			"basn4a08.png",
			"basn6a08.png",
			"basn6a16.png"
		};
        
		for (const auto s : filenames)
		{
			std::cout << "Load " << s << std::endl;
			textures.push_back(Texture::create().withFile(std::string("test_data/")+s).build());
		}

		mesh = Mesh::create().withCube().build();
		material = Shader::create()
				.withSourceResource("unlit_vert.glsl", ShaderType::Vertex)
				.withSourceResource("unlit_frag.glsl", ShaderType::Fragment)
				.withBlend(BlendType::AlphaBlending)
				.build()->createMaterial();
		
        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true, {.3f, .3f, 1, 1})
                .build();

		ImGui::ListBox("Texture", &selection, filenames.data(), filenames.size());
		if (filenames[selection][5] == 'g')
		{
			ImGui::LabelText("png type","Grayscale");
		}
		if (filenames[selection][5] == 'c')
		{
			ImGui::LabelText("png type", "Color");
		}
		if (filenames[selection][5] == 'p')
		{
			ImGui::LabelText("png type", "Palette");
		}
		if (filenames[selection][5] == 'a')
		{
			ImGui::LabelText("png type", "Alpha");
		}

		ImGui::LabelText("Size", "%d x %d", textures[selection]->getWidth(), textures[selection]->getHeight());
		ImGui::LabelText("Transparent", "%s", textures[selection]->isTransparent()?"true":"false");
		const char* colorSpace;
		if (textures[selection]->getSamplerColorSpace() == Texture::SamplerColorspace::Gamma){
			colorSpace = "Gamma";
		} else {
			colorSpace = "Linear";
		}
		ImGui::LabelText("Colorspace", "%s", colorSpace);

		material->setTexture(textures[selection]);
		renderPass.draw(mesh, glm::mat4(1), material);
    }
private:    
	std::vector<const char*> filenames;
	std::vector<std::shared_ptr<sre::Texture>> textures;
    SDLRenderer r;
    Camera camera;
	int selection = 0;

	std::shared_ptr<sre::Mesh> mesh;
	std::shared_ptr<sre::Material> material;
};

int main() {
	std::make_unique<TextureTestExample>();
    return 0;
}
