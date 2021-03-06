﻿#define OS_WIN

#include <iomanip>

#ifdef OS_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

#ifdef OS_LNX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <tiny_gltf.h>

#include <tiny_obj_loader.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Prismarine/Utils.hpp"
#include "Prismarine/Dispatcher.hpp"
#include "Prismarine/SceneObject.hpp"
#include "Prismarine/VertexInstance.hpp"
#include "Prismarine/MaterialSet.hpp"
#include "Prismarine/Radix.hpp"
#include <functional>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

namespace PaperExample {
    using namespace ppr;

    const int32_t kW = 0;
    const int32_t kA = 1;
    const int32_t kS = 2;
    const int32_t kD = 3;
    const int32_t kQ = 4;
    const int32_t kE = 5;
    const int32_t kSpc = 6;
    const int32_t kSft = 7;
    const int32_t kC = 8;
    const int32_t kK = 9;
    const int32_t kM = 10;




    class Controller {
        bool monteCarlo = true;

    public:
        glm::dvec3 eye = glm::dvec3(0.0f, 6.0f, 6.0f);
        glm::dvec3 view = glm::dvec3(0.0f, 2.0f, 0.0f);
        glm::dvec2 mposition;
        ppr::Dispatcher * raysp;

        glm::dmat4 project() {
#ifdef USE_CAD_SYSTEM
            return glm::lookAt(eye, view, glm::dvec3(0.0f, 0.0f, 1.0f));
#elif USE_180_SYSTEM
            return glm::lookAt(eye, view, glm::dvec3(0.0f, -1.0f, 0.0f));
#else
            return glm::lookAt(eye, view, glm::dvec3(0.0f, 1.0f, 0.0f));
#endif
        }

        void setRays(ppr::Dispatcher * r) {
            raysp = r;
        }

