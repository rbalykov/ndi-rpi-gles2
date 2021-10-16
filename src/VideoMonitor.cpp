/*
 * SDLVideo.cpp
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#include <VideoMonitor.h>
#include <Log.h>
#include <iostream>
#include <stdint.h>
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_stdinc.h"

using namespace std;

VideoMonitor::VideoMonitor()
{
	run = true;
	display_index = 0;
	fullscreen = false;
	title = "Hello world!";

	window = NULL;
	surface = NULL;
	renderer = NULL;
	texture = NULL;
}

VideoMonitor::~VideoMonitor()
{
}

int VideoMonitor::Run()
{
	if (! Init())
		return -1;

    SDL_Event Event;
    while(run)
    {
		while(SDL_PollEvent(&Event))
			ProcessEvent(&Event);

		Update();
		Render();
    }
    Cleanup();
	return 0;
}

void VideoMonitor::ProcessEvent(const SDL_Event *Event)
{
	switch (Event->type)
	{
	case SDL_KEYDOWN:
		if (Event->key.keysym.sym == SDLK_q)
			run = false;
	break;
	}
}

void VideoMonitor::Update()
{
	receiver.Discover();
}

void VideoMonitor::Render()
{

}
void VideoMonitor::Cleanup()
{
	if (texture)
		SDL_DestroyTexture(texture);
	if (renderer)
		SDL_DestroyRenderer(renderer);
    if (window)
    	SDL_DestroyWindow(window);
    SDL_Quit();
}

bool VideoMonitor::Init()
{
	if ( 0 != SDL_Init(SDL_INIT_EVERYTHING))
	{
		Log::SDLFault("SDL_Init");
		return false;
	}
	if (0 != SDL_GetDesktopDisplayMode(display_index, &dm))
	{
		Log::SDLFault("SDL_GetDesktopDisplayMode");
		return false;
	}
	if (0 != SDL_GetDisplayBounds(display_index, &outputRect))
	{
		Log::SDLFault("SDL_GetDisplayBounds");
		return false;
	}
	SDL_ShowCursor(SDL_DISABLE);
	window = SDL_CreateWindow(title.c_str(), outputRect.x, outputRect.y,
			outputRect.w/4, outputRect.h/4,
			SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		Log::SDLFault("SDL_CreateWindow");
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		Log::SDLFault("SDL_CreateRenderer");
		return false;
	}

//	SDL_CreateWindowAndRenderer(outputRect.w, outputRect.h,
//		SDL_WINDOW_SHOWN ,
//		&window, &renderer);
//	if ((!window) || (!renderer))
//	{
//		Log::SDLFault("SDL_CreateWindowAndRenderer");
//		return false;
//	}
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING , outputRect.w, outputRect.h);
	if (!texture)
	{
		Log::SDLFault("SDL_CreateTexture");
		return false;
	}

	if (!receiver.Init())
	{
		Log::Fault("NDIReceiver.Init");
		return false;
	}

	return true;
}


/*
 * 	SDL_CreateWindowAndRenderer(FHD_W/4, FHD_H/4,
			SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP , &window, &renderer);
	if (!window) return false;
	if (!renderer) return false;
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF );
	SDL_RenderClear(renderer);


	cerr << "init\n";
	window = SDL_CreateWindow(title.c_str(), xpos, ypos, width, height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS);
	if (window == NULL)
		return false;

	cerr << "window\n";

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 255, 0));
    SDL_UpdateWindowSurface(window);
    SDL_Delay(1000);
	renderer = SDL_CreateRenderer(window, -1,
//			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			SDL_RENDERER_SOFTWARE);
	if (renderer == NULL)
		return false;
	cerr << "renderer\n";
    SDL_Delay(3000);

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF );

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING , width, height);
	if (texture == NULL)
		return false;
	cerr << "texture\n";

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
 //   SDL_Delay(1000);

    SDL_RenderDrawLine(renderer, 0, 0, width, height);
	SDL_RenderPresent(renderer);

    cerr << "hello world!\n";
    SDL_UpdateWindowSurface(window);
//    SDL_Delay(3000);

 */
