#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>
#include <sre/Inspector.hpp>
#include <sre/ModelImporter.hpp>

using namespace sre;

class ShadowExample{
public:
    ShadowExample(){
        r.init();

        std::vector<std::shared_ptr<Material>> materials_unused;

        mesh = sre::ModelImporter::importObj("test_data/", "suzanne.obj", materials_unused);
        meshSphere = Mesh::create().withSphere().build();
        meshPlane = Mesh::create().withQuad(2).build();

        worldLights.setAmbientLight(glm::vec3{0.02f});
        lightDirection = glm::normalize(glm::vec3{1,1,1});
        worldLights.addLight(Light::create().withDirectionalLight(lightDirection).withColor(Color(1,1,1),7).build());

        camera.setPerspectiveProjection(fieldOfViewY,near,far);
        camera.lookAt(eye,at,{0,1,0});

        mat = Shader::getStandardPBR()->createMaterial({{"S_SHADOW","true"}});
        mat->setName("PBR material");

        rebuildShadowMap();

        std::string info;
        if (!mat->getShader()->validateMesh(mesh.get(), info)){
            std::cout << info <<std::endl;
        } else {
            std::cout << "Mesh ok" << std::endl;
        }

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEMOTION && rotateMouse){
                float mouseSpeed = 1/50.0f;
                rotateY = event.motion.x*mouseSpeed;
                rotateX = event.motion.y*mouseSpeed;
            }
            if (event.button.button==SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.keyEvent = [&](SDL_Event& key){
            if (key.type == SDL_KEYDOWN){
                if (key.key.keysym.sym == SDLK_r){
                    rotateMouse = !rotateMouse;
                }
            }
        };
        r.startEventLoop();
    }

    void rebuildShadowMap(){
        shadowMapTexture = Texture::create()
                .withName("ShadowMapTex")
                .withGenerateMipmaps(false)
                .withFilterSampling(linearInterpolation)
                .withWrapUV(Texture::Wrap::ClampToBorder)
                .withDepth(shadowMapSize,shadowMapSize, depthPrecision)
                .build();

        shadowMap = Framebuffer::create()
                .withName("ShadowMap")
                .withDepthTexture(shadowMapTexture)
                .build();

        shadowMapMat = Shader::create()
                .withSourceFile("shadow_vert.glsl", ShaderType::Vertex)
                .withSourceFile("shadow_frag.glsl", ShaderType::Fragment)
                .withName("Shadow")
                .withOffset(biasOffset, biasFactor)                                          // shadow bias
                .withColorWrite({false,false,false,false})
                .build()->createMaterial();

        mat->setName("Shadow material");
    }

    // updates the shadow map
    static void updataShadowmapViewProjection(Camera* shadowmapCamera,
                                              glm::vec3 lightDirection,
                                              float fieldOfViewYDegrees,
                                              float near,
                                              float far,
                                              glm::vec3 eye,
                                              glm::vec3 at,
                                              float shadowMapFraction = 1.0 // amount of view frustum with shadowmap
    )
    {
        glm::vec3 viewDirection = glm::normalize(at - eye);
        glm::vec3 lightEye = eye + viewDirection * ((far-near)*shadowMapFraction/2 + near);
        glm::vec3 lightAt = lightEye - lightDirection;

        shadowmapCamera->setOrthographicProjection(2,-2,2);
        shadowmapCamera->lookAt(lightEye,lightAt, abs(lightDirection.x)<0.5? glm::vec3(1,0,0) : glm::vec3(0,0,1));
    }

    void renderWorld(RenderPass& rp,std::shared_ptr<Material> mat){
        rp.draw(sphere?meshSphere:mesh, glm::rotate(rotateX, glm::vec3(1,0,0))*glm::rotate(rotateY, glm::vec3(0,1,0)),mat);
        rp.draw(meshPlane, glm::translate(glm::vec3{0,-1.2f,0})*glm::rotate(glm::radians(-90.0f), glm::vec3(1,0,0)),mat);
    }

    void render(){
        // shadow pass - build shadow map - render shadow casters with shadow material
        updataShadowmapViewProjection(&shadowmapCamera, lightDirection, fieldOfViewY, near, far, eye, at);
        auto rp = RenderPass::create()
                .withFramebuffer(shadowMap)
                .withCamera(shadowmapCamera)
                .withWorldLights(&worldLights)
                .withClearColor(false)
                .withGUI(false)
                .build();

        renderWorld(rp, shadowMapMat);

        rp.finish();

        // render pass - render world with shadow lookup
        auto rp2 = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{0,0,0,1})
                .withWorldLights(&worldLights)
                .build();

        mat->set("shadowMap", shadowMapTexture);
        shadowViewProjection = shadowmapCamera.getProjectionTransform({shadowMapSize,shadowMapSize}) * shadowmapCamera.getViewTransform();
        mat->set("shadowViewProjection", shadowViewProjection);

        renderWorld(rp2, mat);

        bool c = false;


        ImGui::Checkbox("RotateMouse",&rotateMouse);
        ImGui::Checkbox("Sphere",&sphere);
        if (ImGui::IsItemHovered()){
            ImGui::SetTooltip("r to toogle");
        }
        c |= ImGui::DragFloat("Bias offset",&biasOffset);
        c |= ImGui::DragFloat("Bias factor",&biasFactor);
        c |= ImGui::Checkbox("Linear texture interpolation",&linearInterpolation);
        char* resolutionString =
                "4096\0"
                "2048\0"
                "1024\0"
                "512\0"
                "256\0"
                "128\0"
                "64\0"
                "32\0";
        c |= ImGui::Combo("Resolution",&resolutionId, resolutionString);
        char* depthString =
                "I16\0"
                "I24\0"
                "I32\0"
                "F32\0"
                "I24_STENCIL8\0"
                "F32_STENCIL8\0"
                "STENCIL8\0";

        c |= ImGui::Combo("Texture depth", reinterpret_cast<int *>(&depthPrecision), depthString);

        shadowMapSize = (unsigned int)glm::pow(2,12-resolutionId);
        if (c){
            rebuildShadowMap();
        }

        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }
    }
private:
    float fieldOfViewY = 45;
    float near = 0.1;
    float far = 10;
    int resolutionId = 1;
    unsigned int shadowMapSize = 2048;

    Texture::DepthPrecision depthPrecision = Texture::DepthPrecision::I24;

    glm::vec3 eye = {0,0,3.5};
    glm::vec3 at = {0,0,0};
    float biasOffset = 2.5;
    float biasFactor = 10;
    bool sphere = false;
    bool rotateMouse = true;
    bool linearInterpolation = true;
    glm::vec3 lightDirection;
    SDLRenderer r;
    Camera shadowmapCamera;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> meshSphere;
    std::shared_ptr<Mesh> meshPlane;
    std::shared_ptr<Material> mat;
    std::shared_ptr<Material> shadowMapMat;
    std::shared_ptr<Framebuffer> shadowMap;
    std::shared_ptr<Texture> shadowMapTexture;
    glm::mat4 shadowViewProjection;
    float rotateX = 0;
    float rotateY = 0;
    int texture = 0;
    bool showInspector = false;
    WorldLights worldLights;
};

int main() {
    new ShadowExample();

    return 0;
}