        void work(const glm::dvec2 &position, const double &diff, const bool &mouseleft, const bool keys[10]) {
            glm::dmat4 viewm = project();
            glm::dmat4 unviewm = glm::inverse(viewm);
            glm::dvec3 ca = (viewm * glm::dvec4(eye, 1.0f)).xyz();
            glm::dvec3 vi = (viewm * glm::dvec4(view, 1.0f)).xyz();

            bool isFocus = true;

            if (mouseleft && isFocus)
            {
                glm::dvec2 mpos = glm::dvec2(position) - mposition;
                double diffX = mpos.x;
                double diffY = mpos.y;
                if (glm::abs(diffX) > 0.0) this->rotateX(vi, diffX);
                if (glm::abs(diffY) > 0.0) this->rotateY(vi, diffY);
                if (monteCarlo) raysp->clearSampler();
            }
            mposition = glm::dvec2(position);

            if (keys[kW] && isFocus)
            {
                this->forwardBackward(ca, vi, diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if (keys[kS] && isFocus)
            {
                this->forwardBackward(ca, vi, -diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if (keys[kA] && isFocus)
            {
                this->leftRight(ca, vi, diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if (keys[kD] && isFocus)
            {
                this->leftRight(ca, vi, -diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if ((keys[kE] || keys[kSpc]) && isFocus)
            {
                this->topBottom(ca, vi, diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if ((keys[kQ] || keys[kSft] || keys[kC]) && isFocus)
            {
                this->topBottom(ca, vi, -diff);
                if (monteCarlo) raysp->clearSampler();
            }

            eye = (unviewm * glm::vec4(ca, 1.0f)).xyz();
            view = (unviewm * glm::vec4(vi, 1.0f)).xyz();
        }

        void leftRight(glm::dvec3 &ca, glm::dvec3 &vi, const double &diff) {
            ca.x -= diff / 100.0f;
            vi.x -= diff / 100.0f;
        }
        void topBottom(glm::dvec3 &ca, glm::dvec3 &vi, const double &diff) {
            ca.y += diff / 100.0f;
            vi.y += diff / 100.0f;
        }
        void forwardBackward(glm::dvec3 &ca, glm::dvec3 &vi, const double &diff) {
            ca.z -= diff / 100.0f;
            vi.z -= diff / 100.0f;
        }
        void rotateY(glm::dvec3 &vi, const double &diff) {
            glm::dmat4 rot = glm::rotate(-diff / float(raysp->displayHeight) / 0.5f, glm::dvec3(1.0f, 0.0f, 0.0f));
            vi = (rot * glm::dvec4(vi, 1.0f)).xyz();
        }
        void rotateX(glm::dvec3 &vi, const double &diff) {
            glm::dmat4 rot = glm::rotate(-diff / float(raysp->displayHeight) / 0.5f, glm::dvec3(0.0f, 1.0f, 0.0f));
            vi = (rot * glm::dvec4(vi, 1.0f)).xyz();
        }

    };




    const std::string bgTexName = "background2.jpg";

    GLuint loadCubemap() {
        FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(bgTexName.c_str(), 0);
        if (formato == FIF_UNKNOWN) {
            return 0;
        }
        FIBITMAP* imagen = FreeImage_Load(formato, bgTexName.c_str());
        if (!imagen) {
            return 0;
        }

        FIBITMAP* temp = FreeImage_ConvertTo32Bits(imagen);
        FreeImage_Unload(imagen);
        imagen = temp;

        uint32_t width = FreeImage_GetWidth(imagen);
        uint32_t height = FreeImage_GetHeight(imagen);

        GLuint texture = 0;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        uint8_t * pixelsPtr = FreeImage_GetBits(imagen);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, pixelsPtr);

        return texture;
    }



    bool loadMeshFile(std::string inputfile, std::vector<tinyobj::material_t> &materials, std::vector<tinyobj::shape_t> &shapes, tinyobj::attrib_t &attrib) {
        std::string err = "";
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());
        if (!err.empty()) { // `err` may contain warning message.
            std::cerr << err << std::endl;
        }
        return ret;
    }

    void loadMesh(std::vector<tinyobj::shape_t> &shapes, tinyobj::attrib_t &attrib, std::vector<float> &rawmeshdata) {
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    rawmeshdata.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    rawmeshdata.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    rawmeshdata.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                    if (attrib.normals.size() > 0) {
                        rawmeshdata.push_back(attrib.normals[3 * idx.normal_index + 0]);
                        rawmeshdata.push_back(attrib.normals[3 * idx.normal_index + 1]);
                        rawmeshdata.push_back(attrib.normals[3 * idx.normal_index + 2]);
                    }
                    else {
                        rawmeshdata.push_back(0);
                        rawmeshdata.push_back(0);
                        rawmeshdata.push_back(0);
                    }

                    if (attrib.texcoords.size() > 0) {
                        rawmeshdata.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                        rawmeshdata.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    }
                    else {
                        rawmeshdata.push_back(0);
                        rawmeshdata.push_back(0);
                    }
                }
                index_offset += fv;

                // per-face material
                shapes[s].mesh.material_ids[f];
            }
        }
    }

    class MeshTemplate {
    public:

        ppr::VertexInstance * deviceHandle = nullptr;
        std::vector<float> rawMeshData;
        std::vector<tinyobj::material_t> materials;
        std::vector<tinyobj::shape_t> shapes; 
        tinyobj::attrib_t attrib;
        glm::dmat4 transform = glm::dmat4(1.0);

        MeshTemplate * load(std::string inputfile) {
            if (loadMeshFile(inputfile, materials, shapes, attrib)) {
                loadMesh(shapes, attrib, rawMeshData);

                GLuint glBuf = -1;
                glCreateBuffers(1, &glBuf);
                glNamedBufferData(glBuf, rawMeshData.size() * sizeof(float), rawMeshData.data(), GL_STATIC_DRAW);


                



                // virtual accessor template
                ppr::VirtualAccessor vattr;
                vattr.offset4 = 0;

                ppr::VirtualBufferView bfv;
                bfv.stride4 = 8;
                bfv.offset4 = 0;

                uint32_t stride = 8;
                deviceHandle = new ppr::VertexInstance();

                AccessorSet * acs = new AccessorSet();
                BufferViewSet * bfvi = new BufferViewSet();
                vattr.bufferView = bfvi->addBufferView(bfv);

                deviceHandle->setBufferViewSet(bfvi);
                deviceHandle->setAccessorSet(acs);

                vattr.offset4 = 0;
                vattr.components = 3 - 1;
                deviceHandle->setVertexAccessor(acs->addVirtualAccessor(vattr));

                vattr.offset4 = 3;
                vattr.components = 3 - 1;
                deviceHandle->setNormalAccessor(acs->addVirtualAccessor(vattr));

                vattr.offset4 = 6;
                vattr.components = 2 - 1;
                deviceHandle->setTexcoordAccessor(acs->addVirtualAccessor(vattr));

                deviceHandle->setIndexed(false);
                deviceHandle->setVertices(glBuf);
                deviceHandle->setNodeCount(rawMeshData.size() / stride / 3);
                return this;
            }
        }
    };

    class PhysicsObject {
    public:
        uint32_t materialID = 0;
        btRigidBody* rigidBody = nullptr;
        MeshTemplate* meshTemplate = nullptr;
        double disappearTime = 0.0;

        double creationTime = 0.0;
    };



    // possible rigid bodies
    std::string rigidMeshTypeList[4] = {
        "toys/sphere.obj",
        "toys/cow.obj", 
        "toys/rbox.obj",
        "toys/box.obj" // wall
    };

    uint32_t activeShapes[2] = {
        0, 2
    };


    std::vector<MeshTemplate *> meshTemplates;
    std::vector<PhysicsObject *> objects;



    class PathTracerApplication {
    public:
        PathTracerApplication(const int32_t& argc, const char ** argv, GLFWwindow * wind);
        void passKeyDown(const int32_t& key);
        void passKeyRelease(const int32_t& key);
        void mousePress(const int32_t& button);
        void mouseRelease(const int32_t& button);
        void mouseMove(const double& x, const double& y);
        void process();
        void resize(const int32_t& width, const int32_t& height);
        void resizeBuffers(const int32_t& width, const int32_t& height);
        void pushPhysicsObject();
        void addStaticObject(glm::vec3 position, glm::quat rotation, uint32_t materialID);

    private:
        
        btBroadphaseInterface* broadphase;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btSequentialImpulseConstraintSolver* solver;
        btDiscreteDynamicsWorld* dynamicsWorld;

        GLFWwindow * window;
        ppr::Dispatcher * rays;
        ppr::SceneObject * intersector;
        Controller * cam;
        ppr::MaterialSet * materialManager;
        
        double time = 0;
        double diff = 0;
        glm::dvec2 mousepos;
        double mscale = 1.0f;
        int32_t depth = 16;
        int32_t switch360key = false;
        bool lbutton = false;
        bool keys[11] = { false , false , false , false , false , false , false, false, false };

#ifdef EXPERIMENTAL_GLTF
        tinygltf::Model gltfModel;
        std::vector<PhysicsObject> meshVec = std::vector<PhysicsObject>();
        std::vector<GLuint> glBuffers = std::vector<GLuint>();
        std::vector<uint32_t> rtTextures = std::vector<uint32_t>();
#endif

        struct {
            bool show_test_window = false;
            bool show_another_window = false;
            float f = 0.0f;
        } goptions;
    };

    uint32_t _byType(int &type) {
        switch (type) {
            case TINYGLTF_TYPE_VEC4:
            return 4;
            break;

            case TINYGLTF_TYPE_VEC3:
            return 3;
            break;

            case TINYGLTF_TYPE_VEC2:
            return 2;
            break;

            case TINYGLTF_TYPE_SCALAR:
            return 1;
            break;
        }
        return 1;
    }

    int32_t getTextureIndex(std::map<std::string, double> &mapped) {
        return mapped.count("index") > 0 ? mapped["index"] : -1;
    }



    void PathTracerApplication::pushPhysicsObject() {
        glm::dvec3 genDir = glm::normalize(cam->view - cam->eye);
        glm::dvec3 genPos = cam->eye + genDir * 2.0 + glm::dvec3(0, 1, 0);
        genDir *= 200.0;


        std::random_device rd;     // only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
        uint32_t shapeTypeS = std::uniform_int_distribution<int>(0, 1)(rng);
        uint32_t shapeType = activeShapes[shapeTypeS];




        btScalar mass = 1; btVector3 fallInertia(0, 0, 0);

        btCollisionShape* fallShape;
        if (shapeType == 0) {
            fallShape = new btSphereShape(0.8);
        }
        else
        if (shapeType == 1) {
            fallShape = new btCylinderShape(btVector3(0.7, 1.0, 1.0));
        }
        else 
        if (shapeType == 2) {
            fallShape = new btBoxShape(btVector3(0.7, 0.7, 0.7));
        }
        else {
            fallShape = new btSphereShape(1);
        }


        fallShape->calculateLocalInertia(mass, fallInertia);


        auto rotation = btQuaternion(0, 0, 0, 1);
            rotation.setEuler(
                std::uniform_real_distribution<float>(0, glm::two_pi<float>())(rng),
                std::uniform_real_distribution<float>(0, glm::two_pi<float>())(rng),
                std::uniform_real_distribution<float>(0, glm::two_pi<float>())(rng)
            );

        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(rotation, btVector3(genPos.x, genPos.y, genPos.z)));
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
        btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
        fallRigidBody->applyCentralForce(btVector3(genDir.x, genDir.y, genDir.z));
        dynamicsWorld->addRigidBody(fallRigidBody);

        // init timing state
        double time = glfwGetTime() * 1000.f;

        PhysicsObject * psb = new PhysicsObject();
        psb->meshTemplate = meshTemplates[shapeType];
        psb->rigidBody = fallRigidBody;
        psb->materialID = std::uniform_int_distribution<int>(0, 7)(rng);
        psb->creationTime = time;
        psb->disappearTime = 100000.f;
        objects.push_back(psb);
    }


    void PathTracerApplication::addStaticObject(glm::vec3 position, glm::quat rotation, uint32_t materialID) {
        btCollisionShape* groundShape = new btBoxShape(btVector3(10, 10, 1));//new btStaticPlaneShape(btVector3(0, 1, 0), 1);
        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z)));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);

