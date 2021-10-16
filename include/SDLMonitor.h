/*
 * SDLVideo.h
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#ifndef SRC_SDLVIDEO_H_
#define SRC_SDLVIDEO_H_

#include <string>
#include <SDL2/SDL.h>

class SDLMonitor
{
public:
	SDLMonitor();
	~SDLMonitor();

	static const unsigned FHD_W = 1920;
	static const unsigned FHD_H = 1080;

	void Show();
	void Hide();
	bool Init();

private:

	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	std::string title;
	unsigned int xpos, ypos, width, height;
	bool fullscreen;
};

#endif /* SRC_SDLVIDEO_H_ */
