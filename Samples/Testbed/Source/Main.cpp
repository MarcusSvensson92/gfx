#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Context.h"
#include "CameraController.h"

#include "Atmosphere.h"
#include "Lighting.h"
#include "PostProcessEffects.h"

#include "ImGuiImpl.h"

static void GlfwMinimizeCallback(GLFWwindow* window, int minimized)
{
    Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
    ctx->m_Minimized = minimized == GLFW_TRUE;
}

int main(int argc, char* argv[])
{
    Context ctx;
    ctx.m_Width = 1366;
    ctx.m_Height = 768;
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ctx.m_Window = glfwCreateWindow(ctx.m_Width, ctx.m_Height, "Testbed", nullptr, nullptr);
    glfwHideWindow(ctx.m_Window);

    glfwSetWindowUserPointer(ctx.m_Window, &ctx);

    ctx.m_Minimized = false;
    glfwSetWindowIconifyCallback(ctx.m_Window, GlfwMinimizeCallback);
    
    GfxCreateDeviceParams device_params;
#ifdef _WIN32
    device_params.m_WindowHandle = glfwGetWin32Window(ctx.m_Window);
#endif
    device_params.m_BackBufferWidth = ctx.m_Width;
    device_params.m_BackBufferHeight = ctx.m_Height;
    device_params.m_DesiredBackBufferCount = 2;
    device_params.m_EnableValidationLayer = false;
    ctx.m_Device = GfxCreateDevice(device_params);

    GfxCreateTextureParams color_buffer_params;
    color_buffer_params.m_Width = ctx.m_Width;
    color_buffer_params.m_Height = ctx.m_Height;
    color_buffer_params.m_Usage = GFX_TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
    color_buffer_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
    color_buffer_params.m_Format = GFX_FORMAT_R11G11B10_UFLOAT;
    ctx.m_ColorBuffer = GfxCreateTexture(ctx.m_Device, color_buffer_params);

    GfxCreateTextureParams depth_buffer_params;
    depth_buffer_params.m_Width = ctx.m_Width;
    depth_buffer_params.m_Height = ctx.m_Height;
    depth_buffer_params.m_Format = GFX_FORMAT_D32_SFLOAT;
    depth_buffer_params.m_Usage = GFX_TEXTURE_USAGE_DEPTH_ATTACHMENT_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
    depth_buffer_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
    ctx.m_DepthBuffer = GfxCreateTexture(ctx.m_Device, depth_buffer_params);

    ctx.m_Camera.LookAt(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    ctx.m_Camera.Perspective(glm::radians(75.0f), static_cast<float>(ctx.m_Width), static_cast<float>(ctx.m_Height), 0.1f, 1000.0f);

    Atmosphere atmosphere;
    atmosphere.Init(ctx);

    Lighting lighting;
    lighting.Init(ctx);

    PostProcessEffects post_process_effects;
    post_process_effects.Init(ctx);

    ImGuiImpl imgui_impl;
    imgui_impl.Init(ctx);

    GfxModel sponza = GfxLoadModel(ctx.m_Device, "../Assets/sponza.obj", "../Assets/");

    CameraController camera_controller;

    glfwShowWindow(ctx.m_Window);
    while (!glfwWindowShouldClose(ctx.m_Window))
    {
        glfwPollEvents();

        if (ctx.m_Minimized)
            continue;

        static double last_time = glfwGetTime();
        double time = glfwGetTime();
        float dt = static_cast<float>(time - last_time);
        last_time = time;

        camera_controller.Update(ctx, dt);

        // Window resizing
        int window_width, window_height;
        glfwGetWindowSize(ctx.m_Window, &window_width, &window_height);
        if ((window_width != ctx.m_Width || window_height != ctx.m_Height) && (window_width != 0 && window_height != 0))
        {
            ctx.m_Width = static_cast<uint32_t>(window_width);
            ctx.m_Height = static_cast<uint32_t>(window_height);

            GfxWaitForGpu(ctx.m_Device);
            GfxResizeSwapchain(ctx.m_Device, ctx.m_Width, ctx.m_Height);

            GfxDestroyTexture(ctx.m_Device, ctx.m_ColorBuffer);
            color_buffer_params.m_Width = ctx.m_Width;
            color_buffer_params.m_Height = ctx.m_Height;
            color_buffer_params.m_Format = GFX_FORMAT_R11G11B10_UFLOAT;
            ctx.m_ColorBuffer = GfxCreateTexture(ctx.m_Device, color_buffer_params);

            GfxDestroyTexture(ctx.m_Device, ctx.m_DepthBuffer);
            depth_buffer_params.m_Width = ctx.m_Width;
            depth_buffer_params.m_Height = ctx.m_Height;
            ctx.m_DepthBuffer = GfxCreateTexture(ctx.m_Device, depth_buffer_params);

            atmosphere.Resize(ctx);
            lighting.Resize(ctx);
            post_process_effects.Resize(ctx);
            imgui_impl.Resize(ctx);

            ctx.m_Camera.Perspective(glm::radians(75.0f), static_cast<float>(ctx.m_Width), static_cast<float>(ctx.m_Height), 0.1f, 1000.0f);
        }

#ifdef _DEBUG
        // Technique reloading
        if (glfwGetKey(ctx.m_Window, GLFW_KEY_F5) == GLFW_PRESS)
        {
            GfxWaitForGpu(ctx.m_Device);
            GfxReloadAllTechniques(ctx.m_Device);
        }
#endif

        {
            imgui_impl.Update(ctx);

            ImGui::StyleColorsDark();
            ImGui::NewFrame();

            const float menu_width = 384.0f;
            ImGui::SetNextWindowSize(ImVec2(menu_width, static_cast<float>(ctx.m_Height)));
            ImGui::SetNextWindowPos(ImVec2(static_cast<float>(ctx.m_Width) - menu_width, 0.0f));

            ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            if (ImGui::CollapsingHeader("Atmospheric Scattering", nullptr, true, false))
            {
                ImGui::SliderFloat("Time Of Day", &atmosphere.m_TimeOfDay, 0.f, 24.f);
            }

            if (ImGui::CollapsingHeader("Lighting", nullptr, true, false))
            {
                ImGui::InputFloat("Ambient Light", &lighting.m_AmbientLightIntensity);
                ImGui::InputFloat("Dir Light", &lighting.m_DirectionalLightIntensity);
            }

            if (ImGui::CollapsingHeader("Post Process Effects", nullptr, true, false))
            {
                ImGui::InputFloat("Exposure", &post_process_effects.m_Exposure);
                ImGui::Checkbox("Enable TAA", &post_process_effects.m_TemporalAAEnable);
            }

            ImGui::End();

            ImGui::Render();
        }

        {
            atmosphere.Update(ctx);
            post_process_effects.Update(ctx);
        }

        {
            GfxCommandBuffer cmd = GfxBeginFrame(ctx.m_Device);

            atmosphere.Precompute(ctx, cmd);

            GfxCmdTransitionTexture(cmd, ctx.m_ColorBuffer, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_COLOR_ATTACHMENT);
            GfxCmdTransitionTexture(cmd, ctx.m_DepthBuffer, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_DEPTH_ATTACHMENT);

            lighting.BeginDraw(cmd);

            float clear_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            GfxCmdClearColor(cmd, 0, clear_color);
            GfxCmdClearDepth(cmd, 1.0f, 0);

            const glm::mat4 world = glm::scale(glm::vec3(1e-2f));
            lighting.Draw(ctx, cmd, sponza, world);

            lighting.EndDraw(cmd);

            atmosphere.DrawSky(ctx, cmd);

            GfxCmdTransitionTexture(cmd, ctx.m_ColorBuffer, GFX_TEXTURE_STATE_COLOR_ATTACHMENT, GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdTransitionTexture(cmd, ctx.m_DepthBuffer, GFX_TEXTURE_STATE_DEPTH_ATTACHMENT, GFX_TEXTURE_STATE_SHADER_READ);

            const uint32_t back_buffer_index = GfxGetBackBufferIndex(ctx.m_Device);
            GfxTexture back_buffer = GfxGetBackBuffer(ctx.m_Device, back_buffer_index);
            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_PRESENT, GFX_TEXTURE_STATE_COLOR_ATTACHMENT);

            post_process_effects.Draw(ctx, cmd);

            imgui_impl.Draw(ctx, cmd);

            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_COLOR_ATTACHMENT, GFX_TEXTURE_STATE_PRESENT);

            GfxEndFrame(ctx.m_Device);
        }
    }

    GfxWaitForGpu(ctx.m_Device);

    GfxDestroyTexture(ctx.m_Device, ctx.m_ColorBuffer);
    GfxDestroyTexture(ctx.m_Device, ctx.m_DepthBuffer);

    GfxDestroyModel(ctx.m_Device, sponza);

    atmosphere.Destroy(ctx);
    lighting.Destroy(ctx);
    post_process_effects.Destroy(ctx);
    imgui_impl.Destroy(ctx);

    GfxDestroyDevice(ctx.m_Device);

    glfwDestroyWindow(ctx.m_Window);
    glfwTerminate();

	return 0;
}
