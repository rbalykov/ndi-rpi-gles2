
#include <iostream>
#include <stdint.h>

#include "Log.h"

#include <Processing.NDI.Advanced.h>
#include <Processing.NDI.Find.h>
#include "../include/NDI_Receiver.h"

using namespace std;

NDI_Receiver::NDI_Receiver()
{
	finder = NULL;
	sync = NULL;
	rx = NULL;
	sources = NULL;
	src_count = 0;
	is_active = false;

	rx_desc.allow_video_fields = false;
	rx_desc.bandwidth = NDIlib_recv_bandwidth_highest;
	rx_desc.color_format = NDIlib_recv_color_format_fastest;
	rx_desc.p_ndi_recv_name = "NDI Monitor 0";
}

NDI_Receiver::~NDI_Receiver()
{
	Cleanup();
}

bool NDI_Receiver::Init()
{
	if (!NDIlib_initialize())
		return false;

	finder = NDIlib_find_create_v2();
	if (!finder)
		return false;

	return true;
}

bool NDI_Receiver::IsActive()
{
	return is_active;
}

void NDI_Receiver::Cleanup()
{
	if (sync) 	NDIlib_framesync_destroy(sync);
	if (rx)  	NDIlib_recv_destroy(rx);
	if (finder) NDIlib_find_destroy(finder);
	NDIlib_destroy();
}

bool NDI_Receiver::Discover()
{
	if (!finder)
		return false;
	if (is_active)
		return false;

	sources = NDIlib_find_get_current_sources(finder, &src_count);
	if (!src_count)
		return false;
	rx = NDIlib_recv_create_v3(&rx_desc);
	if (!rx)
		return false;
	NDIlib_recv_connect(rx, sources + 0);
	sync = NDIlib_framesync_create(rx);

	DBG("Found NDI source");
	is_active = true;
	return true;
}

