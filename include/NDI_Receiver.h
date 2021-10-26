/*
 * CApplication.h
 *
 *  Created on: 15 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef NDIPLAYER_H_INCLUDED__
#define NDIPLAYER_H_INCLUDED__

#include <Processing.NDI.Advanced.h>
#include <mutex>

using namespace std;

class NDI_Receiver
{
public:
	NDI_Receiver();
	~NDI_Receiver();
	bool Init();
	void Cleanup();

	bool Discover();
	bool IsActive();

	NDIlib_frame_type_e Capture(NDIlib_video_frame_v2_t* vf,
			NDIlib_audio_frame_v2_t* af, NDIlib_metadata_frame_t* mf);

	void FreeVideo(NDIlib_video_frame_v2_t *vf);
	void FreeAudio(NDIlib_audio_frame_v2_t *af);
	void FreeMeta(NDIlib_metadata_frame_t *mf);

private:

	bool m_is_active;
	NDIlib_find_instance_t m_finder;
	NDIlib_recv_create_v3_t m_rx_desc;
	NDIlib_recv_instance_t m_recv;
	NDIlib_framesync_instance_t m_framesync;
	NDIlib_avsync_instance_t m_avsync;
	const NDIlib_source_t * m_sources;

	mutex m_discover_lock;
	static const unsigned int timeout_ms = 5000;

	void Reset();
};

#endif /* NDIPLAYER_H_INCLUDED__ */