        std::random_device rd;     // only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

        // init timing state
        double time = glfwGetTime() * 1000.f;
        PhysicsObject * psb = new PhysicsObject();
        psb->meshTemplate = meshTemplates[3];
        psb->rigidBody = groundRigidBody;
        psb->materialID = materialID;
        psb->creationTime = time;
        psb->disappearTime = 0.f;
        objects.push_back(psb);

    }





    PathTracerApplication::PathTracerApplication(const int32_t& argc, const char ** argv, GLFWwindow * wind) {
        window = wind;

        if (argc < 1) std::cerr << "-m (--model) for load obj model, -s (--scale) for resize model" << std::endl;
        std::string model_input = "";
        std::string directory = ".";

        // read arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = std::string(argv[i]);
            if ((arg == "-m") || (arg == "--model")) {
                if (i + 1 < argc) {
                    model_input = std::string(argv[++i]);
                }
                else {
                    std::cerr << "Model filename required" << std::endl;
                }
            }
            else
                if ((arg == "-s") || (arg == "--scale")) {
                    if (i + 1 < argc) {
                        mscale = std::stof(argv[++i]);
                    }
                }
                else
                    if ((arg == "-di") || (arg == "--dir")) {
                        if (i + 1 < argc) {
                            directory = std::string(argv[++i]);
                        }
                    }
                    else
                        if ((arg == "-d") || (arg == "--depth")) {
                            if (i + 1 < argc) {
                                depth = std::stoi(argv[++i]);
                            }
                        }
        }
        if (model_input == "") {
            std::cerr << "Physics mode enabled..." << std::endl;
        }



