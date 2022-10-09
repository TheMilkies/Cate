#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

SDL_Window* create_window(const char* title, int w, int h);
SDL_Renderer* get_renderer(SDL_Window* window);

void render(SDL_Renderer* renderer);

void quit();

void quit(SDL_Window* window, SDL_Renderer* renderer);