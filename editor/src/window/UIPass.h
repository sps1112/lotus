#pragma once

#include <config.h>
#include <ui/panels.hpp>
#include <window/backends/imgui_impl_glfw.h>
#include <window/backends/imgui_impl_opengl3.h>

#include <lotus/rendering/RenderPass.h>

namespace Editor
{
    class UIPass final : public Lotus::Renderer::RenderPass
    {
        GLFWwindow* window;
        ImVec4 clearColor = {0.5f, 0.5f, 0.5f, 1.0f};
        constexpr static bool show = true;

    public:

        explicit UIPass(Window* window_) : window((GLFWwindow*) window_)
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            // Setup Dear ImGui style
            ImGui::StyleColorsLight();

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*) window, true);
            ImGui_ImplOpenGL3_Init(GLSL_VERSION_STRING);

            // Load Fonts
            io.Fonts->AddFontFromFileTTF(fullPath("fonts/OpenSans-Regular.ttf").c_str(), 20.0f);
            io.Fonts->AddFontDefault();
        }

        void RenderFrame(double deltaTime) override
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

//            Editor::Panel::MainMenu();
//            Editor::Panel::MainDockSpace();
            Editor::Panel::DemoWindow();
//            Editor::Panel::Log((bool*) &show);

            // Rendering
            ImGui::Render();
//            int display_w, display_h;
//            glfwGetFramebufferSize(window, &display_w, &display_h);
//            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        ~UIPass() override
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

    private:
        static std::string fullPath(const std::string& path)
        {
            return EDITOR_RESOURCE_ROOT + path;
        }
    };
}