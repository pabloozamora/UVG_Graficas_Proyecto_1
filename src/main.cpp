#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include "color.h"
#include "objReader.h"
#include "framebuffer.h"
#include "uniforms.h"
#include "shaders.h"
#include "triangle.h"
#include "camera.h"
#include "Planet.h"
#include "SpaceShip.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Color currentColor;

const std::string modelPathSphere = "../models/sphere.obj";
const std::string modelPathSpaceShip = "../models/spaceship.obj";
Color clearColor(0, 0, 0);  // Color del fondo

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<Face> faces;
std::vector<Vertex> verticesArray;

std::vector<glm::vec3> spaceShipVertices;
std::vector<glm::vec3> spaceShipNormals;
std::vector<Face> spaceShipFaces;
std::vector<Vertex> spaceShipVerticesArray;


Uniforms uniforms;

glm::mat4 model = glm::mat4(1);
glm::mat4 view = glm::mat4(1);
glm::mat4 projection = glm::mat4(1);

enum class Primitive {
    TRIANGLES,
};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error: No se puedo inicializar SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Proyecto 1: Space Travel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error: No se pudo crear una ventana SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error: No se pudo crear SDL_Renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

std::vector<std::vector<Vertex>> primitiveAssembly(
    Primitive polygon,
    const std::vector<Vertex>& transformedVertices
) {
    std::vector<std::vector<Vertex>> assembledVertices;

    switch (polygon) {
        case Primitive::TRIANGLES: {
            assert(transformedVertices.size() % 3 == 0 && "El número de vértices debe ser un múltiplo de 3 para triángulos.");

            for (size_t i = 0; i < transformedVertices.size(); i += 3) {
                std::vector<Vertex> triangle = {
                    transformedVertices[i],
                    transformedVertices[i + 1],
                    transformedVertices[i + 2]
                };
                assembledVertices.push_back(triangle);
            }
            break;
        }
        default:
            std::cerr << "Error: No se reconoce el tipo primitivo." << std::endl;
            break;
    }

    return assembledVertices;
}

std::vector<Fragment> rasterize(Primitive primitive, const std::vector<std::vector<Vertex>>& assembledVertices, std::string name, glm::vec3 worldPos) {
    std::vector<Fragment> fragments;

    switch (primitive) {
        case Primitive::TRIANGLES: {
            for (const std::vector<Vertex>& triangleVertices : assembledVertices) {
                assert(triangleVertices.size() == 3 && "Triangle vertices must contain exactly 3 vertices.");
                std::vector<Fragment> triangleFragments = triangle(triangleVertices[0], triangleVertices[1], triangleVertices[2], name, worldPos);
                fragments.insert(fragments.end(), triangleFragments.begin(), triangleFragments.end());
            }
            break;
        }
        default:
            std::cerr << "Error: No se reconoce el tipo primitivo para rasterización." << std::endl;
            break;
    }

    return fragments;
}

glm::mat4 createProjectionMatrix() {
  float fovInDegrees = 45.0f;
  float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
  float nearClip = 0.1f;
  float farClip = 100.0f;

  return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix(size_t screenWidth, size_t screenHeight) {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

void render(Primitive polygon, std::string name, std::vector<Vertex>& modelVertices, glm::vec3 worldPos){

    // 1. Vertex Shader
    std::vector<Vertex> transformedVertices;

    for (const Vertex& vertex : modelVertices) {
        transformedVertices.push_back(vertexShader(vertex, uniforms));
    }

    // 2. Primitive Assembly
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(polygon, transformedVertices);

    // 3. Rasterization
    std::vector<Fragment> fragments = rasterize(polygon, assembledVertices, name, worldPos);

    // 4. Fragment Shader
    for (Fragment& fragment : fragments) {
    // Apply the fragment shader to compute the final color
    fragment = fragmentShader(fragment, name);
    point(fragment);
    }

}

void setUpRender(Planet& model) {
    float rotationAngle = model.rotationAngle;
    float translationAngle = model.translationAngle;

    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    
    if (model.name != "sun") {
        model.rotationAngle += 1;
        model.translationAngle += model.translationSpeed;
    }
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), model.scaleFactor);

    model.worldPos.x = model.translationAxis.x + (model.translationRadius * cos(glm::radians(translationAngle)));
    model.worldPos.z = model.translationAxis.z + (model.translationRadius * sin(glm::radians(translationAngle)));

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), model.worldPos);

    // Calcular la matriz de modelo
    uniforms.model = translation * rotation * scale;
}

