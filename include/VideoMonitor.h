/*
 * SDLVideo.h
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#ifndef SRC_VIDEOMONITOR_H_
#define SRC_VIDEOMONITOR_H_

#include <string>
#include "EGL_Instance.h"
#include "GBM_Instance.h"
#include "NDI_Receiver.h"
#include "VideoShader.h"

#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

class VideoPoll;
class VideoMonitor;

class VideoFrame
{
public:
	VideoFrame();
	virtual ~VideoFrame(){};

	void Capture(NDIlib_video_frame_v2_t *vf);

	GLuint w() {return m_w;};
	GLuint h() {return m_h;};
	GLuint macro_w() {return m_macro_w;};
	GLuint macro_h() {return m_macro_h;};
	bool   good()	 {return m_good;};

	GLuint plane(GLuint id) { return (id < MAX_PLANES) ? m_planes[id] : 0; };

	enum
	{
		PLANE_A = 0,
		PLANE_B = 1,
		PLANE_C = 2,
		PLANE_D = 3,
		MAX_PLANES = 4
	};
	enum
	{
		BYTES_1 = 1,
		BYTES_2 = 2,
		BYTES_3 = 3,
		BYTES_4 = 4
	};

protected:

	static const GLuint TEXTURE_NONE = 0;

	virtual void do_Capture(NDIlib_video_frame_v2_t *vf);
	void Capture_UYVY_File (const char * file, GLuint w, GLuint h);
	void Capture_UYVY_Frame(NDIlib_video_frame_v2_t *vf);
	void GenTexture(GLuint plane, GLuint w, GLuint h, GLuint bytes, void *data);

	GLuint m_planes[MAX_PLANES];
	GLuint m_w, m_h, m_macro_w, m_macro_h;
	bool m_good;


//	void Lock() 	{ m_lock.lock(); };
//	void Unlock()	{ m_lock.unlock(); };
//	bool pending() { return m_pending; };
//	void pending_clear() { m_pending = false;}
//	uint32_t m_captured, m_dropped;
//	bool m_pending;

private:
	mutex m_lock;
};

class VideoStub: public VideoFrame
{
public:
	VideoStub(const char * file, GLuint w, GLuint h);
	virtual ~VideoStub(){};
};



//
//class VideoFrame_UYVY : public VideoFrame
//{
//public:
//protected:
//	void do_Capture(NDIlib_video_frame_v2_t *vf) override;
//};


extern void VideoPollThread (VideoPoll* poll);

class VideoPoll
{
	friend void VideoPollThread (VideoPoll* poll);
public:
	VideoPoll();
	virtual ~VideoPoll();

	void Start();
	void Stop();
	NDIlib_video_frame_v2_t* PendingVideo();
	void FreeVideo(NDIlib_video_frame_v2_t *vf);
	void FreeAudio(NDIlib_audio_frame_v2_t *af);

private:
	void HandleVideo(NDIlib_video_frame_v2_t *vf);
	void HandleAudio(NDIlib_audio_frame_v2_t *af);

	NDI_Receiver m_ndi;
	bool m_run;

	NDIlib_video_frame_v2_t* m_pending_video;
	NDIlib_audio_frame_v2_t* m_pending_audio;

	uint32_t m_video_captured, m_video_dropped;
	uint32_t m_audio_captured, m_audio_dropped;
	mutex m_videolock, m_audiolock;

};



class VideoMonitor
{
public:
	VideoMonitor();
	~VideoMonitor();

	int Run();
	bool Init(GBM_Instance &gbm, const EGL_Instance &egl);
	void Draw(uint32_t frame_id);

private:
	void Cleanup();
	void Update();
	void Render();

	GLint pDisplay, pFrame, pVideo;
	VideoStub sCino;
//	VideoStub sPattern;
//	VideoStub sIndian;
//	VideoStub sDollar;
//	VideoStub sBlack;

	VideoShader m_UYVY;

	VideoPoll m_poll;
	VideoFrame m_frame;


	static const char shader_file[];
	static const GLfloat vertices[12];
	static const GLfloat vVertices[12];
	static const GLfloat vTexCoords[8];
	static const GLfloat vNormals[12];

};



#endif /* SRC_VIDEOMONITOR_H_ */
