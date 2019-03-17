#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "Context.h"

class Atmosphere
{
public:
    GfxTechnique            m_TechPrecomputeDensityLUT          = NULL;
    GfxTechnique            m_TechPrecomputeAmbientLightLUT     = NULL;
    GfxTechnique            m_TechPrecomputeDirectionalLightLUT = NULL;
    GfxTechnique            m_TechPrecomputeSkyLUT              = NULL;
    GfxTechnique            m_TechSky                           = NULL;

    static const uint32_t   m_DensityLUTSizeX                   = 1024;
    static const uint32_t   m_DensityLUTSizeY                   = 1024;
    GfxTexture              m_DensityLUT                        = NULL;

    static const uint32_t   m_AmbientLightLUTSize               = 256;
    GfxTexture              m_AmbientLightLUT                   = NULL;

    static const uint32_t   m_DirectionalLightLUTSize           = 256;
    GfxTexture              m_DirectionalLightLUT               = NULL;

    static const uint32_t   m_SkyLUTSizeX                       = 32;
    static const uint32_t   m_SkyLUTSizeY                       = 128;
    static const uint32_t   m_SkyLUTSizeZ                       = 32;
    GfxTexture              m_SkyLUTR                           = NULL;
    GfxTexture              m_SkyLUTM                           = NULL;

    GfxSampler              m_LinearClamp                       = NULL;

    GfxRenderSetup          m_SkyRenderSetup                    = NULL;

    float                   m_TimeOfDay                         = 12.f;
    float                   m_SunIntensity                      = 20.f;
    glm::vec3               m_BetaR                             = glm::vec3(6.55e-6f, 1.73e-5f, 2.30e-5f);
    glm::vec3               m_BetaM                             = glm::vec3(2e-6f, 2e-6f, 2e-6f);
    glm::vec2               m_DensityScaleHeightRM              = glm::vec2(8000.f, 1200.f);
    float                   m_MieG                              = 0.8f;
    float                   m_PlanetRadius                      = 6360e3f;
    float                   m_AtmosphereRadius                  = 6420e3f;
    float                   m_HeightAboveGround                 = 500.f;

