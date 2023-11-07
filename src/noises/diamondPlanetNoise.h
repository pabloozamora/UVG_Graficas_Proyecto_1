#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include "../globals.h"

FastNoiseLite diamondNoise;

Color getDiamondPlanetNoise(float x, float y, float z){
    Color fragmentColor;

    glm::vec3 diamondColor = {78.0f/255, 242.0f/255, 230.0f/255};
    glm::vec3 shineColor = {237.0f/255, 250.0f/255, 249.0f/255};

    diamondNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    diamondNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    diamondNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    float animationSpeed = 0.1f; // Adjust the speed of the animation
    float time = frame * animationSpeed;
    diamondNoise.SetFrequency(0.05);

    int zoom = 400;

    // Calculate the shine factor using a sine function
    float shineFactor = 0.2f * std::sin(time);

    float noise = diamondNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    if (noise < 0) {
        noise = noise * shineFactor;
    }

    glm::vec3 tmpColor = diamondColor + shineColor * noise;

    fragmentColor = {tmpColor.x, tmpColor.y, tmpColor.z};

    return fragmentColor;
}