void setUpOrbit(Planet& planet) {
    // Agregar puntos para 360 grados
    std::vector<Vertex> orbitsVertices;
    for (float i = 0.0f; i < 360.0f; i += 1.0f)
    {

      Vertex vertex = {glm::vec3(planet.translationAxis.x + planet.translationRadius * cos(glm::radians(i)),
                        0.0f,
                        planet.translationAxis.z + planet.translationRadius * sin(glm::radians(i))),
                        glm::vec3(1.0f)};
      Vertex transformedVertex = vertexShader(vertex, uniforms);
      orbitsVertices.push_back(transformedVertex);
    }

    for (Vertex vert : orbitsVertices)
    {

      if (vert.position.x < 0 || vert.position.y < 0  ||  vert.position.y > SCREEN_HEIGHT || vert.position.x > SCREEN_WIDTH) 
        continue;

      Fragment fragment = {
          vert.position,
          Color(255, 255, 255),
          vert.position.z,
          1.0f,
          vert.position};
      point(fragment);
    }
}

void fastTravel(SDL_Renderer* renderer, const Camera& camera) {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    Uint32 startTime = SDL_GetTicks(); 
    const Uint32 animationDuration = 750; 

    while (true) {
        Uint32 currentTime = SDL_GetTicks();  

        Uint32 elapsedTime = currentTime - startTime;

        if (elapsedTime >= animationDuration) {
            break; 
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < SCREEN_HEIGHT; y += 2) {
            for (int x = 0; x < SCREEN_WIDTH; x += 2) {
                float noiseValue = noise.GetNoise((x + camera.cameraPosition.x) * 30.0f, (y + camera.cameraPosition.y) * 30.0f);

                if (noiseValue > 0.75f) {
                    SDL_SetRenderDrawColor(renderer, 225, 225, 225, 225);

                    if (x < SCREEN_WIDTH / 2 && y < SCREEN_HEIGHT / 2)
                    SDL_RenderDrawLine(renderer, x - elapsedTime / 100, y - elapsedTime / 100, x, y );

                    else if (x > SCREEN_WIDTH / 2 && y < SCREEN_HEIGHT / 2)
                    SDL_RenderDrawLine(renderer, x + elapsedTime / 100, y - elapsedTime / 100, x, y);

                    else if (x < SCREEN_WIDTH / 2 && y > SCREEN_HEIGHT / 2)
                    SDL_RenderDrawLine(renderer, x - elapsedTime / 100, y + elapsedTime / 100, x, y);

                    else if (x > SCREEN_WIDTH / 2 && y > SCREEN_HEIGHT / 2)
                    SDL_RenderDrawLine(renderer, x + elapsedTime / 100, y + elapsedTime / 100, x, y);
                }
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(10);  
    }
}

int main(int argv, char** args)
{
    if (!init()) {
        return 1;
    }

    clear(10, 10);

    // Inicializar cámara
    float cameraSpeed = 0.1f;

    Camera camera;
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 15.0f);
    camera.targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    // Matriz de proyección
    uniforms.projection = createProjectionMatrix();

    // Matriz de viewport
    uniforms.viewport = createViewportMatrix(SCREEN_WIDTH, SCREEN_HEIGHT);

    /*PREPARAR UNIFORMS*/

    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); // Rotar alrededor del eje Y

    // Preparar uniforms del Sol
    Planet sun;
    sun.name = "sun";
    sun.worldPos = {0.0f, 0.0f, 0.0f};
    sun.rotationAngle = 0.0f;
    sun.scaleFactor = {1.5f, 1.5f, 1.5f};
    sun.translationRadius = 0.0f;
    sun.translationAxis = {0.0f, 0.0f, 0.0f};

    // Preparar uniforms de Mercurio
    Planet mercury;
    mercury.name = "mercury";
    mercury.translationRadius = 2.0f;
    mercury.rotationAngle = 1.0f;
    mercury.scaleFactor = {1.0f, 1.0f, 1.0f};
    mercury.translationAngle = 170.0f;
    mercury.translationSpeed = 1.0f;
    mercury.worldPos = {mercury.translationRadius * cos(glm::radians(mercury.translationAngle)), 0.0f, mercury.translationRadius * sin(glm::radians(mercury.translationAngle))};
    mercury.translationAxis = sun.worldPos;


    //Preparar uniforms de la Tierra
    Planet earth;
    earth.name = "earth";
    earth.translationRadius = 4.0f;
    earth.rotationAngle = 1.0f;
    earth.scaleFactor = {1.0f, 1.0f, 1.0f};
    earth.translationAngle = 100.0f;
    earth.worldPos = {earth.translationRadius * cos(glm::radians(earth.translationAngle)), 0.0f, earth.translationRadius * sin(glm::radians(earth.translationAngle))};
    earth.translationSpeed = 0.5f;
    earth.translationAxis = sun.worldPos;

    //Preparar uniforms de Marte
    Planet mars;
    mars.name = "mars";
    mars.translationRadius = 7.0f;
    mars.rotationAngle = 1.0f;
    mars.scaleFactor = {0.8f, 0.8f, 0.8f};
    mars.translationAngle = 0.0f;
    mars.translationSpeed = 0.3f;
    mars.worldPos = {mars.translationRadius * cos(glm::radians(mars.translationAngle)), 0.0f, mars.translationRadius * sin(glm::radians(mars.translationAngle))};
    mars.translationAxis = sun.worldPos;

    // Preparar uniforms de Jupiter
    Planet jupiter;
    jupiter.name = "gas";
    jupiter.translationRadius = 12.0f;
    jupiter.rotationAngle = 1.0f;
    jupiter.scaleFactor = {1.3f, 1.3f, 1.3f};
    jupiter.translationAngle = 0.0f;
    jupiter.translationSpeed = 0.5f;
    jupiter.worldPos = {jupiter.translationRadius * cos(glm::radians(jupiter.translationAngle)), 0.0f, jupiter.translationRadius * sin(glm::radians(jupiter.translationAngle))};
    jupiter.translationAxis = sun.worldPos;

    // Preparar uniforms de planeta Trippy
    Planet trippy;
    trippy.name = "slime";
    trippy.translationRadius = 15.0f;
    trippy.rotationAngle = 0.0f;
    trippy.scaleFactor = {1.1f, 1.1f, 1.1f};
    trippy.translationAngle = 300.0f;
    trippy.translationSpeed = 0.3f;
    trippy.worldPos = {trippy.translationRadius * cos(glm::radians(trippy.translationAngle)), 0.0f, trippy.translationRadius * sin(glm::radians(trippy.translationAngle))};
    trippy.translationAxis = sun.worldPos;

    // Preparar uniforms de la Luna
    Planet moon;
    moon.name = "moon";
    moon.translationRadius = 1.0f;
    moon.rotationAngle = 0.0f;
    moon.scaleFactor = {0.2f, 0.2f, 0.2f};
    moon.translationAngle = 1.0f;
    moon.translationSpeed = 2.0f;
    moon.translationAxis = earth.worldPos;
    moon.worldPos = {moon.translationAxis.x + (moon.translationRadius * cos(glm::radians(moon.translationAngle))),
                    0.3f,
                    moon.translationAxis.z + (moon.translationRadius * sin(glm::radians(moon.translationAngle)))};

    // Preparar uniforms de Phobos
    Planet phobos;
    phobos.name = "moon";
    phobos.translationRadius = 1.0f;
    phobos.rotationAngle = 0.0f;
    phobos.scaleFactor = {0.2f, 0.2f, 0.2f};
    phobos.translationAngle = 50.0f;
    phobos.translationSpeed = 2.0f;
    phobos.translationAxis = mars.worldPos;
    phobos.worldPos = {phobos.translationAxis.x + (phobos.translationRadius * cos(glm::radians(phobos.translationAngle))),
                    0.3f,
                    phobos.translationAxis.z + (phobos.translationRadius * sin(glm::radians(phobos.translationAngle)))};

    // Preparar uniforms de Deimos
    Planet deimos;
    deimos.name = "moon";
    deimos.translationRadius = 0.7f;
    deimos.rotationAngle = 0.0f;
    deimos.scaleFactor = {0.2f, 0.2f, 0.2f};
    deimos.translationAngle = 150.0f;
    deimos.translationSpeed = 2.0f;
    deimos.translationAxis = mars.worldPos;
    deimos.worldPos = {deimos.translationAxis.x + (deimos.translationRadius * cos(glm::radians(deimos.translationAngle))),
                    -0.3f,
                    deimos.translationAxis.z + (deimos.translationRadius * sin(glm::radians(deimos.translationAngle)))};

    // Preparar uniforms de Europa
    Planet europa;
    europa.name = "moon";
    europa.translationRadius = 1.3f;
    europa.rotationAngle = 0.0f;
    europa.scaleFactor = {0.3f, 0.3f, 0.3f};
    europa.translationAngle = 150.0f;
    europa.translationSpeed = 3.0f;
    europa.translationAxis = jupiter.worldPos;
    europa.worldPos = {europa.translationAxis.x + (europa.translationRadius * cos(glm::radians(europa.translationAngle))),
                    0.1f,
                    europa.translationAxis.z + (europa.translationRadius * sin(glm::radians(europa.translationAngle)))};

    //Agregar satelites alrededor de sus correspondientes planetas
    earth.satelites.push_back(moon);
    mars.satelites.push_back(phobos);
    mars.satelites.push_back(deimos);
    jupiter.satelites.push_back(europa);

    // Agregar planetas alrededor del Sol
    sun.satelites.push_back(mercury);
    sun.satelites.push_back(earth);
    sun.satelites.push_back(mars);
    sun.satelites.push_back(jupiter);
    sun.satelites.push_back(trippy);

    // Preparar uniforms de la nave
    Spaceship spaceship;
    spaceship.worldPos = {0.0f, 0.0f, 20.0f};
    spaceship.scaleFactor = {0.03f, 0.03f, 0.03f};
    spaceship.rotationAngle = 90.0f;
    spaceship.rotationSpeed = 2.0f;
    spaceship.movementSpeed = 1.0f;

    // Vertices de modelo spaceship
    loadOBJ(modelPathSpaceShip, spaceShipVertices, spaceShipNormals, spaceShipFaces);
    spaceShipVerticesArray = setupVertexArray(spaceShipVertices, spaceShipNormals, spaceShipFaces);

    // Vertices de modelo esfera
    loadOBJ(modelPathSphere, vertices, normals, faces);
    verticesArray = setupVertexArray(vertices, normals, faces);

    //Matriz de vista

    bool isRunning = true;
    while (isRunning) {
        frame += 1;
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                        if (spaceship.rotationAngle - spaceship.rotationSpeed == 0.0f) {
                            spaceship.rotationAngle = 360.0f;
                        }
                        else {
                            spaceship.rotationAngle -= spaceship.rotationSpeed;
                        }
                        break;

                    case SDLK_LEFT:
                        if (spaceship.rotationAngle + spaceship.rotationSpeed > 360.0f) {
                            spaceship.rotationAngle = spaceship.rotationSpeed;
                        }
                        else {
                            spaceship.rotationAngle += spaceship.rotationSpeed;
                        }
                        break;

                    case SDLK_UP:
                        spaceship.worldPos.z -= spaceship.movementSpeed * sin(glm::radians(spaceship.rotationAngle));
                        spaceship.worldPos.x += spaceship.movementSpeed * cos(glm::radians(spaceship.rotationAngle));
                        break;

                    case SDLK_DOWN:
                        spaceship.worldPos.z += spaceship.movementSpeed * sin(glm::radians(spaceship.rotationAngle));
                        spaceship.worldPos.x -= spaceship.movementSpeed * cos(glm::radians(spaceship.rotationAngle));
                        break;

                    case SDLK_r:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {0.0f, 0.0f, 20.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_1:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sun.satelites[0].worldPos.x, 0.0f, sun.satelites[0].worldPos.z + 1.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_2:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sun.satelites[1].worldPos.x, 0.0f, sun.satelites[1].worldPos.z + 1.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_3:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sun.satelites[2].worldPos.x, 0.0f, sun.satelites[2].worldPos.z + 1.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_4:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sun.satelites[3].worldPos.x, 0.0f, sun.satelites[3].worldPos.z + 1.5f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_5:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sun.satelites[4].worldPos.x, 0.0f, sun.satelites[4].worldPos.z + 1.5f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    default:
                        break;
                }
            }
        }

        clear(10, 10);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Actualizar cámara

        // Crear la matriz de vista usando el objeto cámara
            uniforms.view = glm::lookAt(
                camera.cameraPosition, // The position of the camera
                camera.targetPosition, // The point the camera is looking at
                camera.upVector        // The up vector defining the camera's orientation
            );

        // Renderizar el Sol
        setUpRender(sun);
        render(Primitive::TRIANGLES, sun.name, verticesArray, sun.worldPos);

        // Renderizar orbitas
        /* for (Planet& planet : sun.satelites) {
            setUpOrbit(planet);
        } */

        // Renderizar nave
        glm::mat4 spaceshipTranslation = glm::translate(glm::mat4(1.0f), spaceship.worldPos);
        glm::mat4 spaceshipScale = glm::scale(glm::mat4(1.0f), spaceship.scaleFactor);
        glm::mat4 spaceshipRotation = glm::rotate(glm::mat4(1.0f), glm::radians(spaceship.rotationAngle), rotationAxis);
        uniforms.model = spaceshipTranslation * spaceshipRotation *  spaceshipScale;

        float d = 5.0f; //Distancia de camara a nave
        
        // Determinar posición de la cámara
        float cameraAngle;
        float dx;
        float dz;

        if (spaceship.rotationAngle > 0 && spaceship.rotationAngle <= 90) {
            // I cuadrante: de 1 a 90
            cameraAngle = spaceship.rotationAngle;
            dz = d * sin(glm::radians(cameraAngle));
            dx = d * cos(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x - dx, 1.0f, spaceship.worldPos.z + dz};
        }

        else if (spaceship.rotationAngle > 90 && spaceship.rotationAngle <= 180) {
            // II cuadrante: de 91 a 180
            cameraAngle = spaceship.rotationAngle - 90.0f;
            dz = d * cos(glm::radians(cameraAngle));
            dx = d * sin(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x + dx, 1.0f, spaceship.worldPos.z + dz};
        }

        else if (spaceship.rotationAngle > 180 && spaceship.rotationAngle <= 270) {
            // III cuadrante: de 181 a 270
            cameraAngle = spaceship.rotationAngle - 180.0f;
            dz = d * sin(glm::radians(cameraAngle));
            dx = d * cos(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x + dx, 1.0f, spaceship.worldPos.z - dz};
        }

        else if (spaceship.rotationAngle > 270 && spaceship.rotationAngle <= 360) {
            // IV cuadrante: de 271 a 360
            cameraAngle = spaceship.rotationAngle - 270.0f;
            dz = d * cos(glm::radians(cameraAngle));
            dx = d * sin(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x - dx, 1.0f, spaceship.worldPos.z - dz};
        }

        

        //Actualizar lookAt de camera
        camera.targetPosition = spaceship.worldPos;

        render(Primitive::TRIANGLES, "ship", spaceShipVerticesArray, spaceship.worldPos);

       
        for (Planet& planet : sun.satelites) {
            // Renderizar satelites
            for (Planet& satellite : planet.satelites) {
                satellite.translationAxis = planet.worldPos;
                setUpRender(satellite);
                render(Primitive::TRIANGLES, satellite.name, verticesArray, satellite.worldPos);
                renderBuffer(renderer);
            }

            // Renderizar planetas
            setUpRender(planet);
            render(Primitive::TRIANGLES, planet.name, verticesArray, planet.worldPos);
            renderBuffer(renderer);

        }

        SDL_RenderPresent(renderer);

    }

    return 0;
}