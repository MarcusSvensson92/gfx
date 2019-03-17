#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "Camera.h"
#include "Context.h"

class CameraController
{
public:
    double      m_LastCursorX   = 0.0;
    double      m_LastCursorY   = 0.0;

    float       m_LastMove      = 0.f;
    float       m_LastStrafe    = 0.f;

    void ApplyMomentum(float& prev, float& curr, float dt)
    {
        curr = curr + (prev - curr) * powf(fabsf(curr) > fabsf(prev) ? 0.6f : 0.8f, dt * 60.f);
        prev = curr;
    }

    void Update(Context& ctx, float dt)
    {
        double cursor_x, cursor_y;
        glfwGetCursorPos(ctx.m_Window, &cursor_x, &cursor_y);
        float cursor_dx = static_cast<float>(cursor_x - m_LastCursorX);
        float cursor_dy = static_cast<float>(cursor_y - m_LastCursorY);
        m_LastCursorX = cursor_x;
        m_LastCursorY = cursor_y;

        const float cursor_speed = 0.005f;
        float cursor_scale = glfwGetMouseButton(ctx.m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? cursor_speed : 0.f;
        float pitch = cursor_scale * cursor_dy;
        float yaw = cursor_scale * cursor_dx;

        const float move_speed = 10.f;
        const float strafe_speed = 10.f;
        float move = move_speed *
            ((glfwGetKey(ctx.m_Window, GLFW_KEY_W) == GLFW_PRESS ?  dt : 0.f) +
             (glfwGetKey(ctx.m_Window, GLFW_KEY_S) == GLFW_PRESS ? -dt : 0.f));
        float strafe = strafe_speed *
            ((glfwGetKey(ctx.m_Window, GLFW_KEY_D) == GLFW_PRESS ? dt : 0.f) +
             (glfwGetKey(ctx.m_Window, GLFW_KEY_A) == GLFW_PRESS ? -dt : 0.f));

        ApplyMomentum(m_LastMove, move, dt);
        ApplyMomentum(m_LastStrafe, strafe, dt);

        ctx.m_Camera.Pitch(pitch);
        ctx.m_Camera.Yaw(yaw);
        ctx.m_Camera.Move(move);
        ctx.m_Camera.Strafe(strafe);
        ctx.m_Camera.UpdateView();
    }
};

#endif