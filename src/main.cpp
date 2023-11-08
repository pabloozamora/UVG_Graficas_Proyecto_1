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

std::vector<Fragment> rasterize(Primitive primitive, const std::vector<std::vector<Vertex>>& assembledVertices) {
    std::vector<Fragment> fragments;

    switch (primitive) {
        case Primitive::TRIANGLES: {
            for (const std::vector<Vertex>& triangleVertices : assembledVertices) {
                assert(triangleVertices.size() == 3 && "Triangle vertices must contain exactly 3 vertices.");
                std::vector<Fragment> triangleFragments = triangle(triangleVertices[0], triangleVertices[1], triangleVertices[2]);
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

void render(Primitive polygon, std::string name, std::vector<Vertex>& modelVertices){

    // 1. Vertex Shader
    std::vector<Vertex> transformedVertices;

    for (const Vertex& vertex : modelVertices) {
        transformedVertices.push_back(vertexShader(vertex, uniforms));
    }

    // 2. Primitive Assembly
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(polygon, transformedVertices);

    // 3. Rasterization
    std::vector<Fragment> fragments = rasterize(polygon, assembledVertices);

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

int main(int argv, char** args)
{
    if (!init()) {
        return 1;
    }

    clear(100, 100);

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
    sun.scaleFactor = {1.0f, 1.0f, 1.0f};
    sun.translationRadius = 0.0f;
    sun.translationAxis = {0.0f, 0.0f, 0.0f};

    // Preparar uniforms de Mercurio
    Planet mercury;
    mercury.name = "mercury";
    mercury.worldPos = {1.5f, 0.0f, 0.0f};
    mercury.translationRadius = 1.5f;
    mercury.rotationAngle = 1.0f;
    mercury.scaleFactor = {0.5f, 0.5f, 0.5f};
    mercury.translationAngle = 170.0f;
    mercury.translationSpeed = 2.0f;
    mercury.translationAxis = sun.worldPos;


    //Preparar uniforms de la Tierra
    Planet earth;
    earth.name = "earth";
    earth.translationRadius = 3.5f;
    earth.rotationAngle = 1.0f;
    earth.scaleFactor = {0.5f, 0.5f, 0.5f};
    earth.translationAngle = 100.0f;
    earth.worldPos = {earth.translationRadius * cos(glm::radians(earth.translationAngle)), 0.0f, earth.translationRadius * sin(glm::radians(earth.translationAngle))};
    earth.translationSpeed = 1.5f;
    earth.translationAxis = sun.worldPos;

    //Preparar uniforms de Marte
    Planet mars;
    mars.name = "mars";
    mars.worldPos = {4.5f, 0.0f, 0.0f};
    mars.translationRadius = 4.5f;
    mars.rotationAngle = 1.0f;
    mars.scaleFactor = {0.5f, 0.5f, 0.5f};
    mars.translationAngle = 0.0f;
    mars.translationSpeed = 1.0f;
    mars.translationAxis = sun.worldPos;

    // Preparar uniforms de Jupiter
    Planet jupiter;
    jupiter.name = "gas";
    jupiter.worldPos = {6.0f, 0.0f, 0.0f};
    jupiter.translationRadius = 6.0f;
    jupiter.rotationAngle = 1.0f;
    jupiter.scaleFactor = {1.0f, 1.0f, 1.0f};
    jupiter.translationAngle = 0.0f;
    jupiter.translationSpeed = 0.5f;
    jupiter.translationAxis = sun.worldPos;

    // Preparar uniforms de planeta Trippy
    Planet trippy;
    trippy.name = "slime";
    trippy.worldPos = {7.5f, 0.0f, 0.0f};
    trippy.translationRadius = 7.5f;
    trippy.rotationAngle = 0.0f;
    trippy.scaleFactor = {0.6f, 0.6f, 0.6f};
    trippy.translationAngle = 300.0f;
    trippy.translationSpeed = 0.3f;
    trippy.translationAxis = sun.worldPos;

    // Preparar uniforms de planeta Diamante
    Planet diamond;
    diamond.name = "diamond";
    diamond.translationRadius = 9.0f;
    diamond.translationAngle = 220.0f;
    diamond.rotationAngle = 0.0f;
    diamond.worldPos = {diamond.translationRadius * cos(glm::radians(diamond.translationAngle)), 0.0f, diamond.translationRadius * sin(glm::radians(diamond.translationAngle))};
    diamond.scaleFactor = {0.5f, 0.5f, 0.5f};
    diamond.translationSpeed = 0.3f;
    diamond.translationAxis = sun.worldPos;

    // Preparar uniforms de la Luna
    Planet moon;
    moon.name = "moon";
    moon.translationRadius = 0.5f;
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
    phobos.worldPos = {5.0f, 0.3f, 0.0f};
    phobos.translationRadius = 0.5f;
    phobos.rotationAngle = 0.0f;
    phobos.scaleFactor = {0.2f, 0.2f, 0.2f};
    phobos.translationAngle = 50.0f;
    phobos.translationSpeed = 2.0f;
    phobos.translationAxis = mars.worldPos;

    // Preparar uniforms de Deimos
    Planet deimos;
    deimos.name = "moon";
    deimos.worldPos = {4.7f, -0.3f, 0.0f};
    deimos.translationRadius = 0.2f;
    deimos.rotationAngle = 0.0f;
    deimos.scaleFactor = {0.2f, 0.2f, 0.2f};
    deimos.translationAngle = 150.0f;
    deimos.translationSpeed = 2.0f;
    deimos.translationAxis = mars.worldPos;

    // Preparar uniforms de Europa
    Planet europa;
    europa.name = "moon";
    europa.worldPos = {6.8f, 0.1f, 0.0f};
    europa.translationRadius = 0.8f;
    europa.rotationAngle = 0.0f;
    europa.scaleFactor = {0.3f, 0.3f, 0.3f};
    europa.translationAngle = 150.0f;
    europa.translationSpeed = 3.0f;
    europa.translationAxis = jupiter.worldPos;

    //PENDIENTE: diferenciar ángulo de rotación y velocidad de rotación

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
    sun.satelites.push_back(diamond);

    // Preparar uniforms de la nave
    Spaceship spaceship;
    spaceship.worldPos = {0.0f, -1.0f, 10.0f};
    spaceship.scaleFactor = {0.1f, 0.1f, 0.1f};
    spaceship.rotationAngle = 90.0f;

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
                        camera.targetPosition.x += cameraSpeed;
                        break;

                    case SDLK_LEFT:
                        camera.targetPosition.x -= cameraSpeed;
                        break;

                    case SDLK_UP:
                        spaceship.worldPos.z -= cameraSpeed;
                        break;

                    case SDLK_DOWN:
                        spaceship.worldPos.z += cameraSpeed;
                        break;

                    case SDLK_w:
                        camera.targetPosition.y += cameraSpeed;
                        break;

                    case SDLK_s:
                        camera.targetPosition.y -= cameraSpeed;
                        break;

                    default:
                        break;
                }
            }
        }

        clear(100, 100);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Delay(1);

        // Actualizar cámara
        camera.cameraPosition = {spaceship.worldPos.x, spaceship.worldPos.y + 0.1f, spaceship.worldPos.z + 6.0f};

        // Crear la matriz de vista usando el objeto cámara
            uniforms.view = glm::lookAt(
                camera.cameraPosition, // The position of the camera
                camera.targetPosition, // The point the camera is looking at
                camera.upVector        // The up vector defining the camera's orientation
            );

        // Renderizar el Sol
        setUpRender(sun);
        render(Primitive::TRIANGLES, sun.name, verticesArray);

        // Renderizar nave
        glm::mat4 spaceshipTranslation = glm::translate(glm::mat4(1.0f), spaceship.worldPos);
        glm::mat4 spaceshipScale = glm::scale(glm::mat4(1.0f), spaceship.scaleFactor);
        glm::mat4 spaceshipRotation = glm::rotate(glm::mat4(1.0f), glm::radians(spaceship.rotationAngle), rotationAxis);
        uniforms.model = spaceshipTranslation * spaceshipRotation *  spaceshipScale;
        render(Primitive::TRIANGLES, "ship", spaceShipVerticesArray);

       
        for (Planet& planet : sun.satelites) {
            // Renderizar satelites
            for (Planet& satellite : planet.satelites) {
                satellite.translationAxis = planet.worldPos;
                setUpRender(satellite);
                render(Primitive::TRIANGLES, satellite.name, verticesArray);
                renderBuffer(renderer);
            }

            // Renderizar planetas
            setUpRender(planet);
            render(Primitive::TRIANGLES, planet.name, verticesArray);
            renderBuffer(renderer);

        }

        SDL_RenderPresent(renderer);

    }

    return 0;
}