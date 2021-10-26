
#include <VideoMonitor.h>
#include <Log.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <err.h>
//#include <sys/mman.h>
//#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>

using namespace std;

void VideoPollThread (VideoPoll* poll);

VideoMonitor::VideoMonitor():
		sCino (VideoStub ("extras/625x799.uyvy", 625, 799)),
		sPattern("extras/2560x1440.uyvy", 2560, 1440),
		sIndian("extras/789x444.uyvy", 789, 444),
		sDollar("extras/dollar_800x329.uyvy", 800, 329),
		sBlack("extras/black_2x1.uyvy", 2, 1),
		m_UYVY("extras/envelope2.glsl"),
		m_ndi(),
		m_poll(&m_ndi)
{
	pDisplay = pFrame = pVideo = -1;
}

VideoMonitor::~VideoMonitor()
{
	m_poll.Stop();
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
	static uint32_t signal_lost = 0;
	static const uint32_t signal_timeout = 120;
//	(void) frame_id;
//	glUniform1f(iTime, (get_time_ns() - start_time) / (double) NSEC_PER_SEC);
	// Replace the above to input elapsed time relative to 60 FPS
	// glUniform1f(iTime, (float) frame / 60.0f);


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
	}
	else
	{
		signal_lost++;
		if (signal_lost > signal_timeout)
		{
			static uint32_t ix = 0;
			uint32_t i = (ix >> 7) & 0x00000003;
			ix++;
			VideoStub* stub;
			if (i == 0) stub = &sCino;
			else if (i == 1) stub = &sPattern;
			else if (i == 2) stub = &sIndian;
			else stub = & sDollar;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, stub->id());
			glUniform1i(pVideo, 0);
			glUniform3f(pFrame, (float)stub->w(), (float)stub->h(), (float)stub->pairs());
		}
	}



//	start_perfcntrs();

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
	std::thread poller(VideoPollThread, &m_poll);
	poller.detach();



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

VideoStub::VideoStub(const char * file, GLuint _w, GLuint _h)
{
	m_h = _h; m_w = _w; m_pairs = (m_w+1)/2;
    std::ifstream input( file, std::ios::binary );
    if (!input.good())
    {
    	cerr << "can't open file: " << file << endl;
    	m_ok = false;
    	return;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    if (buffer.size() != m_pairs * m_h * 4)
    {
    	std::cerr << "wrong stub data for file " << file
    			<<", expected "<<  m_pairs * m_h * 4
    			<< ", got " << buffer.size() << endl;
    	m_ok = false;
    	return;
    }
    input.close();

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pairs, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_ok = true;
};

VideoStub::~VideoStub() {}

VideoPoll::VideoPoll(NDI_Receiver* ndi)
{
	m_ndi = ndi;
	m_run = true;
}

VideoPoll::~VideoPoll() {}


void VideoPollThread (VideoPoll* poll)
{
	if (!poll->m_ndi)
	{
		cerr << "Poll thread: NDI receiver is NULL" << endl;
		return;
	}
	cout << "started poll thread" << endl;
	while(poll->m_run)
	{
		poll->m_ndi->Discover();
		NDIlib_video_frame_v2_t vf;
		NDIlib_audio_frame_v2_t af;
		NDIlib_frame_type_e result = poll->m_ndi->Capture(&vf, &af, NULL);
		switch(result)
		{
		case NDIlib_frame_type_video:
			poll->HandleVideo(&vf);
			poll->m_ndi->FreeVideo(&vf);
			break;

		case NDIlib_frame_type_audio:
			poll->m_ndi->FreeAudio(&af);
			break;

		case NDIlib_frame_type_error:
		case NDIlib_frame_type_none:
		case NDIlib_frame_type_metadata:
		case NDIlib_frame_type_status_change:
		default:
			break;
		}
	}
	cout << "stopped poll thread" << endl;
}

void VideoPoll::Start()
{
	m_run = true;
}

void VideoPoll::Stop()
{
	m_run = false;
}

void VideoPoll::HandleVideo(NDIlib_video_frame_v2_t *vf)
{
	if (!vf) return;
	if (!vf->p_data) return;

	switch(vf->FourCC)
	{
	// interleaved YUV (A)
	case NDIlib_FourCC_video_type_UYVY:	// 4:2:2 [Cb Y Cr Y]
		m_frame.Capture(vf);
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


VideoFrame::VideoFrame()
{
	m_w = m_h = m_macro_w = m_macro_h = 1;
	m_captured = m_dropped = 0;
	m_pending = false;

	for (int i = 0; i < MAX_PLANES; i++)
	{
		m_planes[i] = 0;
	}
}

VideoFrame::~VideoFrame()
{
	cout << "UYVY frames: captured " << m_captured
			<< ", dropped " << m_dropped << endl;
}

void VideoFrame::do_Capture(NDIlib_video_frame_v2_t *vf)
{
	(void) vf;
}

void VideoFrame::Capture(NDIlib_video_frame_v2_t *vf)
{
	m_lock.lock();
	if (m_pending) m_dropped++; else m_captured++;
	do_Capture(vf);
	m_lock.unlock();
}


void VideoFrame_UYVY::do_Capture(NDIlib_video_frame_v2_t *vf)
{
	if (!vf) return;
	if (vf->FourCC != NDIlib_FourCC_video_type_UYVY) return;

	m_h = m_macro_h = vf->yres;
	m_w = vf->xres;
	m_macro_w = (m_w+1)/2;

	if (m_planes[0] == 0)
	{
		glGenTextures(1, &m_planes[0]);
	}
	glBindTexture(GL_TEXTURE_2D, m_planes[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_macro_w, m_h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, vf->p_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_pending = true;

};
