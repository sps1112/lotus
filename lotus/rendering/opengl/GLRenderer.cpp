#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lotus/rendering.h"
#include "lotus/debug.h"

namespace Lotus::Rendering
{

    void GLRenderer::setViewport(int x, int y, int width, int height) const
    {
        glViewport(x, y, width, height);
    }

    void GLRenderer::framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    URefWindow& GLRenderer::getActiveWindow()
    {
        return window;
    }

    void GLRenderer::shutdown()
    {
        glfwDestroyWindow(window->getGLWindow());
        glfwTerminate();
    }

    void GLRenderer::renderScene(const Scene& scene)
    {
        glClearColor(0.2f, 0.3f, 0.3, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Get camera properties
        glm::mat4 view = scene.camera->GetViewMatrix();
        glm::vec3 cameraPos = scene.camera->getPosition();

        // Get viewport properties
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float width = viewport[2];
        float height = viewport[3];
        glm::mat4 projection = glm::perspective(scene.camera->getFieldOfView(), width / height, 0.1f, 100.0f);

        // Get scene lights
        std::vector<CPointLight> pointLights = scene.getPointLightProps();
        std::vector<CSpotlight> spotlights = scene.getSpotlightProps();
        std::vector<CDirectionalLight> dirLights = scene.getDirLightProps();

        // Render actors
        const std::vector<SRefActor>& actors = scene.actors;
        for (const SRefActor& actor : actors)
        {
            if (actor->isActive)
            {
                // TODO: Cache these shaders somewhere? So that lighting and camera properties are set only once?
                SRefShader shader = actor->model.shader;
                shader->use();

                // Set camera
                shader->setMat4fv("view", GL_FALSE, glm::value_ptr(view));
                shader->setMat4fv("projection", GL_FALSE, glm::value_ptr(projection));
                shader->setVec3f("viewPos", glm::value_ptr(cameraPos));

                // Set lighting
                shader->setPointLightArray("pointLight", pointLights);
                shader->setSpotlightArray("spotlight", spotlights);
                shader->setDirLightArray("dirLight", dirLights);

                // Set transforms and draw actor
                CTransform transform = actor->transform;
                glm::mat4 model(1.0f);
                model = glm::translate(model, transform.position);
                model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, transform.rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, transform.rotation.z, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::scale(model, transform.scale);
                shader->setMat4fv("model", GL_FALSE, glm::value_ptr(model));

                // Material. Diffuse is set through a texture
                float specular[] = {0.5f, 0.5f, 0.5f};
                shader->setVec3f("material.specular", specular);
                shader->setFloat("material.shininess", 32);

                renderModel(actor->model.model, shader);
            }
        }

        // Render skybox
//        SRefShader skyShader = scene.skybox->model.shader;
//        skyShader->use();
//        glm::mat4 skyView = glm::mat4(glm::mat3(view));
//        skyShader->setMat4fv("view", GL_FALSE, glm::value_ptr(skyView));
//        skyShader->setMat4fv("projection", GL_FALSE, glm::value_ptr(projection));
//        skyShader->setInt("skybox", 0);
//        renderSky(scene.skybox);
    }

    void GLRenderer::renderSky(SRefSkybox skybox)
    {
        const std::vector<Resource::Mesh> meshes = skybox->model.model->getMeshes();
        if (meshes.size() != 1)
        {
            LOG_ERROR("Number of meshes in the skybox is not 1");
            return;
        }
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(meshes[0].VAO);

        glActiveTexture(GL_TEXTURE0);
        unsigned int id = skybox->cubemap.get()->id;
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        glDrawElements(GL_TRIANGLES, meshes[0].indices.size(), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

    void GLRenderer::renderModel(const Resource::SRefModel& model, const SRefShader& shader)
    {
        std::vector<Resource::Mesh> meshes = model->getMeshes();
        for (Resource::Mesh& mesh : meshes)
        {
            // Setup textures
            unsigned int diffuseNum = 1;
            unsigned int specularNum = 1;
            for (unsigned int i = 0; i < mesh.textures.size(); ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                std::string number;
                std::string name = mesh.textures[i]->type;
                if (name == DIFFUSE_TEXTURE)
                {
                    number = std::to_string(diffuseNum++);
                }
                else if (name == SPECULAR_TEXTURE)
                {
                    number = std::to_string(specularNum++);
                }

                shader->setInt("material." + name + number, i);
                glBindTexture(GL_TEXTURE_2D, mesh.textures[i]->id);
            }

            // Reset the active texture
            glActiveTexture(GL_TEXTURE0);

            glBindVertexArray(mesh.VAO);

            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }

    GLRenderer& GLRenderer::get()
    {
        static GLRenderer renderer;
        return renderer;
    }

    unsigned int GLRenderer::createTexture(unsigned char* data, int width, int height, unsigned int format)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }

    void GLRenderer::init(bool isDebug)
    {
        unsigned int width = 800;
        unsigned int height = 600;

        // Initialize and configure GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, isDebug);

        window = std::make_unique<Window>(Lotus::Context::OPEN_GL, width, height, "Lotus - OpenGL");
        glfwSetFramebufferSizeCallback(window->getGLWindow(), framebufferSizeCallback);

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        {
            LOG_ERROR("Failed to initialize GLAD");
            return;
        }

        // Enable depth options
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Setup framebuffer targets
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // Create color buffer
        glGenTextures(1, &texColorBuffer);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // attach it to the FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

        // create depth/stencil renderbuffer objects
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach it to the FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        // setup completed?
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR("Framebuffer is not complete");
        }

        // Unbind the FBO now that everything has been set up
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Enable debug output
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            LOG_INFO("Setting up a debug context...");
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(debugMessageCallback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    }

    void GLRenderer::debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                          const GLchar* message, const void* userParam)
    {
        // ignore non-significant error/warning codes
        if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
        std::string sourceString;
        std::string typeString;
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:
                sourceString = "Source: API";
                break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                sourceString = "Source: Window System";
                break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                sourceString = "Source: Shader Compiler";
                break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                sourceString = "Source: Third Party";
                break;
            case GL_DEBUG_SOURCE_APPLICATION:
                sourceString = "Source: Application";
                break;
            case GL_DEBUG_SOURCE_OTHER:
                sourceString = "Source: Other";
                break;
        }
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:
                typeString = "Type: Error";
                break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                typeString = "Type: Deprecated Behaviour";
                break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                typeString = "Type: Undefined Behaviour";
                break;
            case GL_DEBUG_TYPE_PORTABILITY:
                typeString = "Type: Portability";
                break;
            case GL_DEBUG_TYPE_PERFORMANCE:
                typeString = "Type: Performance";
                break;
            case GL_DEBUG_TYPE_MARKER:
                typeString = "Type: Marker";
                break;
            case GL_DEBUG_TYPE_PUSH_GROUP:
                typeString = "Type: Push Group";
                break;
            case GL_DEBUG_TYPE_POP_GROUP:
                typeString = "Type: Pop Group";
                break;
            case GL_DEBUG_TYPE_OTHER:
                typeString = "Type: Other";
                break;
        }
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                LOG_ERROR ("({}): {}. {}. {}", id, message, sourceString, typeString);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                LOG_WARN  ("({}): {}. {}. {}", id, message, sourceString, typeString);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                LOG_WARN  ("({}): {}. {}. {}", id, message, sourceString, typeString);
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                LOG_INFO  ("({}): {}. {}. {}", id, message, sourceString, typeString);
                break;
        }
    }
}
