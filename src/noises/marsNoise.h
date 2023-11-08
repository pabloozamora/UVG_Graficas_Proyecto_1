#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include <SDL.h>
#include "../globals.h"

FastNoiseLite marsNoise;

Color getMarsNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 darkColor = {122.0f/255, 70.0f/255, 55.0f/255};
    glm::vec3 lightColor = {239.0f/255, 104.0f/255, 57.0f/255};

    marsNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    marsNoise.SetFrequency(0.2);
    marsNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 40.0f;

    float noise;

    noise = marsNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 marsNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(marsNoiseColor.x, marsNoiseColor.y, marsNoiseColor.z);

    return fragmentColor;
}