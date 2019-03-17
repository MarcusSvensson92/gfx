#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

class Camera
{
public:
    glm::mat4   m_View                  = glm::identity<glm::mat4>();
    glm::mat4   m_Projection            = glm::identity<glm::mat4>();
    glm::mat4   m_UnjitteredProjection  = glm::identity<glm::mat4>();

    glm::vec3   m_Right                 = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3   m_Up                    = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3   m_Look                  = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3   m_Position              = glm::vec3(0.0f, 0.0f, 0.0f);

    float       m_Width                 = 0.0f;
    float       m_Height                = 0.0f;

    float       m_NearZ                 = 0.0f;
    float       m_FarZ                  = 0.0f;

    void Perspective(float fov_y, float width, float height, float near_z, float far_z)
    {
        m_Width = width;
        m_Height = height;
        m_NearZ = near_z;
        m_FarZ = far_z;

        m_Projection = glm::perspective(fov_y, width / height, near_z, far_z);
        m_UnjitteredProjection = m_Projection;
    }
    void ApplyJitter(float jitter_x, float jitter_y)
    {
        glm::mat4 jitter = glm::translate(glm::vec3(jitter_x * (-2.0f / m_Width), jitter_y * (-2.0f / m_Height), 0.0f));
        m_Projection = jitter * m_UnjitteredProjection;
    }

    void UpdateView()
    {
        m_Look = glm::normalize(m_Look);
        m_Up = glm::normalize(glm::cross(m_Right, m_Look));
        m_Right = glm::cross(m_Look, m_Up);

        m_View[0][0] = m_Right.x;
        m_View[1][0] = m_Right.y;
        m_View[2][0] = m_Right.z;
        m_View[0][1] = m_Up.x;
        m_View[1][1] = m_Up.y;
        m_View[2][1] = m_Up.z;
        m_View[0][2] = -m_Look.x;
        m_View[1][2] = -m_Look.y;
        m_View[2][2] = -m_Look.z;
        m_View[3][0] = -glm::dot(m_Right, m_Position);
        m_View[3][1] = -glm::dot(m_Up, m_Position);
        m_View[3][2] =  glm::dot(m_Look, m_Position);
    }

    void LookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up)
    {
        m_Position = position;
        m_Look = glm::normalize(target - position);
        m_Right = glm::normalize(glm::cross(m_Look, up));
        m_Up = glm::cross(m_Right, m_Look);
    }

    void Pitch(float angle)
    {
        glm::mat3 rotation = glm::mat3(glm::rotate(angle, m_Right));
        m_Up = rotation * m_Up;
        m_Look = rotation * m_Look;
    }
    void Yaw(float angle)
    {
        glm::mat3 rotation = glm::mat3(glm::rotate(angle, glm::vec3(0.f, 1.f, 0.f)));
        m_Right = rotation * m_Right;
        m_Up = rotation * m_Up;
        m_Look = rotation * m_Look;
    }

    void Move(float distance)
    {
        m_Position += distance * m_Look;
    }
    void Strafe(float distance)
    {
        m_Position += distance * m_Right;
    }
};

#endif