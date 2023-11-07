#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "color.h"
#include "objReader.h"
#include "framebuffer.h"
#include "uniforms.h"
#include "shaders.h"
#include "triangle.h"
#include "camera.h"
#include "planet.h"
#include <string>

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Color currentColor;

const std::string modelPath = "../models/sphere.obj";
Color clearColor(0, 0, 0);  // Color del fondo

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<Face> faces;
std::vector<Vertex> verticesArray;
std::vector<std::vector<Vertex>> modelsVertices;

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

void render(Primitive polygon, std::string name){

    // 1. Vertex Shader
    std::vector<Vertex> transformedVertices;

    for (const Vertex& vertex : verticesArray) {
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
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 10.0f);
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

    // Preparar uniforms de Jupiter
    Planet jupiter;
    jupiter.name = "gas";
    jupiter.worldPos = {2.0f, 0.0f, 0.0f};
    jupiter.translationRadius = 2.0f;
    jupiter.rotationAngle = 1.0f;
    jupiter.scaleFactor = {0.7f, 0.7f, 0.7f};
    jupiter.translationAngle = 1.0f;
    jupiter.translationSpeed = 1.0f;
    jupiter.translationAxis = sun.worldPos;

    // Preparar uniforms de la luna
    Planet moon;
    moon.name = "moon";
    moon.worldPos = {2.5f, 0.0f, 0.0f};
    moon.translationRadius = 0.5f;
    moon.rotationAngle = 0.0f;
    moon.scaleFactor = {0.2f, 0.2f, 0.2f};
    moon.translationAngle = 1.0f;
    moon.translationSpeed = 2.0f;
    moon.translationAxis = jupiter.worldPos;

    //PENDIENTE: diferenciar ángulo de rotación y velocidad de rotación

    //Agregar satelites alrededor de sus correspondientes planetas
    jupiter.satelites.push_back(moon);

    // Agregar planetas alrededor del Sol
    sun.satelites.push_back(jupiter);

    loadOBJ(modelPath, vertices, normals, faces);
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
                        camera.cameraPosition.z -= cameraSpeed;
                        break;

                    case SDLK_DOWN:
                        camera.cameraPosition.z += cameraSpeed;
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

        // Crear la matriz de vista usando el objeto cámara
            uniforms.view = glm::lookAt(
                camera.cameraPosition, // The position of the camera
                camera.targetPosition, // The point the camera is looking at
                camera.upVector        // The up vector defining the camera's orientation
            );

        // Renderizar el Sol
        setUpRender(sun);
        render(Primitive::TRIANGLES, sun.name);

       
        for (Planet& planet : sun.satelites) {
            // Renderizar satelites
            for (Planet& satellite : planet.satelites) {
                satellite.translationAxis = planet.worldPos;
                setUpRender(satellite);
                render(Primitive::TRIANGLES, satellite.name);
                renderBuffer(renderer);
            }

            // Renderizar planetas
            setUpRender(planet);
            render(Primitive::TRIANGLES, planet.name);
            renderBuffer(renderer);

        }

        SDL_RenderPresent(renderer);

    }

    return 0;
}