    void Init(Context& ctx)
    {
        m_TechPrecomputeDensityLUT = GfxLoadTechnique(ctx.m_Device, "../Techniques/AtmospherePrecomputeDensityLUT.json");
        m_TechPrecomputeAmbientLightLUT = GfxLoadTechnique(ctx.m_Device, "../Techniques/AtmospherePrecomputeAmbientLightLUT.json");
        m_TechPrecomputeDirectionalLightLUT = GfxLoadTechnique(ctx.m_Device, "../Techniques/AtmospherePrecomputeDirectionalLightLUT.json");
        m_TechPrecomputeSkyLUT = GfxLoadTechnique(ctx.m_Device, "../Techniques/AtmospherePrecomputeSkyLUT.json");
        m_TechSky = GfxLoadTechnique(ctx.m_Device, "../Techniques/AtmosphereSky.json");

        GfxCreateTextureParams density_lut_params;
        density_lut_params.m_Type = GFX_TEXTURE_TYPE_2D;
        density_lut_params.m_Width = m_DensityLUTSizeX;
        density_lut_params.m_Height = m_DensityLUTSizeY;
        density_lut_params.m_Format = GFX_FORMAT_R32G32_SFLOAT;
        density_lut_params.m_Usage = GFX_TEXTURE_USAGE_STORE_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
        density_lut_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
        m_DensityLUT = GfxCreateTexture(ctx.m_Device, density_lut_params);

        GfxCreateTextureParams ambient_light_lut_params;
        ambient_light_lut_params.m_Type = GFX_TEXTURE_TYPE_1D;
        ambient_light_lut_params.m_Width = m_AmbientLightLUTSize;
        ambient_light_lut_params.m_Format = GFX_FORMAT_R32G32B32A32_SFLOAT;
        ambient_light_lut_params.m_Usage = GFX_TEXTURE_USAGE_STORE_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
        ambient_light_lut_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
        m_AmbientLightLUT = GfxCreateTexture(ctx.m_Device, ambient_light_lut_params);

        GfxCreateTextureParams directional_light_lut_params;
        directional_light_lut_params.m_Type = GFX_TEXTURE_TYPE_1D;
        directional_light_lut_params.m_Width = m_DirectionalLightLUTSize;
        directional_light_lut_params.m_Format = GFX_FORMAT_R32G32B32A32_SFLOAT;
        directional_light_lut_params.m_Usage = GFX_TEXTURE_USAGE_STORE_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
        directional_light_lut_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
        m_DirectionalLightLUT = GfxCreateTexture(ctx.m_Device, directional_light_lut_params);

        GfxCreateTextureParams sky_lut_params;
        sky_lut_params.m_Type = GFX_TEXTURE_TYPE_3D;
        sky_lut_params.m_Width = m_SkyLUTSizeX;
        sky_lut_params.m_Height = m_SkyLUTSizeY;
        sky_lut_params.m_Depth = m_SkyLUTSizeZ;
        sky_lut_params.m_Format = GFX_FORMAT_R32G32B32A32_SFLOAT;
        sky_lut_params.m_Usage = GFX_TEXTURE_USAGE_STORE_BIT | GFX_TEXTURE_USAGE_SAMPLE_BIT;
        sky_lut_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
        m_SkyLUTR = GfxCreateTexture(ctx.m_Device, sky_lut_params);
        m_SkyLUTM = GfxCreateTexture(ctx.m_Device, sky_lut_params);

        GfxCreateSamplerParams linear_clamp_params;
        linear_clamp_params.m_MagFilter = GFX_FILTER_LINEAR;
        linear_clamp_params.m_MinFilter = GFX_FILTER_LINEAR;
        linear_clamp_params.m_MipFilter = GFX_FILTER_LINEAR;
        linear_clamp_params.m_AddressModeU = GFX_ADDRESS_MODE_CLAMP;
        linear_clamp_params.m_AddressModeV = GFX_ADDRESS_MODE_CLAMP;
        linear_clamp_params.m_AddressModeW = GFX_ADDRESS_MODE_CLAMP;
        m_LinearClamp = GfxCreateSampler(ctx.m_Device, linear_clamp_params);

        Resize(ctx);

        ctx.m_AtmosphereAmbientLightLUT = m_AmbientLightLUT;
        ctx.m_AtmosphereDirectionalLightLUT = m_DirectionalLightLUT;
    }
    void Resize(const Context& ctx)
    {
        if (m_SkyRenderSetup)
            GfxDestroyRenderSetup(ctx.m_Device, m_SkyRenderSetup);
        GfxCreateRenderSetupParams sky_render_setup_params;
        sky_render_setup_params.m_ColorAttachmentCount = 1;
        sky_render_setup_params.m_ColorAttachments = &ctx.m_ColorBuffer;
        sky_render_setup_params.m_DepthAttachment = ctx.m_DepthBuffer;
        m_SkyRenderSetup = GfxCreateRenderSetup(ctx.m_Device, m_TechSky, sky_render_setup_params);
    }
    void Destroy(const Context& ctx)
    {
        GfxDestroyRenderSetup(ctx.m_Device, m_SkyRenderSetup);

        GfxDestroySampler(ctx.m_Device, m_LinearClamp);
        GfxDestroyTexture(ctx.m_Device, m_SkyLUTM);
        GfxDestroyTexture(ctx.m_Device, m_SkyLUTR);
        GfxDestroyTexture(ctx.m_Device, m_DirectionalLightLUT);
        GfxDestroyTexture(ctx.m_Device, m_AmbientLightLUT);
        GfxDestroyTexture(ctx.m_Device, m_DensityLUT);

        GfxDestroyTechnique(ctx.m_Device, m_TechSky);
        GfxDestroyTechnique(ctx.m_Device, m_TechPrecomputeSkyLUT);
        GfxDestroyTechnique(ctx.m_Device, m_TechPrecomputeDirectionalLightLUT);
        GfxDestroyTechnique(ctx.m_Device, m_TechPrecomputeAmbientLightLUT);
        GfxDestroyTechnique(ctx.m_Device, m_TechPrecomputeDensityLUT);
    }

    void Update(Context& ctx)
    {
        // Calculate light direction
        const glm::vec3 sun_axis_vector = glm::vec3(0.f, -0.7809f, 0.6247f);
        const glm::vec3 sun_apex_vector = glm::vec3(0.f, 0.7071f, 0.7071f);
        float angle = (3.14159264f / 12.f) * (12.f - m_TimeOfDay);
        glm::mat3 rotation_matrix = glm::mat3(glm::rotate(angle, sun_axis_vector));
        ctx.m_AtmosphereLightDirection = -glm::normalize(rotation_matrix * sun_apex_vector);
        ctx.m_AtmosphereLightCoord = glm::dot(glm::vec3(0.f, 1.f, 0.f), -ctx.m_AtmosphereLightDirection) * 0.5f + 0.5f;
    }

