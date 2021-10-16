/*
 * CApplication.cpp
 *
 *  Created on: 15 окт. 2021 г.
 *      Author: rbalykov
 */

#include <iostream>
#include <stdint.h>

#include "NDIReceiver.h"

#include <SDL2/SDL.h>
#include <Processing.NDI.Advanced.h>
#include <Processing.NDI.Find.h>

using namespace std;

NDIReceiver::NDIReceiver()
{
	finder = NULL;
	sync = NULL;
	rx = NULL;
	sources = NULL;
	src_count = 0;
	is_active = false;

	rx_desc.allow_video_fields = false;
	rx_desc.bandwidth = NDIlib_recv_bandwidth_lowest;
	rx_desc.color_format = NDIlib_recv_color_format_RGBX_RGBA;
	rx_desc.p_ndi_recv_name = "NDI Monitor 0";
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

bool NDIReceiver::Discover()
{
	if (!finder)
		return false;

	sources = NDIlib_find_get_current_sources(finder, &src_count); // @suppress("Invalid arguments")
	if (!src_count)
		return false;
	rx = NDIlib_recv_create_v3(&rx_desc);
	if (!rx)
		return false;
	NDIlib_recv_connect(rx, sources);
	sync = NDIlib_framesync_create(rx);
	is_active = true;
	return true;
}