        for (int i = 0; i < 4;i++) {
            meshTemplates.push_back((new MeshTemplate())->load(rigidMeshTypeList[i]));
        }

        // initial transform




        glm::dmat4 matrix(1.0);
        matrix = glm::dmat4(1.0);
        matrix = glm::scale(matrix, glm::dvec3(0.8f, 0.8f, 0.8f));
        //matrix = glm::scale(matrix, glm::dvec3(1.25f, 1.25f, 1.25f));
        //matrix = glm::translate(matrix, glm::dvec3(-0.1f, -0.5f, 0.0f));
        meshTemplates[0]->transform = matrix;

        matrix = glm::dmat4(1.0);
        matrix = glm::rotate(matrix, glm::half_pi<double>(), glm::dvec3(0.0, 0.0, 1.0));
        matrix = glm::scale(matrix, glm::dvec3(0.25f));
        matrix = glm::translate(matrix, glm::dvec3(0.0f, 0.8f, 0.0f));
        meshTemplates[1]->transform = matrix;

        matrix = glm::dmat4(1.0);
        matrix = glm::scale(matrix, glm::dvec3(0.7f, 0.7f, 0.7f));
        meshTemplates[2]->transform = matrix;

        matrix = glm::dmat4(1.0);
        matrix = glm::scale(matrix, glm::dvec3(1.0f, 1.0f, 0.1f) * 10.0);
        meshTemplates[3]->transform = matrix;


