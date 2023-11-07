#include <glm/glm.hpp>
#include <string>
#include <vector>
#pragma once

struct Planet {
    float rotationAngle;
    float translationAngle;
    float translationRadius;
    float translationSpeed;
    glm::vec3 translationAxis;
    glm::vec3 worldPos;
    glm::vec3 scaleFactor;
    std::string name;
    std::vector<Planet> satelites;
};