/*
 * SDLVideo.h
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#ifndef SRC_SDLVIDEO_H_
#define SRC_SDLVIDEO_H_

#include <string>
#include "NDIReceiver.h"

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





	NDIReceiver receiver;

	static const unsigned FHD_W = 1920;
	static const unsigned FHD_H = 1080;

	bool run;
};


#endif /* SRC_SDLVIDEO_H_ */