        // init material system
        materialManager = new ppr::MaterialSet();

        // init ray tracer
        rays = new ppr::Dispatcher();
        rays->setSkybox(loadCubemap());
        
        // camera contoller
        cam = new Controller();
        cam->setRays(rays);

        // create geometry intersector
        intersector = new ppr::SceneObject();
        intersector->allocate(1024 * 1024);
        

        // here is 5 basic materials

        // yellow plastic
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(1.0f, 0.9f, 0.6f, 1.f);
            submat.specular = glm::vec4(0.0f, 0.0f, 0.00f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // red platic
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(1.0f, 0.6f, 0.6f, 1.f);
            submat.specular = glm::vec4(0.0f, 0.0f, 0.00f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // blue (slightly purple) plastic
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(0.6f, 0.7f, 1.0f, 1.f);
            submat.specular = glm::vec4(0.0f, 0.0f, 0.00f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // green plastic
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(0.6f, 1.0f, 0.6f, 1.0f);
            submat.specular = glm::vec4(0.0f, 0.0f, 0.00f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // fully metallic
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            submat.specular = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // copper
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(1.0f, 0.7f, 0.6f, 1.0f);
            submat.specular = glm::vec4(0.0f, 0.4f, 1.0f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // blue metallic
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(0.6f, 0.6f, 1.0f, 1.0f);
            submat.specular = glm::vec4(0.0f, 0.05f, 1.0f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }

        // gold
        {
            ppr::VirtualMaterial submat;
            submat.diffuse = glm::vec4(1.0f, 0.95f, 0.6f, 1.0f);
            submat.specular = glm::vec4(0.0f, 0.4f, 1.0f, 1.0f);
            submat.emissive = glm::vec4(0.0f);
            materialManager->addSubmat(&submat);
        }



        // init physics
        broadphase = new btDbvtBroadphase();
        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        solver = new btSequentialImpulseConstraintSolver;
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
        dynamicsWorld->setGravity(btVector3(0, -10, 0));


        // invisible physics plane (planned ray trace)
        btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -22, 0)));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);


        addStaticObject(glm::vec3(  0.0, 0.0, -10.0), glm::quat(glm::vec3(0.0, 0.0, 0.0)), 3);
        addStaticObject(glm::vec3( 10.0, 0.0, 0.0), glm::quat(glm::vec3(0.0, glm::pi<float>() * 0.5, 0.0)), 2);
        addStaticObject(glm::vec3(-10.0, 0.0, 0.0), glm::quat(glm::vec3(0.0, -glm::pi<float>() * 0.5, 0.0)), 1);
        addStaticObject(glm::vec3( 0.0, -10.0, 0.0), glm::quat(glm::vec3(-glm::pi<float>() * 0.5, 0.0, 0.0)), 0);
        addStaticObject(glm::vec3( 0.0, -12.0, 10.0), glm::quat(glm::vec3(0.0, 0.0, 0.0)), 0);

        // init timing state
        time = glfwGetTime() * 1000.f;
        diff = 0;
    }

