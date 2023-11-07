#include <glm/glm.hpp>
#include "uniforms.h"
#include "fragment.h"
#include <SDL.h>
#include <string>
#include "FastNoiseLite.h"
#include "./noises/earthNoise.h"
#include "./noises/sunNoise.h"
#include "./noises/slimePlanetNoise.h"
#include "./noises/diamondPlanetNoise.h"
#include "./noises/gasPlanetNoise.h"
#include "./noises/moonNoise.h"

Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    // Aplicar las transformaciones al vértice utilizando las matrices de uniforms
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

    // Perspectiva
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    // Aplicar transformación del viewport
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);
    
    // Transformar la normal
    glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
    transformedNormal = glm::normalize(transformedNormal);

    return Vertex{
        glm::vec3(screenVertex),
        transformedNormal,
        vertex.position
    };
}

Fragment fragmentShader(Fragment& fragment, const std::string name) {

    float x = fragment.originalPos.x;
    float y = fragment.originalPos.y;
    float z = fragment.originalPos.z;

    if (name == "earth") {
        
        fragment.color = getEarthNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "moon") {

        fragment.color = getMoonNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "sun") {
        fragment.color = getSunNoise(x,y,z);
    }

    else if (name == "diamond") {
        fragment.color = getDiamondPlanetNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "slime") {
        fragment.color = getSlimePlanetNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "gas") {
        fragment.color = getGasPlanetNoise(x,y,z) * fragment.intensity;
    }

    return fragment;
}
