#include "../FastNoiseLite.h"
#include "../color.h"
#include "../fragment.h"
#include <SDL.h>
#include "../globals.h"

FastNoiseLite earthNoise;
FastNoiseLite terrainNoise;
FastNoiseLite skyNoise;

Color getEarthNoise(float x, float y, float z){
    Color fragmentColor;

    glm::vec3 groundColor = {67.0f/255, 122.0f/255, 41.0f/255};
    glm::vec3 terrainColor = {91.0f/255, 186.0f/255, 47.0f/255};
    glm::vec3 beachColor = {228.0f/255, 236.0f/255, 135.0f/255};
    glm::vec3 oceanColor = {42.0f/255, 50.0f/255, 115.0f/255};
    glm::vec3 seaColor = {147.0f/255, 157.0f/255, 234.0f/255};
    glm::vec3 cloudColor = {228.0f/255, 229.0f/255, 240.0f/255};

    earthNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    earthNoise.SetFrequency(0.002);
    earthNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    earthNoise.SetFractalOctaves(6);

    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    terrainNoise.SetFractalOctaves(7);
    terrainNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    terrainNoise.SetFrequency(0.005);

    skyNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    skyNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    float animationSpeed = 0.01f;
    skyNoise.SetFractalOctaves(6);

    
    float ox = 70.0f;
    float oy = 70.0f;
    float zoom = 500.0f;

    //Primera capa: Oceano y Tierra
    float firstLayer = earthNoise.GetNoise((x + ox) * zoom,(y + oy) * zoom, z * zoom);
    glm::vec3 tempColor;

    //Segunda capa: Textura de continentes
    if (firstLayer < 0.2) {
        zoom = 800.0f;
        float continentNoise = terrainNoise.GetNoise(x * zoom, y * zoom, z * zoom);
        tempColor = groundColor + (terrainColor * (continentNoise));
        fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
        //Tercera capa: Textura de playas
        if (firstLayer > 0.18) {
            tempColor = beachColor + terrainColor * continentNoise;
            fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
        }
    //Cuarta capa: Profundidad de oceanos
    } else {
        float oceanNoise = terrainNoise.GetNoise(x * zoom, y * zoom, z * zoom);
        tempColor = oceanColor + seaColor * oceanNoise;
        fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
    }

    //Quinta capa: Nubes animadas
    zoom = 100.0f;
    float cloudNoise = skyNoise.GetNoise((x + frame * 0.007f) * zoom, y * zoom, z * zoom);
    if (cloudNoise > 0.2) {
        tempColor = tempColor + cloudColor * cloudNoise;
        fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
    }
    
    return fragmentColor;
};