    // key downs
    void PathTracerApplication::passKeyDown(const int32_t& key) {
        if (key == GLFW_KEY_W) keys[kW] = true;
        if (key == GLFW_KEY_A) keys[kA] = true;
        if (key == GLFW_KEY_S) keys[kS] = true;
        if (key == GLFW_KEY_D) keys[kD] = true;
        if (key == GLFW_KEY_Q) keys[kQ] = true;
        if (key == GLFW_KEY_E) keys[kE] = true;
        if (key == GLFW_KEY_C) keys[kC] = true;
        if (key == GLFW_KEY_SPACE) keys[kSpc] = true;
        if (key == GLFW_KEY_LEFT_SHIFT) keys[kSft] = true;
        if (key == GLFW_KEY_K) keys[kK] = true;
        if (key == GLFW_KEY_M) keys[kM] = true;
    }

    // key release
    void PathTracerApplication::passKeyRelease(const int32_t& key) {
        if (key == GLFW_KEY_W) keys[kW] = false;
        if (key == GLFW_KEY_A) keys[kA] = false;
        if (key == GLFW_KEY_S) keys[kS] = false;
        if (key == GLFW_KEY_D) keys[kD] = false;
        if (key == GLFW_KEY_Q) keys[kQ] = false;
        if (key == GLFW_KEY_E) keys[kE] = false;
        if (key == GLFW_KEY_C) keys[kC] = false;
        if (key == GLFW_KEY_SPACE) keys[kSpc] = false;
        if (key == GLFW_KEY_LEFT_SHIFT) keys[kSft] = false;
        if (key == GLFW_KEY_K) {
            if (keys[kK]) switch360key = true;
            keys[kK] = false;
        }
        if (key == GLFW_KEY_M) {
            pushPhysicsObject();
            keys[kM] = false;
        }
    }

    // mouse moving and pressing
    void PathTracerApplication::mousePress(const int32_t& button) { if (button == GLFW_MOUSE_BUTTON_LEFT) lbutton = true; }
    void PathTracerApplication::mouseRelease(const int32_t& button) { if (button == GLFW_MOUSE_BUTTON_LEFT) lbutton = false; }
    void PathTracerApplication::mouseMove(const double& x, const double& y) { mousepos.x = x, mousepos.y = y; }

    // resize buffers and canvas functions
    void PathTracerApplication::resizeBuffers(const int32_t& width, const int32_t& height) { rays->resizeBuffers(width, height); }
    void PathTracerApplication::resize(const int32_t& width, const int32_t& height) { rays->resize(width, height); }

    // processing
    void PathTracerApplication::process() {
        double t = glfwGetTime() * 1000.f;
        diff = t - time;
        time = t;




        // switch to 360 degree view
        cam->work(mousepos, diff, lbutton, keys);
        if (switch360key) {
            rays->switchMode();
            switch360key = false;
        }

        // clear BVH and load materials
        dynamicsWorld->stepSimulation(diff / 1000.f, 10);
        intersector->clearTribuffer();
        materialManager->loadToVGA();


        // initial transform
        glm::dmat4 matrix(1.0);
        matrix = glm::scale(matrix, glm::dvec3(mscale));

        // reload meshes with parameters
        for (int i = 0; i < objects.size();i++) {
            PhysicsObject * obj = objects[i];

            btTransform trans = obj->rigidBody->getWorldTransform();

            btScalar matr[16];
            trans.getOpenGLMatrix(matr);
            obj->meshTemplate->deviceHandle->setTransform(glm::dmat4(glm::make_mat4(matr)) *obj->meshTemplate->transform * matrix);
            obj->meshTemplate->deviceHandle->setMaterialOffset(obj->materialID);

            intersector->loadMesh(obj->meshTemplate->deviceHandle);

            if ((time - obj->creationTime) >= obj->disappearTime && obj->disappearTime > 0.0) {
                objects.erase(objects.begin() + i); i--;
                dynamicsWorld->removeRigidBody(obj->rigidBody);
            }
        }

        // build BVH in device
        intersector->build(matrix);

        // process ray tracing
        rays->camera(cam->eye, cam->view);
        for (int32_t j = 0;j < depth;j++) {
            if (rays->getRayCount() <= 0) break;
            rays->intersection(intersector);
            rays->applyMaterials(materialManager);
            rays->shade();
            rays->reclaim();
        }
        rays->sample();
        rays->render();
    }
}

