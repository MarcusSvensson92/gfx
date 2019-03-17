//
// Sample code based on https://vulkan-tutorial.com/Depth_buffering
//

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Gfx.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main(int argc, char* argv[])
{
    uint32_t width = 1366;
    uint32_t height = 768;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Textured Quad", nullptr, nullptr);
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

    GfxTechnique tech = GfxLoadTechnique(device, "../Techniques/TexturedQuad.json");

    GfxCreateTextureParams depth_texture_params;
    depth_texture_params.m_Width = width;
    depth_texture_params.m_Height = height;
    depth_texture_params.m_Format = GFX_FORMAT_D32_SFLOAT;
    depth_texture_params.m_Usage = GFX_TEXTURE_USAGE_DEPTH_ATTACHMENT_BIT;
    depth_texture_params.m_InitialState = GFX_TEXTURE_STATE_DEPTH_ATTACHMENT;
    GfxTexture depth_texture = GfxCreateTexture(device, depth_texture_params);

    GfxRenderSetup render_setups[2];
    for (uint32_t i = 0; i < 2; ++i)
    {
        GfxTexture back_buffer = GfxGetBackBuffer(device, i);

        GfxCreateRenderSetupParams render_setup_params;
        render_setup_params.m_ColorAttachmentCount = 1;
        render_setup_params.m_ColorAttachments = &back_buffer;
        render_setup_params.m_DepthAttachment = depth_texture;
        render_setups[i] = GfxCreateRenderSetup(device, tech, render_setup_params);
    }

    GfxTexture albedo_texture = GfxLoadTexture(device, "../Assets/texture.jpg");

    GfxCreateSamplerParams sampler_params;
    sampler_params.m_MagFilter = GFX_FILTER_LINEAR;
    sampler_params.m_MinFilter = GFX_FILTER_LINEAR;
    sampler_params.m_MipFilter = GFX_FILTER_LINEAR;
    sampler_params.m_AddressModeU = GFX_ADDRESS_MODE_CLAMP;
    sampler_params.m_AddressModeV = GFX_ADDRESS_MODE_CLAMP;
    sampler_params.m_AddressModeW = GFX_ADDRESS_MODE_CLAMP;
    GfxSampler linear_clamp = GfxCreateSampler(device, sampler_params);

    struct VertexData
    {
        const glm::vec3 positions[8] =
        {
            glm::vec3(-0.5f, -0.5f, 0.0f),
            glm::vec3( 0.5f, -0.5f, 0.0f),
            glm::vec3( 0.5f,  0.5f, 0.0f),
            glm::vec3(-0.5f,  0.5f, 0.0f),

            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3( 0.5f, -0.5f, -0.5f),
            glm::vec3( 0.5f,  0.5f, -0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),
        };
        const glm::vec2 tex_coords[8] =
        {
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),

            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
        };
    } vertex_data;
    GfxCreateBufferParams buffer_params;
    buffer_params.m_Size = sizeof(vertex_data);
    buffer_params.m_Usage = GFX_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_params.m_Data = &vertex_data;
    GfxBuffer vertex_buffer = GfxCreateBuffer(device, buffer_params);

    const uint16_t indices[12] =
    {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,
    };
    buffer_params.m_Size = sizeof(indices);
    buffer_params.m_Usage = GFX_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buffer_params.m_Data = &indices;
    GfxBuffer index_buffer = GfxCreateBuffer(device, buffer_params);

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

            for (uint32_t i = 0; i < 2; ++i)
            {
                GfxDestroyRenderSetup(device, render_setups[i]);
            }

            GfxDestroyTexture(device, depth_texture);
            depth_texture_params.m_Width = width;
            depth_texture_params.m_Height = height;
            depth_texture = GfxCreateTexture(device, depth_texture_params);

            for (uint32_t i = 0; i < 2; ++i)
            {
                GfxTexture back_buffer = GfxGetBackBuffer(device, i);

                GfxCreateRenderSetupParams render_setup_params;
                render_setup_params.m_ColorAttachmentCount = 1;
                render_setup_params.m_ColorAttachments = &back_buffer;
                render_setup_params.m_DepthAttachment = depth_texture;
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

        {
            GfxCommandBuffer cmd = GfxBeginFrame(device);

            GfxCmdBeginTechnique(cmd, tech);

            const uint32_t back_buffer_index = GfxGetBackBufferIndex(device);
            GfxTexture back_buffer = GfxGetBackBuffer(device, back_buffer_index);
            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_PRESENT, GFX_TEXTURE_STATE_COLOR_ATTACHMENT);

            GfxCmdSetRenderSetup(cmd, render_setups[back_buffer_index]);

            const float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            GfxCmdClearColor(cmd, 0, color);
            GfxCmdClearDepth(cmd, 1.0f, 0);

            GfxCmdBindVertexBuffer(cmd, 0, vertex_buffer, 0);
            GfxCmdBindVertexBuffer(cmd, 1, vertex_buffer, sizeof(VertexData::positions));
            GfxCmdBindIndexBuffer(cmd, index_buffer, 0, sizeof(uint16_t));

            GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), linear_clamp);
            GfxCmdSetTexture(cmd, GFX_HASH("Albedo"), albedo_texture, GFX_TEXTURE_STATE_SHADER_READ);

            static double start_time = glfwGetTime();
            float time = static_cast<float>(glfwGetTime() - start_time);

            struct SConstants
            {
                glm::mat4 World;
                glm::mat4 View;
                glm::mat4 Projection;
            };
            SConstants* constants = static_cast<SConstants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(SConstants)));
            constants->World = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            constants->View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            constants->Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
            constants->Projection[1][1] *= -1.0f;

            GfxCmdDrawIndexed(cmd, 12, 1, 0, 0);

            GfxCmdEndTechnique(cmd);

            GfxCmdTransitionTexture(cmd, back_buffer, GFX_TEXTURE_STATE_COLOR_ATTACHMENT, GFX_TEXTURE_STATE_PRESENT);

            GfxEndFrame(device);
        }
    }

    GfxWaitForGpu(device);

    GfxDestroyBuffer(device, index_buffer);
    GfxDestroyBuffer(device, vertex_buffer);

    GfxDestroySampler(device, linear_clamp);
    GfxDestroyTexture(device, albedo_texture);

    for (uint32_t i = 0; i < 2; ++i)
    {
        GfxDestroyRenderSetup(device, render_setups[i]);
    }
    GfxDestroyTexture(device, depth_texture);

    GfxDestroyTechnique(device, tech);

    GfxDestroyDevice(device);

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}