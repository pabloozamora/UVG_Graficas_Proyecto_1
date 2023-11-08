#include <glm/glm.hpp>
#pragma once

struct Spaceship {
    glm::vec3 worldPos;
    glm::vec3 scaleFactor;
    float rotationAngle;
    float movementSpeed;
    float rotationSpeed;
};