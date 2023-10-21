
#include <iostream>
#include <stdint.h>

#include "Log.h"
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Find.h>
#include <Processing.NDI.Recv.h>
#include <Processing.NDI.structs.h>

#include "NDI_Receiver.h"
#include <iostream>

using namespace std;

NDI_Receiver::NDI_Receiver()
{
	m_finder = NULL;
	m_framesync = NULL;
	m_avsync = NULL;
	m_recv = NULL;
	m_sources = NULL;
	m_is_active = false;

//	rx_desc.allow_video_fields = false;
//	m_rx_desc.bandwidth = NDIlib_recv_bandwidth_highest;
	m_rx_desc.bandwidth = NDIlib_recv_bandwidth_lowest;
	m_rx_desc.color_format = NDIlib_recv_color_format_fastest;
	m_rx_desc.p_ndi_recv_name = "NDI Monitor 0";
}

NDI_Receiver::~NDI_Receiver()
{
	Cleanup();
}

bool NDI_Receiver::Init()
{
	if (!NDIlib_initialize())
		return false;

	return true;
}

bool NDI_Receiver::IsActive()
{
	return m_is_active;
}

void NDI_Receiver::Reset()
{
	m_is_active = false;
	if (m_framesync) 	NDIlib_framesync_destroy(m_framesync);
//	if (m_avsync) NDIlib_avsync_destroy(m_avsync);
	if (m_recv)  	NDIlib_recv_destroy(m_recv);
	if (m_finder) NDIlib_find_destroy(m_finder);

	m_framesync = NULL;
	m_avsync = NULL;
	m_recv = NULL;
	m_finder = NULL;
}

void NDI_Receiver::Cleanup()
{
	Reset();
	NDIlib_destroy();
}

bool NDI_Receiver::Discover()
{
	if (!m_discover_lock.try_lock())
		return false;

	if (m_is_active)
		goto success;

	if (!m_finder)
	{
		m_finder = NDIlib_find_create_v2();
		if (!m_finder)
			goto failure;
	}

	unsigned int src_count;
	m_sources = NDIlib_find_get_current_sources(m_finder, &src_count);
	if (!src_count)
		goto failure;

	cerr << "have source" << endl;
	m_recv = NDIlib_recv_create_v3(&m_rx_desc);
	if (!m_recv)
		goto failure;

	cerr << "have recv" << endl;
	NDIlib_recv_connect(m_recv, m_sources + 0);
//	m_avsync = NDIlib_avsync_create(m_recv);
//	if (!m_avsync)
//		goto failure;
//
//	cerr << "have avsync" << endl;

	m_is_active = true;

	success:
	m_discover_lock.unlock();
	return true;

	failure:
	m_discover_lock.unlock();
	return false;
}

NDIlib_frame_type_e
NDI_Receiver::Capture(NDIlib_video_frame_v2_t* vf,
		NDIlib_audio_frame_v2_t* af, NDIlib_metadata_frame_t* mf)
{
	if (!m_is_active)
		return NDIlib_frame_type_error;

	m_discover_lock.lock();
	NDIlib_frame_type_e result = NDIlib_recv_capture_v2(m_recv, vf, af, mf, timeout_ms);
	if (result == NDIlib_frame_type_error)
	{
		Reset();
	}
	m_discover_lock.unlock();
	return result;
}

void NDI_Receiver::FreeVideo(NDIlib_video_frame_v2_t *vf)
{
	if (m_recv) NDIlib_recv_free_video_v2(m_recv, vf);
}
void NDI_Receiver::FreeAudio(NDIlib_audio_frame_v2_t *af)
{
	if (m_recv) NDIlib_recv_free_audio_v2(m_recv, af);
}
void NDI_Receiver::FreeMeta(NDIlib_metadata_frame_t *mf)
{
	if (m_recv) NDIlib_recv_free_metadata(m_recv, mf);
}



