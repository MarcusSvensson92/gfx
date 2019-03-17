//
// Sample code based on https://vulkan-tutorial.com/Drawing_a_triangle
//

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Gfx.h>

int main(int argc, char* argv[])
{
    uint32_t width  = 1366;
    uint32_t height = 768;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Hello Triangle", nullptr, nullptr);
    glfwHideWindow(window);

    GfxCreateDeviceParams device_params;
#ifdef _WIN32
    device_params.m_WindowHandle = glfwGetWin32Window(window);
#endif
    device_params.m_BackBufferWidth = width;
    device_params.m_BackBufferHeight = height;
    device_params.m_DesiredBackBufferCount = 2;
    device_params.m_EnableValidationLayer = true;
    GfxDevice device = GfxCreateDevice(device_params);

    GfxTechnique tech = GfxLoadTechnique(device, "../Techniques/HelloTriangle.json");

    GfxRenderSetup render_setups[2];
    for (uint32_t i = 0; i < 2; ++i)
    {
        GfxTexture back_buffer = GfxGetBackBuffer(device, i);

        GfxCreateRenderSetupParams render_setup_params;
        render_setup_params.m_ColorAttachmentCount = 1;
        render_setup_params.m_ColorAttachments = &back_buffer;
        render_setups[i] = GfxCreateRenderSetup(device, tech, render_setup_params);
    }

    glfwShowWindow(window);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Window resizing
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);
        if ((window_width != width || window_height != height) && (window_width > 0 && window_height > 0))
        {
            width = static_cast<uint32_t>(window_width);
            height = static_cast<uint32_t>(window_height);

            GfxWaitForGpu(device);
            GfxResizeSwapchain(device, width, height);

            for (uint32_t i = 0; i < 2; ++i)
            {
                GfxDestroyRenderSetup(device, render_setups[i]);
            }

            for (uint32_t i = 0; i < 2; ++i)
            {
                GfxTexture back_buffer = GfxGetBackBuffer(device, i);

                GfxCreateRenderSetupParams render_setup_params;
                render_setup_params.m_ColorAttachmentCount = 1;
                render_setup_params.m_ColorAttachments = &back_buffer;
                render_setups[i] = GfxCreateRenderSetup(device, tech, render_setup_params);
            }
        }

#ifdef _DEBUG
        // Technique reloading
        if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
        {
            GfxWaitForGpu(device);
            GfxReloadAllTechniques(device);
        }
#endif

        // Command buffer generation
        {
            GfxCommandBuffer cmd = GfxBeginFrame(device);

            const uint32_t back_buffer_index = GfxGetBackBufferIndex(device);
            GfxTexture back_buffer = GfxGetBackBuffer(device, back_buffer_index);
            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_PRESENT, GFX_TEXTURE_STATE_COLOR_ATTACHMENT);

            GfxCmdBeginTechnique(cmd, tech);

            GfxCmdSetRenderSetup(cmd, render_setups[back_buffer_index]);

            const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            GfxCmdClearColor(cmd, 0, color);

            GfxCmdDraw(cmd, 3, 1, 0);

            GfxCmdEndTechnique(cmd);

            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_COLOR_ATTACHMENT, GFX_TEXTURE_STATE_PRESENT);

            GfxEndFrame(device);
        }
    }

    GfxWaitForGpu(device);

    for (uint32_t i = 0; i < 2; ++i)
    {
        GfxDestroyRenderSetup(device, render_setups[i]);
    }

    GfxDestroyTechnique(device, tech);
    GfxDestroyDevice(device);

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}