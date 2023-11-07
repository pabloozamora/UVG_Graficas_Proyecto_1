#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include "../globals.h"

FastNoiseLite slimeNoise;

Color getSlimePlanetNoise(float x, float y, float z) {
    Color fragmentColor;

    Color trippyColor = {73, 218, 37};

    float noise;

    slimeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    float animationSpeed = 0.001f; // Adjust the speed of the animation
    float time = frame * animationSpeed;
    slimeNoise.SetFrequency(0.02 + sin(3 * time) * 0.01);
    int zoom = 200;
    noise = (1 + slimeNoise.GetNoise(x*zoom, y*zoom, z*zoom));

    fragmentColor = trippyColor * noise *15.0f;

    return fragmentColor;

}