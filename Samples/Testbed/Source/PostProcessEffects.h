#ifndef POST_PROCESS_EFFECTS_H
#define POST_PROCESS_EFFECTS_H

#include "Context.h"

class PostProcessEffects
{
public:
    GfxTechnique                m_TechTemporalAA                = NULL;
    GfxTechnique                m_TechToneMapping               = NULL;

    GfxTexture                  m_TemporalBuffers[2]            = { NULL, NULL };

    GfxRenderSetup              m_ToneMappingRenderSetups[2]    = { NULL, NULL };

    GfxSampler                  m_NearestClamp                  = NULL;
    GfxSampler                  m_LinearClamp                   = NULL;

    float                       m_Exposure                      = 2.0f;

    bool                        m_TemporalAAEnable              = true;
    uint64_t                    m_TemporalAAFrameCounter        = 0;
    glm::mat4                   m_TemporalAAPrevViewProj        = glm::identity<glm::mat4>();

    void Init(const Context& ctx)
    {
        m_TechTemporalAA = GfxLoadTechnique(ctx.m_Device, "../Techniques/TemporalAntiAliasing.json");
        m_TechToneMapping = GfxLoadTechnique(ctx.m_Device, "../Techniques/ToneMapping.json");

        GfxCreateSamplerParams sampler_params;
        sampler_params.m_MagFilter = GFX_FILTER_NEAREST;
        sampler_params.m_MinFilter = GFX_FILTER_NEAREST;
        sampler_params.m_MipFilter = GFX_FILTER_NEAREST;
        sampler_params.m_AddressModeU = GFX_ADDRESS_MODE_CLAMP;
        sampler_params.m_AddressModeV = GFX_ADDRESS_MODE_CLAMP;
        sampler_params.m_AddressModeW = GFX_ADDRESS_MODE_CLAMP;
        m_NearestClamp = GfxCreateSampler(ctx.m_Device, sampler_params);
        sampler_params.m_MagFilter = GFX_FILTER_LINEAR;
        sampler_params.m_MinFilter = GFX_FILTER_LINEAR;
        sampler_params.m_MipFilter = GFX_FILTER_LINEAR;
        m_LinearClamp = GfxCreateSampler(ctx.m_Device, sampler_params);

        Resize(ctx);
    }
    void Resize(const Context& ctx)
    {
        if (m_TemporalBuffers[0])
            GfxDestroyTexture(ctx.m_Device, m_TemporalBuffers[0]);
        if (m_TemporalBuffers[1])
            GfxDestroyTexture(ctx.m_Device, m_TemporalBuffers[1]);

        GfxCreateTextureParams temporal_color_params;
        temporal_color_params.m_Width = ctx.m_Width;
        temporal_color_params.m_Height = ctx.m_Height;
        temporal_color_params.m_Usage = GFX_TEXTURE_USAGE_STORE_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
        temporal_color_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
        temporal_color_params.m_Format = GFX_FORMAT_R16G16B16A16_SFLOAT;
        m_TemporalBuffers[0] = GfxCreateTexture(ctx.m_Device, temporal_color_params);
        m_TemporalBuffers[1] = GfxCreateTexture(ctx.m_Device, temporal_color_params);

        for (uint32_t i = 0; i < 2; ++i)
        {
            if (m_ToneMappingRenderSetups[i])
                GfxDestroyRenderSetup(ctx.m_Device, m_ToneMappingRenderSetups[i]);
        }

        for (uint32_t i = 0; i < 2; ++i)
        {
            GfxTexture backbuffer = GfxGetBackBuffer(ctx.m_Device, i);

            GfxCreateRenderSetupParams render_setup_params;
            render_setup_params.m_ColorAttachmentCount = 1;
            render_setup_params.m_ColorAttachments = &backbuffer;
            m_ToneMappingRenderSetups[i] = GfxCreateRenderSetup(ctx.m_Device, m_TechToneMapping, render_setup_params);
        }
    }
    void Destroy(const Context& ctx)
    {
        GfxDestroySampler(ctx.m_Device, m_NearestClamp);
        GfxDestroySampler(ctx.m_Device, m_LinearClamp);
        for (uint32_t i = 0; i < 2; ++i)
        {
            GfxDestroyRenderSetup(ctx.m_Device, m_ToneMappingRenderSetups[i]);
        }
        GfxDestroyTexture(ctx.m_Device, m_TemporalBuffers[0]);
        GfxDestroyTexture(ctx.m_Device, m_TemporalBuffers[1]);
        GfxDestroyTechnique(ctx.m_Device, m_TechToneMapping);
        GfxDestroyTechnique(ctx.m_Device, m_TechTemporalAA);
    }

