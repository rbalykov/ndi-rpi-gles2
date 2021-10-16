/*
 * SDLVideo.cpp
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#include "SDLMonitor.h"
#include <iostream>

using namespace std;

SDLMonitor::SDLMonitor()
{
	xpos = 0;
	ypos = 0;
	width = FHD_W/4;
	height = FHD_H/4;
	fullscreen = false;
	title = "Hello world!";

	window = NULL;
	surface = NULL;
	renderer = NULL;
	texture = NULL;
}

SDLMonitor::~SDLMonitor()
{
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (texture)
		SDL_DestroyTexture(texture);
    if (window)
    	SDL_DestroyWindow(window);
    SDL_Quit();
}

bool SDLMonitor::Init()
{
	if ( SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return false;

	window = SDL_CreateWindow(title.c_str(), xpos, ypos, width, height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS);
	if (window == NULL)
		return false;

	surface = SDL_GetWindowSurface(window);
	renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
		return false;

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF );

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
			SDL_TEXTUREACCESS_STREAMING , width, height);
	if (texture == NULL)
		return false;

//	SDL_RenderClear(renderer);
//	SDL_RenderCopy(renderer, texture, NULL, NULL);
//	SDL_RenderPresent(renderer);
//    SDL_Delay(1000);
//
//    SDL_RenderDrawLine(renderer, 0, 0, width, height);
//	SDL_RenderPresent(renderer);
//    SDL_Delay(1000);

//    cerr << "hello world!\n";
//    SDL_FillRect(surface, NULL, SDL_MapRGB( surface->format, 0, 255, 0));
//    SDL_UpdateWindowSurface(window);
	return true;
}

void SDLMonitor::Show()
{

}

void SDLMonitor::Hide()
{

}
