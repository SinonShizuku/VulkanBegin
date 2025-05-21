#pragma once
#include "myVulkan/EasyVKStart.h"

struct vertex2d {
    glm::vec2 position;
    glm::vec2 texCoord;
};

struct vertex3d {
    glm::vec3 position;
    glm::vec4 color;
};

inline glm::mat4 FlipVertical(const glm::mat4& projection) {
    glm::mat4 _projection = projection;
    for (uint32_t i = 0; i < 4; i++)
        _projection[i][1] *= -1;
    return _projection;
}