    void Precompute(const Context& ctx, GfxCommandBuffer cmd)
    {
        static bool precompute = true;
        if (precompute)
        {
            precompute = false;

            // Precompute density LUT
            {
                GfxCmdBeginTechnique(cmd, m_TechPrecomputeDensityLUT);

                GfxCmdTransitionTexture(cmd, m_DensityLUT, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_SHADER_WRITE);
                GfxCmdSetTexture(cmd, GFX_HASH("DensityLUT"), m_DensityLUT, GFX_TEXTURE_STATE_SHADER_WRITE);

                struct Constants
                {
                    glm::vec2   m_DensityScaleHeightRM;
                    float       m_PlanetRadius;
                    float       m_AtmosphereRadius;
                };
                Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
                constants->m_DensityScaleHeightRM = m_DensityScaleHeightRM;
                constants->m_PlanetRadius = m_PlanetRadius;
                constants->m_AtmosphereRadius = m_AtmosphereRadius;

                const uint32_t group_count_x = (m_DensityLUTSizeX + 8 - 1) / 8;
                const uint32_t group_count_y = (m_DensityLUTSizeY + 8 - 1) / 8;
                GfxCmdDispatch(cmd, group_count_x, group_count_y, 1);

                GfxCmdTransitionTexture(cmd, m_DensityLUT, GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_SHADER_READ);

                GfxCmdEndTechnique(cmd);
            }

            // Precompute ambient light LUT
            {
                GfxCmdBeginTechnique(cmd, m_TechPrecomputeAmbientLightLUT);

                GfxCmdTransitionTexture(cmd, m_AmbientLightLUT, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_SHADER_WRITE);
                GfxCmdSetTexture(cmd, GFX_HASH("AmbientLightLUT"), m_AmbientLightLUT, GFX_TEXTURE_STATE_SHADER_WRITE);

                GfxCmdSetTexture(cmd, GFX_HASH("DensityLUT"), m_DensityLUT, GFX_TEXTURE_STATE_SHADER_READ);
                GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), m_LinearClamp);

                struct Constants
                {
                    glm::vec3   m_BetaR;
                    float       m_PlanetRadius;
                    glm::vec3   m_BetaM;
                    float       m_AtmosphereRadius;
                    glm::vec2   m_DensityScaleHeightRM;
                    float       m_MieG;
                    float       m_SunIntensity;
                };
                Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
                constants->m_BetaR = m_BetaR;
                constants->m_PlanetRadius = m_PlanetRadius;
                constants->m_BetaM = m_BetaM;
                constants->m_AtmosphereRadius = m_AtmosphereRadius;
                constants->m_DensityScaleHeightRM = m_DensityScaleHeightRM;
                constants->m_MieG = m_MieG;
                constants->m_SunIntensity = m_SunIntensity;

                const uint32_t group_count_x = (m_AmbientLightLUTSize + 32 - 1) / 32;
                GfxCmdDispatch(cmd, group_count_x, 1, 1);

                GfxCmdTransitionTexture(cmd, m_AmbientLightLUT, GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_SHADER_READ);

                GfxCmdEndTechnique(cmd);
            }

