#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#define GLM_ENABLE_EXPERIMENTAL
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

        mesh = sre::ModelImporter::importObj("examples_data/", "suzanne.obj", materials_unused);

        meshPlane = Mesh::create().withQuad(2).build();

        worldLights.setAmbientLight(glm::vec3{0.02f});
        lightDirection = glm::normalize(glm::vec3{1,1,1});
        worldLights.addLight(Light::create().withDirectionalLight(lightDirection).withColor(Color(1,1,1),7).build());

        camera.setPerspectiveProjection(fieldOfViewY,near,far);
        camera.lookAt(eye,at,{0,1,0});

        mat = Shader::getStandardPBR()->createMaterial({{"S_SHADOW","true"}});
        mat->setName("PBR material");

        shadowMapMat = Shader::getShadow()->createMaterial();
        mat->setName("Shadow material");

        if (!sre::renderInfo().supportFBODepthAttachment){         // if not support for depth textures
            shadowMapTexture = Texture::create()
                    .withName("ShadowMapTex")
                    .withGenerateMipmaps(false)
                    .withFilterSampling(false)                     // filtering not supported, since packing depth in RGBA
                    .withRGBAData(nullptr,shadowMapSize,shadowMapSize)
                    .build();

            shadowMap = Framebuffer::create()
                    .withName("ShadowMap")
                    .withColorTexture(shadowMapTexture)
                    .build();
        } else {
            shadowMapTexture = Texture::create()
                    .withName("ShadowMapTex")
                    .withGenerateMipmaps(false)
                    .withFilterSampling(true)
                    .withWrapUV(Texture::Wrap::ClampToBorder)
                    .withDepth(shadowMapSize,shadowMapSize, Texture::DepthPrecision::I24)
                    .build();

            shadowMap = Framebuffer::create()
                    .withName("ShadowMap")
                    .withDepthTexture(shadowMapTexture)
                    .build();
        }

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEMOTION){
                float mouseSpeed = 1/50.0f;
                rotateY = event.motion.x*mouseSpeed;
                rotateX = event.motion.y*mouseSpeed;
            }
            if (event.button.button==SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.startEventLoop();
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

        shadowmapCamera->setOrthographicProjection(4,-4,4);
        shadowmapCamera->lookAt(lightEye,lightAt, abs(lightDirection.x)<0.5? glm::vec3(1,0,0) : glm::vec3(0,0,1));
    }

    void renderWorld(RenderPass& rp,std::shared_ptr<Material> mat){
        rp.draw(mesh, glm::rotate(rotateX, glm::vec3(1,0,0))*glm::rotate(rotateY, glm::vec3(0,1,0)),mat);
        rp.draw(meshPlane, glm::translate(glm::vec3{0,-1.2f,0})*glm::rotate(glm::radians(-90.0f), glm::vec3(1,0,0)),mat);
    }

    void render(){
        // shadow pass - build shadow map - render shadow casters with shadow material
        updataShadowmapViewProjection(&shadowmapCamera, lightDirection, fieldOfViewY, near, far, eye, at);
        auto rp = RenderPass::create()
                .withFramebuffer(shadowMap)
                .withCamera(shadowmapCamera)
                .withWorldLights(&worldLights)
                .withClearColor(true,{1,1,1,1})
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
        static glm::mat4 offset = glm::translate(glm::vec3(0.5f)) * glm::scale(glm::vec3(0.5f));
        glm::mat4 shadowViewProjOffset = offset * shadowmapCamera.getProjectionTransform({shadowMapSize,shadowMapSize}) * shadowmapCamera.getViewTransform();
        mat->set("shadowViewProjOffset", shadowViewProjOffset);

        renderWorld(rp2, mat);

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
    unsigned int shadowMapSize = 1024;
    glm::vec3 eye = {0,0,3.5};
    glm::vec3 at = {0,0,0};
    glm::vec3 lightDirection;
    SDLRenderer r;
    Camera shadowmapCamera;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> meshPlane;
    std::shared_ptr<Material> mat;
    std::shared_ptr<Material> shadowMapMat;
    std::shared_ptr<Framebuffer> shadowMap;
    std::shared_ptr<Texture> shadowMapTexture;
    float rotateX = 0;
    float rotateY = 0;
    int texture = 0;
    bool showInspector = false;
    WorldLights worldLights;
};

int main() {
    std::make_unique<ShadowExample>();
    return 0;
}

