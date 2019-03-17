#ifndef LIGHTING_H
#define LIGHTING_H

#include "Context.h"

class Lighting
{
public:
    GfxTechnique                m_Tech                      = NULL;
    GfxSampler                  m_LinearClamp               = NULL;
    GfxRenderSetup              m_RenderSetup               = NULL;

    float                       m_AmbientLightIntensity     = 0.20f;
    float                       m_DirectionalLightIntensity = 0.02f;

    void Init(const Context& ctx)
    {
        m_Tech = GfxLoadTechnique(ctx.m_Device, "../Techniques/Lighting.json");

        GfxCreateSamplerParams sampler_params;
        sampler_params.m_MagFilter = GFX_FILTER_LINEAR;
        sampler_params.m_MinFilter = GFX_FILTER_LINEAR;
        sampler_params.m_MipFilter = GFX_FILTER_LINEAR;
        sampler_params.m_AddressModeU = GFX_ADDRESS_MODE_WRAP;
        sampler_params.m_AddressModeV = GFX_ADDRESS_MODE_WRAP;
        sampler_params.m_AddressModeW = GFX_ADDRESS_MODE_CLAMP;
        m_LinearClamp = GfxCreateSampler(ctx.m_Device, sampler_params);

        Resize(ctx);
    }
    void Resize(const Context& ctx)
    {
        if (m_RenderSetup)
            GfxDestroyRenderSetup(ctx.m_Device, m_RenderSetup);
        GfxCreateRenderSetupParams render_setup_params;
        render_setup_params.m_ColorAttachmentCount = 1;
        render_setup_params.m_ColorAttachments = &ctx.m_ColorBuffer;
        render_setup_params.m_DepthAttachment = ctx.m_DepthBuffer;
        m_RenderSetup = GfxCreateRenderSetup(ctx.m_Device, m_Tech, render_setup_params);
    }
    void Destroy(const Context& ctx)
    {
        GfxDestroyRenderSetup(ctx.m_Device, m_RenderSetup);
        GfxDestroySampler(ctx.m_Device, m_LinearClamp);
        GfxDestroyTechnique(ctx.m_Device, m_Tech);
    }

    void BeginDraw(GfxCommandBuffer cmd)
    {
        GfxCmdBeginTechnique(cmd, m_Tech);
        GfxCmdSetRenderSetup(cmd, m_RenderSetup);
    }
    void EndDraw(GfxCommandBuffer cmd)
    {
        GfxCmdEndTechnique(cmd);
    }

    void Draw(const Context& ctx, GfxCommandBuffer cmd, GfxModel model, const glm::mat4& world)
    {
        GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), m_LinearClamp);
        GfxCmdSetTexture(cmd, GFX_HASH("AmbientLightLUT"), ctx.m_AtmosphereAmbientLightLUT, GFX_TEXTURE_STATE_SHADER_READ);
        GfxCmdSetTexture(cmd, GFX_HASH("DirectionalLightLUT"), ctx.m_AtmosphereDirectionalLightLUT, GFX_TEXTURE_STATE_SHADER_READ);

        GfxCmdBindModelVertexBuffer(cmd, model, GFX_MODEL_VERTEX_ATTRIBUTE_POSITION, 0);
        GfxCmdBindModelVertexBuffer(cmd, model, GFX_MODEL_VERTEX_ATTRIBUTE_TEXCOORD, 1);
        GfxCmdBindModelVertexBuffer(cmd, model, GFX_MODEL_VERTEX_ATTRIBUTE_NORMAL, 2);
        GfxCmdBindModelIndexBuffer(cmd, model);

        const glm::mat4 view_proj = ctx.m_Camera.m_Projection * ctx.m_Camera.m_View;
        
        const uint32_t mesh_count = GfxGetModelMeshCount(model);
        for (uint32_t i = 0; i < mesh_count; ++i)
        {
            uint32_t material_index = GfxGetModelMaterialIndex(model, i);
            GfxTexture diffuse_texture = GfxGetModelDiffuseTexture(model, material_index);
            if (!diffuse_texture)
                continue;
            GfxCmdSetTexture(cmd, GFX_HASH("Diffuse"), diffuse_texture, GFX_TEXTURE_STATE_SHADER_READ);
        
            struct Constants
            {
                glm::mat4   m_World;
                glm::mat4   m_WorldViewProj;
                glm::vec3   m_ViewPosition;
                float       m_PositionScale;
                glm::vec3   m_LightDirection;
                float       m_LightCoord;
                float       m_AmbientLightIntensity;
                float       m_DirectionalLightIntensity;
            };
            Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
            constants->m_World = world;
            constants->m_WorldViewProj = view_proj * world;
            constants->m_ViewPosition = ctx.m_Camera.m_Position;
            constants->m_PositionScale = GfxGetModelQuantizationScale(model);
            constants->m_LightDirection = ctx.m_AtmosphereLightDirection;
            constants->m_LightCoord = ctx.m_AtmosphereLightCoord;
            constants->m_AmbientLightIntensity = m_AmbientLightIntensity;
            constants->m_DirectionalLightIntensity = m_DirectionalLightIntensity;
        
            GfxCmdDrawModel(cmd, model, i, 1);
        }
    }
};

#endif