PaperExample::PathTracerApplication * app;

static void error_callback(int32_t error, const char* description){
    std::cerr << ("Error: \n" + std::string(description)) << std::endl;
}

static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (action == GLFW_PRESS) app->passKeyDown(key);
    if (action == GLFW_RELEASE) app->passKeyRelease(key);
}

static void mouse_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods){
    if (action == GLFW_PRESS) app->mousePress(button);
    if (action == GLFW_RELEASE) app->mouseRelease(button);
}

static void mouse_move_callback(GLFWwindow* window, double x, double y){
    app->mouseMove(x, y);
}

int main(const int argc, const char ** argv)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);

    const double measureSeconds = 2.0;
    const double superSampling = 2.0;

    int32_t baseWidth = 640;
    int32_t baseHeight = 360;

    //int32_t baseWidth = 960;
    //int32_t baseHeight = 540;

    //int32_t baseWidth = 800;
    //int32_t baseHeight = 450;

    GLFWwindow* window = glfwCreateWindow(baseWidth, baseHeight, "Simple example", NULL, NULL);
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }

#ifdef _WIN32 //Windows DPI scaling
    HWND win = glfwGetWin32Window(window);
    int32_t baseDPI = 96;
    int32_t dpi = baseDPI;
#else //Other not supported
    int32_t baseDPI = 96;
    int32_t dpi = 96;
#endif
    
    // DPI scaling for Windows
#if (defined MSVC && defined _WIN32)
    dpi = GetDpiForWindow(win);
    int32_t canvasWidth = baseWidth * ((double)dpi / (double)baseDPI);
    int32_t canvasHeight = baseHeight * ((double)dpi / (double)baseDPI);
#else
    int32_t canvasWidth = baseWidth;
    int32_t canvasHeight = baseHeight;
    glfwGetFramebufferSize(window, &canvasWidth, &canvasHeight);
    dpi = double(baseDPI) * (double(canvasWidth) / double(baseWidth));
#endif



    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    if (glewInit() != GLEW_OK) glfwTerminate();

    app = new PaperExample::PathTracerApplication(argc, argv, window);
    app->resizeBuffers(int(double(baseWidth) * double(superSampling)), int(double(baseHeight) * double(superSampling)));
    app->resize(canvasWidth, canvasHeight);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetWindowSize(window, canvasWidth, canvasHeight);

    double lastTime = glfwGetTime();
    double prevFrameTime = lastTime;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int32_t oldWidth = canvasWidth, oldHeight = canvasHeight, oldDPI = dpi;
        glfwGetFramebufferSize(window, &canvasWidth, &canvasHeight);

        // DPI scaling for Windows
#if (defined MSVC && defined _WIN32)
        dpi = GetDpiForWindow(win);
#else
        {
            glfwGetWindowSize(window, &baseWidth, &baseHeight);
            dpi = double(baseDPI) * (double(canvasWidth) / double(baseWidth));
        }
#endif

        // scale window by DPI
        double ratio = double(dpi) / double(baseDPI);
        if (oldDPI != dpi) {
            canvasWidth  = baseWidth  * ratio;
            canvasHeight = baseHeight * ratio;
            glfwSetWindowSize(window, canvasWidth, canvasHeight);
        }

        // scale canvas
        if (oldWidth != canvasWidth || oldHeight != canvasHeight) {
            // set new base size
            baseWidth  = canvasWidth / ratio;
            baseHeight = canvasHeight / ratio;

            // resize canvas
            app->resize(canvasWidth, canvasHeight);
        }

        // do ray tracing
        app->process();

        // Measure speed
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= measureSeconds) {
            std::cout << "FPS: " << 1.f / (currentTime - prevFrameTime) << std::endl;
            lastTime += measureSeconds;
        }
        prevFrameTime = currentTime;

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
