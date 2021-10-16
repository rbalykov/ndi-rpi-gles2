/*
 * CApplication.cpp
 *
 *  Created on: 15 окт. 2021 г.
 *      Author: rbalykov
 */

#include "NDIReceiver.h"

#include <SDL2/SDL.h>
#include <Processing.NDI.Advanced.h>
#include <iostream>

using namespace std;

NDIReceiver::NDIReceiver()
{
	finder = NULL;
	sync = NULL;
	rx = NULL;
	sources = NULL;
	src_count = 0;
}

NDIReceiver::~NDIReceiver()
{
	if (sync) 	NDIlib_framesync_destroy(sync);
	if (rx)  	NDIlib_recv_destroy(rx);
	if (finder) NDIlib_find_destroy(finder);
	NDIlib_destroy();
}


bool NDIReceiver::Init()
{
	if (!NDIlib_initialize())
		return false;

	finder = NDIlib_find_create_v2();
	if (!finder)
		return false;


	return true;
}