            // Precompute directional light LUT
            {
                GfxCmdBeginTechnique(cmd, m_TechPrecomputeDirectionalLightLUT);

                GfxCmdTransitionTexture(cmd, m_DirectionalLightLUT, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_SHADER_WRITE);
                GfxCmdSetTexture(cmd, GFX_HASH("DirectionalLightLUT"), m_DirectionalLightLUT, GFX_TEXTURE_STATE_SHADER_WRITE);

                GfxCmdSetTexture(cmd, GFX_HASH("DensityLUT"), m_DensityLUT, GFX_TEXTURE_STATE_SHADER_READ);
                GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), m_LinearClamp);

                struct Constants
                {
                    glm::vec3   m_BetaR;
                    float       m_PlanetRadius;
                    glm::vec3   m_BetaM;
                    float       m_AtmosphereRadius;
                    float       m_SunIntensity;
                };
                Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
                constants->m_BetaR = m_BetaR;
                constants->m_PlanetRadius = m_PlanetRadius;
                constants->m_BetaM = m_BetaM;
                constants->m_AtmosphereRadius = m_AtmosphereRadius;
                constants->m_SunIntensity = m_SunIntensity;

                const uint32_t group_count_x = (m_DirectionalLightLUTSize + 32 - 1) / 32;
                GfxCmdDispatch(cmd, group_count_x, 1, 1);

                GfxCmdTransitionTexture(cmd, m_DirectionalLightLUT, GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_SHADER_READ);

                GfxCmdEndTechnique(cmd);
            }

            // Precompute sky LUT
            {
                GfxCmdBeginTechnique(cmd, m_TechPrecomputeSkyLUT);

                GfxCmdTransitionTexture(cmd, m_SkyLUTR, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_SHADER_WRITE);
                GfxCmdTransitionTexture(cmd, m_SkyLUTM, GFX_TEXTURE_STATE_SHADER_READ, GFX_TEXTURE_STATE_SHADER_WRITE);
                GfxCmdSetTexture(cmd, GFX_HASH("SkyLUTR"), m_SkyLUTR, GFX_TEXTURE_STATE_SHADER_WRITE);
                GfxCmdSetTexture(cmd, GFX_HASH("SkyLUTM"), m_SkyLUTM, GFX_TEXTURE_STATE_SHADER_WRITE);

                GfxCmdSetTexture(cmd, GFX_HASH("DensityLUT"), m_DensityLUT, GFX_TEXTURE_STATE_SHADER_READ);
                GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), m_LinearClamp);

                struct Constants
                {
                    glm::vec3   m_BetaR;
                    float       m_PlanetRadius;
                    glm::vec3   m_BetaM;
                    float       m_AtmosphereRadius;
                    glm::vec2   m_DensityScaleHeightRM;
                };
                Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
                constants->m_BetaR = m_BetaR;
                constants->m_PlanetRadius = m_PlanetRadius;
                constants->m_BetaM = m_BetaM;
                constants->m_AtmosphereRadius = m_AtmosphereRadius;
                constants->m_DensityScaleHeightRM = m_DensityScaleHeightRM;

                const uint32_t group_count_x = (m_SkyLUTSizeX + 8 - 1) / 8;
                const uint32_t group_count_y = (m_SkyLUTSizeY + 8 - 1) / 8;
                const uint32_t group_count_z = (m_SkyLUTSizeZ + 4 - 1) / 4;
                GfxCmdDispatch(cmd, group_count_x, group_count_y, group_count_z);

                GfxCmdTransitionTexture(cmd, m_SkyLUTR, GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_SHADER_READ);
                GfxCmdTransitionTexture(cmd, m_SkyLUTM, GFX_TEXTURE_STATE_SHADER_WRITE, GFX_TEXTURE_STATE_SHADER_READ);

                GfxCmdEndTechnique(cmd);
            }
        }
    }

    void DrawSky(const Context& ctx, GfxCommandBuffer cmd)
    {
        // Draw sky
        {
            GfxCmdBeginTechnique(cmd, m_TechSky);

            GfxCmdSetRenderSetup(cmd, m_SkyRenderSetup);

            GfxCmdSetTexture(cmd, GFX_HASH("SkyLUTR"), m_SkyLUTR, GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdSetTexture(cmd, GFX_HASH("SkyLUTM"), m_SkyLUTM, GFX_TEXTURE_STATE_SHADER_READ);
            GfxCmdSetSampler(cmd, GFX_HASH("LinearClamp"), m_LinearClamp);

            glm::mat4 view = ctx.m_Camera.m_View;
            view[3][0] = view[3][1] = view[3][2] = 0.f; // Set translation to zero

            struct Constants
            {
                glm::mat4   m_InvViewProjZeroTranslation;
                glm::vec3   m_SunDirection;
                float       m_SunIntensity;
                glm::vec3   m_BetaR;
                float       m_PlanetRadius;
                glm::vec3   m_BetaM;
                float       m_AtmosphereRadius;
                float       m_HeightAboveGround;
                float       m_MieG;
            };
            Constants* constants = static_cast<Constants*>(GfxCmdAllocUploadBuffer(cmd, GFX_HASH("Constants"), sizeof(Constants)));
            constants->m_InvViewProjZeroTranslation = glm::inverse(ctx.m_Camera.m_Projection * view);
            constants->m_SunDirection = glm::normalize(ctx.m_AtmosphereLightDirection);
            constants->m_SunIntensity = m_SunIntensity;
            constants->m_BetaR = m_BetaR;
            constants->m_PlanetRadius = m_PlanetRadius;
            constants->m_BetaM = m_BetaM;
            constants->m_AtmosphereRadius = m_AtmosphereRadius;
            constants->m_HeightAboveGround = m_HeightAboveGround;
            constants->m_MieG = m_MieG;

            GfxCmdDraw(cmd, 3, 1, 0);

            GfxCmdEndTechnique(cmd);
        }
    }
};

#endif