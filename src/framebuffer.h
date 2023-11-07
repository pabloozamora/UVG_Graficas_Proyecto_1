#pragma once
#include "color.h"
#include <glm/glm.hpp>
#include <vector>
#include <SDL.h>
#include "fragment.h"

void clear(int ox, int oy);

void point(const Fragment&);

void renderBuffer(SDL_Renderer*);