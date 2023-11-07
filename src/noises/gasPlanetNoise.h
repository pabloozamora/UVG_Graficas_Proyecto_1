#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include <SDL.h>
#include "../globals.h"

FastNoiseLite gasPlanetNoise;

Color getGasPlanetNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 darkColor = {101.0f/255, 65.0f/255, 21.0f/255};
    glm::vec3 lightColor = {209.0f/255, 195.0f/255, 183.0f/255};

    gasPlanetNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    gasPlanetNoise.SetFrequency(0.2);
    gasPlanetNoise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
    gasPlanetNoise.SetFractalOctaves(8);
    gasPlanetNoise.SetFractalWeightedStrength(0.2);
    gasPlanetNoise.SetDomainWarpAmp(7.0);
    gasPlanetNoise.DomainWarp(x, y, z);

    float zoom = 20.0f;

    float noise;

    noise = gasPlanetNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 gasPlanetNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(gasPlanetNoiseColor.x, gasPlanetNoiseColor.y, gasPlanetNoiseColor.z);

    return fragmentColor;
}