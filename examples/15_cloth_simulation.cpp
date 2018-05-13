#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"


#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sre/Skybox.hpp>
#include <sre/Inspector.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace sre;

/**
 * Based on "Mosegaards Cloth Simulation Coding Tutorial" ( http://cg.alexandra.dk/2009/06/02/mosegaards-cloth-simulation-coding-tutorial/ )
 */

/* Some physics constants */
float DAMPING = 0.01f; // how much to damp the cloth simulation each frame
float TIME_STEPSIZE2 = 0.5f*0.5f; // how large time step each particle takes each frame
int CONSTRAINT_ITERATIONS = 7; // how many iterations of constraint satisfaction each frame (more is rigid, less is soft)

    using namespace glm;



    std::shared_ptr<sre::Texture> buildCandyColorTexture(u8vec4 color1, u8vec4 color2, int width){
        std::vector<u8vec4> textureData;
        for (int i=0;i<width;i++){
            if (i%2==0){
                textureData.push_back(color1);
            } else {
                textureData.push_back(color2);
            }
        }

        auto dataPtr = glm::value_ptr(textureData[0]);
        return sre::Texture::create().withRGBAData(reinterpret_cast<const char *>(dataPtr), width, 1).withGenerateMipmaps(false).withFilterSampling(false).build();

    }

    /* The particle class represents a particle of mass that can move around in 3D space*/
    class Particle
    {
    private:
        bool movable; // can the particle move or not ? used to pin parts of the cloth

        float mass; // the mass of the particle (is always 1 in this example)
        vec3 pos; // the current position of the particle in 3D space
        vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
        vec3 acceleration; // a vector representing the current acceleration of the particle
        vec3 accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading

    public:
        explicit Particle(vec3 pos) : pos(pos), old_pos(pos),acceleration(vec3(0,0,0)), mass(1), movable(true), accumulated_normal(vec3(0,0,0)){}
        Particle() = default;

        void addForce(vec3 f)
        {
            acceleration += f/mass;
        }

        /* This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
           The method is called by Cloth.time_step()
           Given the equation "force = mass * acceleration" the next position is found through verlet integration*/
        void timeStep()
        {
            if(movable)
            {
                vec3 temp = pos;
                pos = pos + (pos-old_pos)*(1.0f-DAMPING) + acceleration*TIME_STEPSIZE2;
                old_pos = temp;
                acceleration = vec3(0,0,0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)
            }
        }

        vec3& getPos() {return pos;}

        void resetAcceleration() {acceleration = vec3(0,0,0);}

        void offsetPos(const vec3 v) { if(movable) pos += v;}

        void makeUnmovable() {movable = false;}

        void addToNormal(vec3 normal)
        {
            accumulated_normal += normalize(normal);
        }

        vec3& getNormal() { return accumulated_normal;} // notice, the normal is not unit length

        void resetNormal() {accumulated_normal = vec3(0,0,0);}

    };

    class Constraint
    {
    private:
        float rest_distance; // the length between particle p1 and p2 in rest configuration

    public:
        Particle *p1, *p2; // the two particles that are connected through this constraint

        Constraint(Particle *p1, Particle *p2) :  p1(p1),p2(p2)
        {
            vec3 vec = p1->getPos()-p2->getPos();
            rest_distance = length(vec);
        }

        /* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
        the method is called by Cloth.time_step() many times per frame*/
        void satisfyConstraint()
        {
            vec3 p1_to_p2 = p2->getPos() - p1->getPos(); // vector from p1 to p2
            float current_distance = length(p1_to_p2); // current distance between p1 and p2
            vec3 correctionVector = p1_to_p2*(1 - rest_distance/current_distance); // The offset vector that could moves p1 into a distance of rest_distance to p2
            vec3 correctionVectorHalf = correctionVector*0.5f; // Lets make it half that length, so that we can move BOTH p1 and p2.
            p1->offsetPos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
            p2->offsetPos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.
        }

    };

    class Cloth
    {
    private:
        struct Vertex {
            vec3 position;
            vec2 uv;
            vec3 normal;
        };

        std::shared_ptr<sre::Mesh> mesh;
        std::shared_ptr<Material> material;

        int num_particles_width; // number of particles in "width" direction
        int num_particles_height; // number of particles in "height" direction
        // total number of particles is num_particles_width*num_particles_height

        std::vector<Particle> particles; // all particles that are part of this cloth
        std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth

        int getParticleIndex(int x, int y) { return y*num_particles_width + x; }

        Particle* getParticle(int x, int y) {return &particles[getParticleIndex(x,y)];}
        void makeConstraint(Particle *p1, Particle *p2) {constraints.push_back(Constraint(p1,p2));}


        /* A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the
        normal vector of the triangle defined by the position of the particles p1, p2, and p3.
        The magnitude of the normal vector is equal to the area of the parallelogram defined by p1, p2 and p3
        */
        vec3 calcTriangleNormal(Particle *p1,Particle *p2,Particle *p3)
        {
            vec3 pos1 = p1->getPos();
            vec3 pos2 = p2->getPos();
            vec3 pos3 = p3->getPos();

            vec3 v1 = pos2-pos1;
            vec3 v2 = pos3-pos1;

            return cross(v1,v2);
        }

        /* A private method used by windForce() to calcualte the wind force for a single triangle
        defined by p1,p2,p3*/
        void addWindForcesForTriangle(Particle *p1,Particle *p2,Particle *p3, const vec3 direction)
        {
            vec3 normal = calcTriangleNormal(p1,p2,p3);
            vec3 d = normalize(normal);
            vec3 force = normal*(dot(d,direction));
            p1->addForce(force);
            p2->addForce(force);
            p3->addForce(force);
        }
    public:

        /* This is a important constructor for the entire system of particles and constraints*/
        Cloth(float width, float height, int num_particles_width, int num_particles_height) : num_particles_width(num_particles_width), num_particles_height(num_particles_height)
        {
            particles.resize(num_particles_width*num_particles_height); //I am essentially using this vector as an array with room for num_particles_width*num_particles_height particles

            // creating particles in a grid of particles from (0,0,0) to (width,-height,0)
            for(int x=0; x<num_particles_width; x++)
            {
                for(int y=0; y<num_particles_height; y++)
                {
                    vec3 pos = vec3(width * (x/(float)num_particles_width),
                                    -height * (y/(float)num_particles_height),
                                    0);
                    particles[y*num_particles_width+x]= Particle(pos); // insert particle in column x at y'th row
                }
            }

            // Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
            for(int x=0; x<num_particles_width; x++)
            {
                for(int y=0; y<num_particles_height; y++)
                {
                    if (x<num_particles_width-1) makeConstraint(getParticle(x,y),getParticle(x+1,y));
                    if (y<num_particles_height-1) makeConstraint(getParticle(x,y),getParticle(x,y+1));
                    if (x<num_particles_width-1 && y<num_particles_height-1) makeConstraint(getParticle(x,y),getParticle(x+1,y+1));
                    if (x<num_particles_width-1 && y<num_particles_height-1) makeConstraint(getParticle(x+1,y),getParticle(x,y+1));
                }
            }

            // Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
            for(int x=0; x<num_particles_width; x++)
            {
                for(int y=0; y<num_particles_height; y++)
                {
                    if (x<num_particles_width-2) makeConstraint(getParticle(x,y),getParticle(x+2,y));
                    if (y<num_particles_height-2) makeConstraint(getParticle(x,y),getParticle(x,y+2));
                    if (x<num_particles_width-2 && y<num_particles_height-2) makeConstraint(getParticle(x,y),getParticle(x+2,y+2));
                    if (x<num_particles_width-2 && y<num_particles_height-2) makeConstraint(getParticle(x+2,y),getParticle(x,y+2));			}
            }

            // making the upper left most three and right most three particles unmovable
            for(int i=0;i<3; i++)
            {
                getParticle(0+i ,0)->offsetPos(vec3(0.5,0.0,0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
                getParticle(0+i ,0)->makeUnmovable();

                getParticle(0+i ,0)->offsetPos(vec3(-0.5,0.0,0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
                getParticle(num_particles_width-1-i ,0)->makeUnmovable();
            }

            mesh = sre::Mesh::create()
                    .withName("Cloth mesh")
                    .withPositions(getPositions())
                    .withNormals(getNormals())
                    .withUVs(getUVs())
                    .withIndices(createIndices())
                    .withMeshTopology(MeshTopology::TriangleStrip)
                    .build();

            material = Shader::getStandardPBR()->createMaterial({{"S_TWO_SIDED","true"}});
            material->setColor({1.0f,1.0f,1.0f,1.0f});
            material->setMetallicRoughness({.5f,.5f});

            u8vec4 color1 = u8vec4(255,255,255, 255);
            u8vec4 color2 = u8vec4(153,51,51, 255);
            material->setTexture(buildCandyColorTexture(color1, color2, num_particles_width-1));
        }

        /* drawing the cloth as a smooth shaded (and colored according to column) OpenGL triangular mesh
        Called from the display() method
        The cloth is seen as consisting of triangles for four particles in the grid as follows:

        (x,y)   *--* (x+1,y)
                | /|
                |/ |
        (x,y+1) *--* (x+1,y+1)

        */
        void drawShaded(sre::RenderPass& rp)
        {
            // reset normals (which where written to last frame)
            for(auto & particle : particles)
            {
                particle.resetNormal();
            }

            //create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
            for(int x = 0; x<num_particles_width-1; x++)
            {
                for(int y=0; y<num_particles_height-1; y++)
                {
                    vec3 normal = calcTriangleNormal(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1));
                    getParticle(x+1,y)->addToNormal(normal);
                    getParticle(x,y)->addToNormal(normal);
                    getParticle(x,y+1)->addToNormal(normal);

                    normal = calcTriangleNormal(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1));
                    getParticle(x+1,y+1)->addToNormal(normal);
                    getParticle(x+1,y)->addToNormal(normal);
                    getParticle(x,y+1)->addToNormal(normal);
                }
            }

            // update mesh data
            mesh->update()
                    .withPositions(getPositions())
                    .withNormals(getNormals())
                    .build();

            rp.draw(mesh,glm::mat4(1.0f), material);
        }

        std::vector<uint32_t > createIndices(){
            std::vector<uint32_t> indices;

            for (int j = 0; j < num_particles_height-1; j++) {
                int index = 0;
                if (j > 0) {
                    indices.push_back(static_cast<uint32_t>(j * num_particles_width)); // make degenerate
                }
                for (int i = 0; i <= num_particles_width-1; i++) {
                    index = j * num_particles_width + i;
                    indices.push_back(static_cast<uint32_t>(index));
                    indices.push_back(static_cast<uint32_t>(index + num_particles_width));
                }
                if (j + 1 < num_particles_height-1) {
                    indices.push_back(static_cast<uint32_t>(index + num_particles_width)); // make degenerate
                }
            }
            return indices;
        }

        std::vector<glm::vec3> getPositions(){
            std::vector<glm::vec3> res;
            for(int y=0; y<num_particles_height; y++)
            {
                for(int x = 0; x<num_particles_width; x++)
                {
                    res.push_back(getParticle(x, y)->getPos());
                }
            }
            return res;
        }

        std::vector<glm::vec3> getNormals(){
            std::vector<glm::vec3> res;
            for(int y=0; y<num_particles_height; y++)
            {
                for(int x = 0; x<num_particles_width; x++)
                {
                    res.push_back(getParticle(x, y)->getNormal());
                }
            }
            return res;
        }

        std::vector<glm::vec4> getUVs(){
            std::vector<glm::vec4> res;
            for(int y=0; y<num_particles_height; y++)
            {
                for(int x = 0; x<num_particles_width; x++)
                {
                    vec4 uv(x/(num_particles_width - 1.0f),y/(num_particles_height-1.0f),0.0f,0.0f);
                    res.push_back(uv);
                }
            }
            return res;
        }

        /* this is an important methods where the time is progressed one time step for the entire cloth.
        This includes calling satisfyConstraint() for every constraint, and calling timeStep() for all particles
        */
        void timeStep()
        {
            for(int i=0; i<CONSTRAINT_ITERATIONS; i++) // iterate over all constraints several times
            {
                for(auto & constraint : constraints)
                {
                    constraint.satisfyConstraint(); // satisfy constraint.
                }
            }

            for (auto & particle : particles)
            {
                particle.timeStep(); // calculate the position of each particle at the next time step.
            }
        }

        /* used to add gravity (or any other arbitrary vector) to all particles*/
        void addForce(const vec3 direction)
        {
            for(auto & particle : particles)
            {
                particle.addForce(direction); // add the forces to each particle
            }
        }

        /* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
        void windForce(const vec3 direction)
        {
            for(int x = 0; x<num_particles_width-1; x++)
            {
                for(int y=0; y<num_particles_height-1; y++)
                {
                    addWindForcesForTriangle(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1),direction);
                    addWindForcesForTriangle(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1),direction);
                }
            }
        }

        /* used to detect and resolve the collision of the cloth with the ball.
        This is based on a very simples scheme where the position of each particle is simply compared to the sphere and corrected.
        This also means that the sphere can "slip through" if the ball is small enough compared to the distance in the grid bewteen particles
        */
        void ballCollision(const vec3 center,const float radius )
        {

            for(auto & particle : particles)
            {
                vec3 v = particle.getPos()-center;
                float l = length(v);
                if ( length(v) < radius) // if the particle is inside the ball
                {
                    particle.offsetPos(normalize(v)*(radius-l)); // project the particle to the surface of the ball
                }
            }
        }
    };

/***** Above are definition of classes; vec3, Particle, Constraint, and Cloth *****/


class ClothSimulation {
public:
    ClothSimulation(){
        r.init();

        cloth1 = std::make_shared<Cloth>(14,10,particlesWidth,particlesHeight); // one Cloth object of the Cloth class

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(80,0.1,100);

        sphereMaterial = Shader::getStandardPBR()->createMaterial();
        sphereMaterial->setColor({0.0f,1.0f,0.0f,1.0f});
        sphereMaterial->setMetallicRoughness({.5f,.5f});

        sphere = Mesh::create().withSphere().build();

        worldLights.setAmbientLight({0.05,0.05,0.05});
        worldLights.addLight(Light::create().withDirectionalLight({1, 1,1}).withColor({1,1,1}).build());

        skybox = Skybox::create();

        r.frameRender = [&](){
            render();
        };
        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.mouseEvent = [&](SDL_Event& event){
            static float rotateY = 0;
            static float rotateX = 0;
            if (event.type == SDL_MOUSEMOTION){
                float mouseSpeed = 1/50.0f;
                rotateY = event.motion.x*mouseSpeed;
                rotateX = (event.motion.y/(float)Renderer::instance->getWindowSize().y) * 3.14f-3.14f/2;
            }
            if (event.type == SDL_MOUSEBUTTONUP){
                if (event.button.button==SDL_BUTTON_RIGHT){
                    showGui = true;
                }
            }
            if (event.type == SDL_MOUSEWHEEL){
                camDist += event.wheel.y*0.1f;
                camDist = glm::max(camDist,1.0f);
            }
            auto rot = glm::rotate(rotateY, glm::vec3(0,1,0))*glm::rotate(rotateX, glm::vec3(1,0,0));

            auto rotatedPos = rot * glm::vec4(0,0,camDist,1);
            glm::vec3 offset = glm::vec3 (7,-5,0);
            camera.lookAt(glm::vec3(rotatedPos)+offset,offset,{0,1,0});
        };

        SDL_Event event; // init mouse event
        r.mouseEvent(event);

        r.startEventLoop();
    }

    void update(float deltaTime){
        ball_time++;
        ball_pos[2] = cos(ball_time/50.0f) * 7;

        cloth1->addForce(gravity*TIME_STEPSIZE2); // add gravity each frame, pointing down
        cloth1->windForce(wind*TIME_STEPSIZE2); // generate some wind each frame
        cloth1->timeStep(); // calculate the particle positions of the next frame
        cloth1->ballCollision(ball_pos,ball_radius+ball_collider_epsilon); // resolve collision with the ball
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withSkybox(skybox)
                .withName("Frame")
                .build();

        // setup light
        cloth1->drawShaded(renderPass);

        // draw solid sphere(ball_pos);
        renderPass.draw(sphere, glm::translate(ball_pos)*glm::scale(glm::vec3(ball_radius,ball_radius,ball_radius)), sphereMaterial);

        auto size = Renderer::instance->getWindowSize();
        ImVec2 imSize(250, 220.0f);
        ImVec2 imPos(size.x-250, 0);
        ImGui::SetNextWindowSize(imSize);                                   // imgui window size should have same width as SDL window size
        ImGui::SetNextWindowPos(imPos);
        ImGui::Begin("Cloth simulation settings", nullptr, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
        // create window without title
        ImGui::DragInt("Constraint iterations", &CONSTRAINT_ITERATIONS,1,1,30);
        ImGui::DragFloat("Damping", &DAMPING,0.05,0,1);
        ImGui::DragFloat3("Gravity", &gravity.x);
        ImGui::DragFloat3("Wind", &wind.x);
        ImGui::DragFloat("Ball size", &ball_radius,1.5,0.25,5);
        bool updated = ImGui::DragInt("Particles width", &particlesWidth,5,5,100);
        updated |= ImGui::DragInt("Particles height", &particlesHeight,5,5,100);
        ImGui::LabelText("Particle count", "%i", particlesWidth*particlesHeight);
        if (updated){
            cloth1 = std::make_shared<Cloth>(14,10,particlesWidth,particlesHeight); // one Cloth object of the Cloth class
        }
        ImGui::End();


        static Inspector inspector;
        inspector.update();
        if (showGui) inspector.gui();
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Cloth> cloth1;
    std::shared_ptr<Mesh> sphere;
    std::shared_ptr<Material> sphereMaterial;
    std::shared_ptr<Skybox> skybox;
    float camDist = 15;
    bool showGui = false;

    vec3 gravity = vec3 (0,-0.2,0);
    vec3 wind = vec3(0.5,0,0.2);
    int particlesWidth = 55;
    int particlesHeight = 45;

    vec3 ball_pos = vec3(7,-5,0); // the center of our one ball
    float ball_radius = 2; // the radius of our one ball
    float ball_collider_epsilon = .1; // the radius of our one ball

    float ball_time = 0; // counter for used to calculate the z position of the ball below
};

int main() {
    std::make_unique<ClothSimulation>();
    return 0;
}