    void Update(Context& ctx)
    {
        m_TemporalAAFrameCounter++;

        float jitter_x = 0.0f;
        float jitter_y = 0.0f;
        if (m_TemporalAAEnable)
        {
            const float halton_23[8][2] =
            {
                { 0.0f / 8.0f, 0.0f / 9.0f }, { 4.0f / 8.0f, 3.0f / 9.0f },
                { 2.0f / 8.0f, 6.0f / 9.0f }, { 6.0f / 8.0f, 1.0f / 9.0f },
                { 1.0f / 8.0f, 4.0f / 9.0f }, { 5.0f / 8.0f, 7.0f / 9.0f },
                { 3.0f / 8.0f, 2.0f / 9.0f }, { 7.0f / 8.0f, 5.0f / 9.0f },
            };
            const float* jitter = halton_23[m_TemporalAAFrameCounter % 8];
            jitter_x = jitter[0];
            jitter_y = jitter[1];
        }
        ctx.m_Camera.ApplyJitter(jitter_x, jitter_y);
    }

    void Draw(const Context& ctx, GfxCommandBuffer cmd)
    {
        if (m_TemporalAAEnable)
        {
            GfxCmdTransitionTexture(cmd, m_TemporalBuffers[m_TemporalAAFrameCounter & 1], GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_SHADER_WRITE);

            GfxCmdBeginTechnique(cmd, m_TechTemporalAA);

            GfxCmdSetTexture(cmd, GFX_HASH("OutTemporal"), m_TemporalBuffers[m_TemporalAAFrameCounter & 1], GFX_TEXTURE_STATE_SHADER_WRITE);
            GfxCmdSetTexture(cmd, GFX_HASH("Temporal"), m_TemporalBuffers[(m_TemporalAAFrameCounter + 1) & 1], GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdSetTexture(cmd, GFX_HASH("Color"), ctx.m_ColorBuffer, GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdSetTexture(cmd, GFX_HASH("Depth"), ctx.m_DepthBuffer, GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), m_LinearClamp);
            GfxCmdSetSampler(cmd, GFX_HASH("NearestClamp"), m_NearestClamp);

            const glm::mat4 curr_view_proj = ctx.m_Camera.m_UnjitteredProjection * ctx.m_Camera.m_View;

            struct Constants
            {
                glm::mat4   m_InvViewProj;
                glm::mat4   m_PrevViewProj;
                float       m_Exposure;
            };
            Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
            constants->m_InvViewProj = glm::inverse(curr_view_proj);
            constants->m_PrevViewProj = m_TemporalAAPrevViewProj;
            constants->m_Exposure = m_Exposure;

            m_TemporalAAPrevViewProj = curr_view_proj;

            const uint32_t group_count_x = (ctx.m_Width + 8 - 1) / 8;
            const uint32_t group_count_y = (ctx.m_Height + 8 - 1) / 8;
            GfxCmdDispatch(cmd, group_count_x, group_count_y, 1);

            GfxCmdEndTechnique(cmd);

            GfxCmdTransitionTexture(cmd, m_TemporalBuffers[m_TemporalAAFrameCounter & 1], GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_SHADER_READ);
        }

        {
            GfxCmdBeginTechnique(cmd, m_TechToneMapping);

            const uint32_t back_buffer_index = GfxGetBackBufferIndex(ctx.m_Device);
            GfxCmdSetRenderSetup(cmd, m_ToneMappingRenderSetups[back_buffer_index]);

            GfxCmdSetTexture(cmd, GFX_HASH("Source"), m_TemporalAAEnable ? m_TemporalBuffers[m_TemporalAAFrameCounter & 1] : ctx.m_ColorBuffer, GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdSetSampler(cmd, GFX_HASH("NearestClamp"), m_NearestClamp);

            struct Constants
            {
                float       m_Exposure;
                uint32_t    m_ResolveTemporal;
            };
            Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
            constants->m_Exposure = m_Exposure;
            constants->m_ResolveTemporal = m_TemporalAAEnable ? 1 : 0;

            GfxCmdDraw(cmd, 3, 1, 0);

            GfxCmdEndTechnique(cmd);
        }
    }
};

#endif