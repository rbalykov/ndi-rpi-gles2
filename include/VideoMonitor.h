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

using namespace std;

class VideoStub
{
public:
	VideoStub(const char * file, GLuint _w, GLuint _h);
	virtual ~VideoStub();

	bool ok()   {return m_ok;};
	GLuint id() {return m_id;};
private:
	GLuint m_id;
	bool m_ok;
	GLuint w,h, pairs;
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

	bool init_shadertoy(const char *file);
	int load_shader(const char *file);
	int create_program(const char *vs_src, const char *fs_src);
	int link_program(unsigned program);

//	GBM_Instance *gbm;

	/* Shadertoy rendering (to FBO): */
	GLuint stoy_program;
	GLuint stoy_fbo, stoy_fbotex;
	GLint stoy_time_loc;
	GLuint stoy_vbo;

	/* Cube rendering (textures from FBO): */
	GLfloat aspect;
	GLuint program;
	/* uniform handles: */
	GLint modelviewmatrix, modelviewprojectionmatrix, normalmatrix;
	GLint texture;
	GLuint vbo;
	GLuint positionsoffset, texcoordsoffset, normalsoffset;
	GLuint tex[2], tx_UYVY;
	GLint iTime, iFrame, pPairs, pPixels, pVideo;

	uint32_t texw, texh;

	VideoStub sCino;
	VideoStub sPattern;
	VideoStub sIndian;
	VideoStub sBlack;

	static const char shader_file[];
	static const GLfloat vertices[12];
	static const GLfloat vVertices[12];
	static const GLfloat vTexCoords[8];
	static const GLfloat vNormals[12];
	static const char scene_vs[];
	static const char scene_fs[];
	static const char transcode_vs[];
//	static const char transcode_fs_head[];
	static const char transcode_fs_impl[];
//	static const char transcode_fs_tail[];

	static const char fullscreen_vs[];
	static const char fullscreen_fs_head[];
	static const char fullscreen_fs_tail[];

//	NDIReceiver receiver;
};



#endif /* SRC_VIDEOMONITOR_H_ */
