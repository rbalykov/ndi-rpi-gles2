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

class VideoMonitor
{
public:
	VideoMonitor();
	~VideoMonitor();

	int Run();

private:
	bool Init();
	void Cleanup();
	void Update();
	void Render();
	void ProcessEvent(const SDL_Event *Event);

	bool ProbeDisplay();

	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	std::string title;

	SDL_Rect output;
	SDL_DisplayMode dm;
	bool fullscreen;

	static const unsigned FHD_W = 1920;
	static const unsigned FHD_H = 1080;

	bool run;
};


#endif /* SRC_SDLVIDEO_H_ */
