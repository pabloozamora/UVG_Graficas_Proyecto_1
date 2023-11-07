#include <glm/glm.hpp>
#include <SDL.h>
#include <array>
#include "color.h"
#include <vector>
#include "fragment.h"
#include "FastNoiseLite.h"

const int width = 1300;
const int height = 800;

FragColor blank{
  Color{0, 0, 0},
  std::numeric_limits<double>::max()
};

FragColor star{
  Color{255, 255, 255},
  std::numeric_limits<double>::max()
};

std::array<FragColor, width * height> framebuffer;

void clear(int ox, int oy) {
    //std::fill(framebuffer.begin(), framebuffer.end(), blank);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
             FastNoiseLite noiseGenerator;
            noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

            float scale = 1000.0f;
            float noiseValue = noiseGenerator.GetNoise((x + (ox * 100.0f)) * scale, (y + oy * 100.0f) * scale);

            // If the noise value is above a threshold, draw a star
            if (noiseValue > 0.97f) {
                framebuffer[y * width + x] = star;
            } else {
                framebuffer[y * width + x] = blank;
            }
        }
    }

}

void point(const Fragment& f) {

    if (f.z < framebuffer[f.position.y * width + f.position.x].z) {
       framebuffer[f.position.y * width + f.position.x] = FragColor{f.color, f.z};
    }
}

void renderBuffer(SDL_Renderer* renderer) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    void* texturePixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &texturePixels, &pitch);

    Uint32 format = SDL_PIXELFORMAT_ARGB8888;
    SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);

    Uint32* texturePixels32 = static_cast<Uint32*>(texturePixels);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int framebufferY = height - y - 1;
            int index = y * (pitch / sizeof(Uint32)) + x;
            Color& color = framebuffer[framebufferY * width + x].color;
            texturePixels32[index] = SDL_MapRGBA(mappingFormat, color.r, color.g, color.b, color.a);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_Rect textureRect = {0, 0, width, height};
    SDL_RenderCopy(renderer, texture, NULL, &textureRect);
    SDL_DestroyTexture(texture);

}