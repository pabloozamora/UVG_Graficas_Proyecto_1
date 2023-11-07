#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include <SDL.h>
#include "../globals.h"

FastNoiseLite moonNoise;

Color getMoonNoise(float x, float y, float z) {

    Color fragmentColor;

    glm::vec3 darkColor = {95.0f/255, 106.0f/255, 116.0f/255};
    glm::vec3 lightColor = {209.0f/255, 195.0f/255, 183.0f/255};

    moonNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    moonNoise.SetFrequency(0.2);
    moonNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 60.0f;

    float noise;

    noise = moonNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 moonNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(moonNoiseColor.x, moonNoiseColor.y, moonNoiseColor.z);

    return fragmentColor;
}

//Posible Marte?

/* Color getJupiterNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 darkColor = {101.0f/255, 65.0f/255, 21.0f/255};
    glm::vec3 lightColor = {209.0f/255, 195.0f/255, 183.0f/255};

    jupiterNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    jupiterNoise.SetFrequency(0.2);
    jupiterNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 100.0f;

    float noise;

    noise = jupiterNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 jupiterNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(jupiterNoiseColor.x, jupiterNoiseColor.y, jupiterNoiseColor.z);

    return fragmentColor;
} */