
//#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

#include <VideoMonitor.h>
#include <Log.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include "Processing.NDI.structs.h"

using namespace std;

void VideoPollThread (VideoPoll* poll);

VideoMonitor::VideoMonitor():
		sCino ("extras/625x799.uyvy", 625, 799)
//		,sPattern("extras/2560x1440.uyvy", 2560, 1440)
//		,sIndian("extras/789x444.uyvy", 789, 444)
//		,sDollar("extras/dollar_800x329.uyvy", 800, 329)
//		,sBlack("extras/black_2x1.uyvy", 2, 1)
		,m_UYVY("extras/envelope2.glsl")
//		,m_ndi()
//		,m_poll()
{
	pDisplay = pFrame = pVideo = -1;
}

VideoMonitor::~VideoMonitor()
{
}

int VideoMonitor::Run()
{

	return 0;
}

void VideoMonitor::Update()
{
//	receiver.Discover();
}

void VideoMonitor::Render()
{


}
void VideoMonitor::Cleanup()
{

}

void VideoMonitor::Draw(uint32_t frame_id)
{
	static uint32_t signal_waiting = 0;
	static const uint32_t signal_timeout = 30;
	(void) frame_id;

/*
	if (m_poll.pending())
	{
		signal_lost = 0;
		VideoFrame* frame = m_poll.pending_frame();
		frame->Lock();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frame->plane(0));
		glUniform1i(pVideo, 0);
		glUniform3f(pFrame, (float)frame->w(), (float)frame->h(),
				(float)frame->macro_w() );
		frame->Unlock();
	}
	else
	{
		signal_lost++;
		if (signal_lost > signal_timeout)
		{
		}
	}
*/
//	start_perfcntrs();
	static uint32_t ix = 0;
	uint32_t i = (ix >> 7) & 0x00000003;
	ix++;
	static VideoFrame* stub = &sCino;
//	if (i == 0) stub = &sCino;
//	else if (i == 1) stub = &sPattern;
//	else if (i == 2) stub = &sIndian;
//	else stub = & sDollar;

	NDIlib_video_frame_v2_t* pv = m_poll.PendingVideo();
	if (pv)
	{
		m_frame.Capture(pv);
		signal_waiting = 0;
		stub = &m_frame;
	}
	else
	{
		signal_waiting++;
		if (signal_waiting > signal_timeout)
		{
			stub = &sCino;
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, stub->plane(VideoFrame::PLANE_A));
	glUniform1i(pVideo, 0);
	glUniform3f(pFrame, (float)stub->w(), (float)stub->h(), (float)stub->macro_w());

	glDrawArrays(GL_TRIANGLES, 0, 6);

//	end_perfcntrs();
}


bool VideoMonitor::Init(GBM_Instance &gbm, const EGL_Instance &egl)
{
	(void) egl;
	GLuint program, vbo;

	if (!m_UYVY.ok())
	{
		cerr << "Missing UYVY shader." << endl;
		return false;
	}
	program = m_UYVY.id();
	m_poll.Start();

	glViewport(0, 0, gbm.w(), gbm.h());
	glUseProgram(program);
	pDisplay = glGetUniformLocation(program, "pDisplay");
	pFrame = glGetUniformLocation(program, "pFrame");
	pVideo = glGetUniformLocation(program, "pVideo");

	glUniform3f(pDisplay, gbm.w(), gbm.h(), 0);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) (intptr_t) 0);
	glEnableVertexAttribArray(0);

	return true;
}

const GLfloat VideoMonitor::vertices[12] =
{
	// First triangle:
	1.0f, 1.0f,
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	// Second triangle:
	-1.0f, -1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f,
};

const GLfloat VideoMonitor::vVertices[] =
{
	-1.0f, -1.0f, 0.0f,
	+1.0f, -1.0f, 0.0f,
	-1.0f, +1.0f, 0.0f,
	+1.0f, +1.0f, 0.0f
};

