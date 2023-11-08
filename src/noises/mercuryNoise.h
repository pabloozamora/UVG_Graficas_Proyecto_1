#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include <SDL.h>
#include "../globals.h"

FastNoiseLite mercuryNoise;

Color getMercuryNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 darkColor = {91.0f/255, 91.0f/255, 91.0f/255};
    glm::vec3 lightColor = {222.0f/255, 222.0f/255, 222.0f/255};

    mercuryNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    mercuryNoise.SetFrequency(0.2);
    mercuryNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 40.0f;

    float noise;

    noise = mercuryNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 mercuryNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(mercuryNoiseColor.x, mercuryNoiseColor.y, mercuryNoiseColor.z);

    return fragmentColor;
}