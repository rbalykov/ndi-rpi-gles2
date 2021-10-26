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

class VideoStub
{
public:
	VideoStub(const char * file, GLuint _w, GLuint _h);
	virtual ~VideoStub();

	bool ok()   {return m_ok;};
	GLuint id() {return m_id;};
	GLuint w() {return m_w;};
	GLuint h() {return m_h;};
	GLuint pairs() {return m_pairs;};
private:
	GLuint m_id;
	bool m_ok;
	GLuint m_w, m_h, m_pairs;
};



class VideoFrame
{
public:
	VideoFrame();
	virtual ~VideoFrame();
	void Capture(NDIlib_video_frame_v2_t *vf);
	void Lock() 	{ m_lock.lock(); };
	void Unlock()	{ m_lock.unlock(); };

	GLuint w() {return m_w;};
	GLuint h() {return m_h;};
	GLuint macro_w() {return m_macro_w; };
	GLuint macro_h() {return m_macro_h; };
	GLuint plane(int id) { return (id < MAX_PLANES) ? m_planes[id] : 0; };
	bool pending() { return m_pending; };
	void pending_clear() { m_pending = false;}

protected:
	virtual void do_Capture(NDIlib_video_frame_v2_t *vf);
	static const uint8_t MAX_PLANES = 4;
	GLuint m_planes[MAX_PLANES];
	GLuint m_w, m_h, m_macro_w, m_macro_h;
	uint32_t m_captured, m_dropped;
	bool m_pending;

private:
	mutex m_lock;
};

class VideoFrame_UYVY : public VideoFrame
{
public:
protected:
	void do_Capture(NDIlib_video_frame_v2_t *vf) override;
};


extern void VideoPollThread (VideoPoll* poll);

class VideoPoll
{
	friend void VideoPollThread (VideoPoll* poll);
public:
	VideoPoll(NDI_Receiver* ndi);
	virtual ~VideoPoll();

	void Start();
	void Stop();
	bool pending() { return m_frame.pending(); };
	VideoFrame* pending_frame() { return &m_frame; };
private:

	void HandleVideo(NDIlib_video_frame_v2_t *vf);
	NDI_Receiver* m_ndi;
	mutex m_lock;
	bool m_run;
	VideoFrame_UYVY m_frame;

//	static const uint8_t BUFFERS = 2;
//	GLuint m_texture_id[BUFFERS];
//	bool m_read_done[BUFFERS];
//	int m_capture_id;

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

//	bool init_shadertoy(const char *file);
//	int load_shader(const char *file);
//	int create_program(const char *vs_src, const char *fs_src);
//	int link_program(unsigned program);

	GLint pDisplay, pFrame, pVideo;
	VideoStub sCino;
	VideoStub sPattern;
	VideoStub sIndian;
	VideoStub sDollar;
	VideoStub sBlack;

	VideoShader m_UYVY;

	NDI_Receiver m_ndi;
	VideoPoll m_poll;


	static const char shader_file[];
	static const GLfloat vertices[12];
	static const GLfloat vVertices[12];
	static const GLfloat vTexCoords[8];
	static const GLfloat vNormals[12];

};



#endif /* SRC_VIDEOMONITOR_H_ */