const GLfloat VideoMonitor::vTexCoords[] =
{
	1.0f, 1.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f
};

const GLfloat VideoMonitor::vNormals[] =
{
	+0.0f, +0.0f, +1.0f, // forward
	+0.0f, +0.0f, +1.0f, // forward
	+0.0f, +0.0f, +1.0f, // forward
	+0.0f, +0.0f, +1.0f, // forward
};

// *****************************************************************************
// *****************************************************************************
// *****************************************************************************


VideoFrame::VideoFrame()
{
	m_w = m_h = m_macro_w = m_macro_h = 1;
	m_good = false;

	for (int i = 0; i < MAX_PLANES; i++)
	{
		m_planes[i] = TEXTURE_NONE;
	}
}

void VideoFrame::Capture(NDIlib_video_frame_v2_t *vf)
{
//	m_lock.lock();
//	if (m_pending) m_dropped++; else m_captured++;
	do_Capture(vf);
//	m_lock.unlock();
}
void VideoFrame::GenTexture(GLuint plane, GLuint w, GLuint h, GLuint bytes, void *data)
{
	if ((!data) || (plane >= MAX_PLANES) || (w < 1) || (h < 1))
		return;

	GLenum format;
	switch (bytes)
	{
	case 1:
		format = GL_RED;
		break;
	case 2:
		format = GL_RG;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		return;
	}
	if (m_planes[plane] == TEXTURE_NONE)
	{
		glGenTextures(1, &m_planes[plane]);
	}
	glBindTexture(GL_TEXTURE_2D, m_planes[plane]);
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format,
			GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void VideoFrame::Capture_UYVY_File(const char * file, GLuint w, GLuint h)
{
	m_h = h; m_w = w;
	m_macro_w = (m_w+1)/2;
	m_macro_h = m_h;

    std::ifstream input( file, std::ios::binary );
    if (!input.good())
    {
    	cerr << "can't open file: " << file << endl;
    	m_good = false;
    	return;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    if (buffer.size() != m_macro_w * m_h * 4)
    {
    	std::cerr << "wrong stub data for file " << file
    			<<", expected "<<  m_macro_w * m_h * 4
    			<< ", got " << buffer.size() << endl;
    	m_good = false;
    	return;
    }
    input.close();

	GenTexture(PLANE_A, m_macro_w, m_h, BYTES_4, buffer.data());
	m_good = true;
}

VideoStub::VideoStub(const char * file, GLuint w, GLuint h) :
		VideoFrame()
{
	Capture_UYVY_File(file, w, h);
}

void VideoFrame::do_Capture(NDIlib_video_frame_v2_t *vf)
{
	if (!vf) return;
	if (!vf->p_data) return;

	switch(vf->FourCC)
	{
	// interleaved YUV (A)
	case NDIlib_FourCC_video_type_UYVY:	// 4:2:2 [Cb Y Cr Y]
		Capture_UYVY_Frame(vf);
		break;
	case NDIlib_FourCC_video_type_UYVA:	// 4:2:2:4 [UYVY] [A]

	// semi-planar YUV (A)
	case NDIlib_FourCC_video_type_PA16:	// 4:2:2:4 [Y] [Cb Cr] [A] 16_BIT
	case NDIlib_FourCC_video_type_P216:	// 4:2:2 [Y] [Cb Cr] 16_BIT
	case NDIlib_FourCC_video_type_NV12: // 4:2:0 [Y] [Cb Cr]

	// planar YUV
	case NDIlib_FourCC_video_type_YV12: // 4:2:0 [Y] [Cr] [Cb]
	case NDIlib_FourCC_video_type_I420: // 4:2:0 [Y] [Cb] [Cr]

	case NDIlib_FourCC_video_type_BGRA:
	case NDIlib_FourCC_video_type_BGRX:

	case NDIlib_FourCC_video_type_RGBX:
	case NDIlib_FourCC_video_type_RGBA:
	default:
		break;
	}
}

void VideoFrame::Capture_UYVY_Frame(NDIlib_video_frame_v2_t *vf)
{
//	cout << "xres " << vf->xres << " yres " << vf->yres
//		 << " aspect= " << vf->picture_aspect_ratio;
//	cout << " rate N " << vf->frame_rate_N << " rate D " << vf->frame_rate_D;
//	cout << " timecode " << vf->timecode << " stamp " << vf->timestamp;
//	cout << "\nsize " << vf->line_stride_in_bytes << endl;

	m_w = vf->xres;
	m_h = vf->yres;
	m_macro_w = vf->line_stride_in_bytes/BYTES_4;
	m_macro_h = m_h;
	GenTexture(PLANE_A, m_macro_w, m_h, BYTES_4, vf->p_data);
}



// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

VideoPoll::VideoPoll()
{
	m_video_captured = m_video_dropped = m_audio_captured = m_audio_dropped = 0;
	m_pending_video = NULL;
	m_pending_audio = NULL;
	m_run = false;

	m_ndi.Init();
}

VideoPoll::~VideoPoll()
{
	Stop();
}


void VideoPollThread (VideoPoll* poll)
{
	cout << "started poll thread" << endl;
	while(poll->m_run)
	{
		poll->m_ndi.Discover();
		NDIlib_video_frame_v2_t vf;
		NDIlib_audio_frame_v2_t af;
		NDIlib_frame_type_e result = poll->m_ndi.Capture(&vf, &af, NULL);
		switch(result)
		{
		case NDIlib_frame_type_video:
			poll->HandleVideo(&vf);
			poll->m_ndi.FreeVideo(&vf);
			break;

		case NDIlib_frame_type_audio:
			poll->m_audio_dropped++;
			poll->m_ndi.FreeAudio(&af);
			break;

		case NDIlib_frame_type_error:
		case NDIlib_frame_type_none:
		case NDIlib_frame_type_metadata:
		case NDIlib_frame_type_status_change:
		default:
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	cout << "stopped poll thread" << endl;
}

void VideoPoll::Start()
{
	if (m_run) return;
	m_run = true;
	std::thread poller(VideoPollThread, this); // @suppress("Invalid arguments")
	poller.detach();
}

void VideoPoll::Stop()
{
	m_run = false;
}

void VideoPoll::FreeVideo(NDIlib_video_frame_v2_t *vf)
{
	m_ndi.FreeVideo(vf);
}

void VideoPoll::FreeAudio(NDIlib_audio_frame_v2_t *af)
{
	m_ndi.FreeAudio(af);
}

void VideoPoll::HandleVideo(NDIlib_video_frame_v2_t *vf)
{
	m_videolock.lock();

	if (m_pending_video == NULL)
	{
		m_pending_video = vf;
		m_video_captured++;
	}
	else
	{
		m_ndi.FreeVideo(m_pending_video);
		m_pending_video = vf;
		m_video_dropped++;
	}
	m_videolock.unlock();
}

NDIlib_video_frame_v2_t* VideoPoll::PendingVideo()
{
	NDIlib_video_frame_v2_t* result;

	m_videolock.lock();
	result = m_pending_video;
	m_pending_video = NULL;
	m_videolock.unlock();

	return result;
}


//
//void VideoFrame_UYVY::do_Capture(NDIlib_video_frame_v2_t *vf)
//{
//	if (!vf) return;
//	if (vf->FourCC != NDIlib_FourCC_video_type_UYVY) return;
//
//	m_h = m_macro_h = vf->yres;
//	m_w = vf->xres;
//	m_macro_w = (m_w+1)/2;
//
//	if (m_planes[0] == 0)
//	{
//		glGenTextures(1, &m_planes[0]);
//	}
//	glBindTexture(GL_TEXTURE_2D, m_planes[0]);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_macro_w, m_h,
//			0, GL_RGBA, GL_UNSIGNED_BYTE, vf->p_data);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glBindTexture(GL_TEXTURE_2D, 0);
//
//	m_pending = true;
//};
