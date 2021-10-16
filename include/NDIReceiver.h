/*
 * CApplication.h
 *
 *  Created on: 15 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef NDIPLAYER_H_INCLUDED__
#define NDIPLAYER_H_INCLUDED__

#include <Processing.NDI.Advanced.h>

class NDIReceiver
{
public:
	NDIReceiver();
	~NDIReceiver();
	bool Init();
	void Cleanup();

	bool Discover();

private:

	bool is_active;
	NDIlib_find_instance_t finder;
	NDIlib_recv_create_v3_t rx_desc;
	NDIlib_recv_instance_t rx;
	NDIlib_framesync_instance_t sync;
	const NDIlib_source_t * sources;
	unsigned int src_count;
};

#endif /* NDIPLAYER_H_INCLUDED__ */
