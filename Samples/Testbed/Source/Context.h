#ifndef CONTEXT_H
#define CONTEXT_H

#include <GLFW/glfw3.h>

#include <Gfx.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

struct Context
{
    GLFWwindow*             m_Window;
    uint32_t                m_Width;
    uint32_t                m_Height;
    bool                    m_Minimized;

    GfxDevice               m_Device;
    
    GfxTexture              m_ColorBuffer;
    GfxTexture              m_DepthBuffer;

    Camera                  m_Camera;

    GfxTexture              m_AtmosphereAmbientLightLUT;
    GfxTexture              m_AtmosphereDirectionalLightLUT;
    glm::vec3               m_AtmosphereLightDirection;
    float                   m_AtmosphereLightCoord;
};

#endif