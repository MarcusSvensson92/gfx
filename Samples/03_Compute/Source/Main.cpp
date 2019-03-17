//
// Sample code based on http://wili.cc/blog/opengl-cs
//

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Gfx.h>

int main(int argc, char* argv[])
{
    uint32_t width = 1366;
    uint32_t height = 768;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Compute", nullptr, nullptr);
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
    
    GfxTechnique tech = GfxLoadTechnique(device, "../Techniques/Compute.json");

    GfxCreateTextureParams color_texture_params;
    color_texture_params.m_Width = width;
    color_texture_params.m_Height = height;
    color_texture_params.m_Format = GFX_FORMAT_R8G8B8A8_UNORM;
    color_texture_params.m_Usage = GFX_TEXTURE_USAGE_STORE_BIT;
    color_texture_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_WRITE;
    GfxTexture color_texture = GfxCreateTexture(device, color_texture_params);
    
    glfwShowWindow(window);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    
        // Window resizing
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);
        if ((window_width != width || window_height != height) && (window_width != 0 && window_height != 0))
        {
            width = static_cast<uint32_t>(window_width);
            height = static_cast<uint32_t>(window_height);
    
            GfxWaitForGpu(device);
            GfxResizeSwapchain(device, width, height);
    
            GfxDestroyTexture(device, color_texture);
            color_texture_params.m_Width = width;
            color_texture_params.m_Height = height;
            color_texture = GfxCreateTexture(device, color_texture_params);
        }
    
#ifdef _DEBUG
        // Technique reloading
        if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
        {
            GfxWaitForGpu(device);
            GfxReloadAllTechniques(device);
        }
#endif
    
        {
            GfxCommandBuffer cmd = GfxBeginFrame(device);
    
            GfxCmdBeginTechnique(cmd, tech);

            GfxCmdSetTexture(cmd, GFX_HASH("ColorTexture"), color_texture, GFX_TEXTURE_STATE_SHADER_WRITE);

            static double start_time = glfwGetTime();
            float time = static_cast<float>(glfwGetTime() - start_time);

            struct SConstants
            {
                float       m_Roll;
            };
            SConstants* constants = static_cast<SConstants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(SConstants)));
            constants->m_Roll = time;
    
            GfxCmdDispatch(cmd, (width + 8 - 1) / 8, (height + 8 - 1) / 8, 1);
    
            GfxCmdEndTechnique(cmd);

            const uint32_t back_buffer_index = GfxGetBackBufferIndex(device);
            GfxTexture back_buffer = GfxGetBackBuffer(device, back_buffer_index);

            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_PRESENT, GFX_TEXTURE_STATE_COPY_DST);
            GfxCmdTransitionTexture(cmd, color_texture, GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_COPY_SRC);

            GfxCmdBlitTexture(cmd, back_buffer, color_texture);

            GfxCmdTransitionTexture(cmd, color_texture, GFX_TEXTURE_STATE_COPY_SRC, GFX_TEXTURE_STATE_SHADER_WRITE);
            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_COPY_DST, GFX_TEXTURE_STATE_PRESENT);
    
            GfxEndFrame(device);
        }
    }
    
    GfxWaitForGpu(device);
    
    GfxDestroyTexture(device, color_texture);
    
    GfxDestroyTechnique(device, tech);
    
    GfxDestroyDevice(device);

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}