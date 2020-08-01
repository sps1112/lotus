#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lotus/lotus.h"
#include "lotus/debug.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = 400, lastY = 300;

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

/**
 * Print out an error message with file name and line number
 */
GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        LOG_ERROR("{} | {} ({})", error, file, line);
    }
    return errorCode;
}


namespace LR = Lotus::Rendering;

int main()
{
    // Have to initialize the engine first
    Lotus::init();

    // Then choose the renderer and initialize that
    LR::GLRenderer& renderer = LR::GLRenderer::get();
    renderer.init(true);

    glCheckError();

    // Set the viewport dimensions
    renderer.setViewport(0, 0, 800, 600);

    LR::URefWindow& window = renderer.getActiveWindow();

    // Capture mouse
    glfwSetInputMode(window->getGLWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Callback for mouse movements
//    glfwSetCursorPosCallback(window->getGLWindow(), mouse_callback);

    // Callback for scroll
//    glfwSetScrollCallback(window, scroll_callback);

    // Create the shaders to be used
    LR::SRefShader shader = std::make_shared<LR::Shader>(
            "/home/priyansh/code/lotus/examples/quickstart/resources/shaders/standard.vsh",
            "/home/priyansh/code/lotus/examples/quickstart/resources/shaders/diffuse.fsh"
    );

    LR::SRefShader whiteShader = std::make_shared<LR::Shader>(
            "/home/priyansh/code/lotus/examples/quickstart/resources/shaders/standard.vsh",
            "/home/priyansh/code/lotus/examples/quickstart/resources/shaders/emission.fsh"
    );

    LR::SRefShader skyShader = std::make_shared<LR::Shader>(
            "/home/priyansh/code/lotus/examples/quickstart/resources/shaders/sky.vsh",
            "/home/priyansh/code/lotus/examples/quickstart/resources/shaders/sky.fsh"
    );

    // Initialize and import the model
    Lotus::Resource::SRefModel model = std::make_shared<Lotus::Resource::Model>(
            "/home/priyansh/code/lotus/examples/quickstart/resources/mesh/untitled.obj"
    );
    model->import();

    Lotus::Resource::SRefModel planeModel = std::make_shared<Lotus::Resource::Model>(
            "/home/priyansh/code/lotus/examples/quickstart/resources/mesh/plane.obj"
    );
    planeModel->import();

    glCheckError();

    Lotus::Scene scene;

    const std::vector<std::string>& cubemapPaths = {
            "/home/priyansh/code/lotus/examples/quickstart/resources/textures/skybox/right.jpg",
            "/home/priyansh/code/lotus/examples/quickstart/resources/textures/skybox/left.jpg",
            "/home/priyansh/code/lotus/examples/quickstart/resources/textures/skybox/top.jpg",
            "/home/priyansh/code/lotus/examples/quickstart/resources/textures/skybox/bottom.jpg",
            "/home/priyansh/code/lotus/examples/quickstart/resources/textures/skybox/front.jpg",
            "/home/priyansh/code/lotus/examples/quickstart/resources/textures/skybox/back.jpg"
    };

    Lotus::Resource::Cubemap cubemap(cubemapPaths);

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


//    Lotus::SRefSkybox skybox = std::make_shared<Lotus::Skybox>(cubemapPaths, model, skyShader);
//    scene.addSkybox(skybox);

    // Actors
//    Lotus::SRefActor actor = std::make_shared<Lotus::Actor>(ORIGIN, model, shader);
//    scene.addActor(actor);

    Lotus::SRefActor actor2 = std::make_shared<Lotus::Actor>(glm::vec3(2.0f, 2.0f, 0.0f), model, shader);
    scene.addActor(actor2);

    Lotus::SRefActor plane = std::make_shared<Lotus::Actor>(glm::vec3(0.0f, -1.11f, 0.0f), planeModel, shader);
    scene.addActor(plane);

    // Camera
    Lotus::SRefCamera camera = std::make_shared<Lotus::LCamera>(glm::vec3(0.0f, 0.0f, 5.0f));
    scene.addCamera(camera);

    // Lights
    Lotus::CSpotlight light;
    light.diffuse = glm::vec3(0.9375f, 0.99375f, 0.148f); // yellow
    light.ambient = glm::vec3(0.2f);
    light.specular = glm::vec3(0.5f);
    light.constant = 1.0f;
    light.linear = 0.09f;
    light.quadratic = 0.032f;
    light.innerCutOff = glm::cos(glm::radians(30.0f));
    light.outerCutOff = glm::cos(glm::radians(32.5f));

    light.direction = glm::vec3(-0.2f, -1.0f, 0.0f);
    light.position = glm::vec3(3.0f, 5.0f, 0.0f);
    Lotus::SRefALight light3 = std::make_shared<Lotus::ALight>(light, model, whiteShader);
    light3->transform.scale = glm::vec3 (0.2f, 0.2f, 0.2f);
    scene.addLight(light3, ELight::SPOT);

    light.position = glm::vec3(-2.0f, 2.0f, 2.0f);
    Lotus::SRefALight light4 = std::make_shared<Lotus::ALight>(light, model, whiteShader);
    light4->transform.scale = glm::vec3 (0.2f, 0.2f, 0.2f);
    scene.addLight(light4, ELight::POINT);

    // Run the main render loop
    GLFWwindow* pWindow = window->getGLWindow();
    while (!glfwWindowShouldClose(pWindow))
    {
        glCheckError();
        glfwPollEvents();
        if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera->ProcessKeyboard(FORWARD, deltaTime);
        }
        if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera->ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (glfwGetKey(pWindow, GLFW_KEY_E) == GLFW_PRESS)
        {
            camera->ProcessKeyboard(UP, deltaTime);
        }
        if (glfwGetKey(pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        {
            camera->ProcessKeyboard(DOWN, deltaTime);
        }
        if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera->ProcessKeyboard(RIGHT, deltaTime);
        }
        if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera->ProcessKeyboard(LEFT, deltaTime);
        }
        if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(pWindow, true);
        }

        double xpos, ypos;
        glfwGetCursorPos(pWindow, &xpos, &ypos);
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;
        camera->ProcessMouseMovement(xoffset, yoffset);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glCheckError();
        renderer.renderScene(scene);

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera->getFieldOfView()), (float) 800 / (float) 600, 0.1f, 100.0f);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyShader->use();
        skyShader->setInt("skybox", 12);
        view = glm::mat4(glm::mat3(camera->GetViewMatrix())); // remove translation from the view matrix
        skyShader->setMat4fv("view", GL_FALSE, glm::value_ptr(view));
        skyShader->setMat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glCheckError();
        glfwSwapBuffers(pWindow);
    }

    glCheckError();
    renderer.shutdown();
    return 0;
}

/**
 * Function to process and consume input events.
 * @param window: pointer to the active window object
 */
//void process_input(GLFWwindow* window)
//{
//
//}
//
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

}
//
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//    camera.ProcessMouseScroll(yoffset);
//}
