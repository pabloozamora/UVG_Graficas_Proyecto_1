#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include "../globals.h"

FastNoiseLite sunNoise;

Color getSunNoise(float x, float y, float z) {

    Color fragmentColor;

    glm::vec3 darkColor = glm::vec3(0.73f, 0.06f, 0.03f);
    glm::vec3 brightColor = glm::vec3(0.99f, 0.45f, 0.04f);

    float noise;

    sunNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    float animationSpeed = 0.01f; // Adjust the speed of the animation
    float time = frame * animationSpeed;
    sunNoise.SetFrequency(0.02 + (sin(3 * time) + 1) * 0.01);
    int zoom = 600;
    noise = (1 + sunNoise.GetNoise(x*zoom, y*zoom, z*zoom))/2.5f;

    glm::vec3 sunNoiseColor = mix(brightColor, darkColor, noise*2.0f);

    fragmentColor = Color(sunNoiseColor.x, sunNoiseColor.y, sunNoiseColor.z);

    return fragmentColor;

};