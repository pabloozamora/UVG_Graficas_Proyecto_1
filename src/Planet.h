#include <glm/glm.hpp>
#include <string>
#pragma once

struct Planet {
    float rotationAngle;
    glm::vec3 translationVector;
    glm::vec3 scaleFactor;
    std::string name;
};