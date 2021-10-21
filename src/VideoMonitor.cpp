
#include <VideoMonitor.h>
#include <Log.h>
#include <stdint.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

using namespace std;

#define BOOL_ASSERT(x...)	\
{							\
	if (x)					\
	{						\
		DBG("Error code: ", glGetError());	\
		return false;		\
	}						\
}


VideoMonitor::VideoMonitor()
{
	run = true;

}

VideoMonitor::~VideoMonitor()
{
}

int VideoMonitor::Run()
{
	Init();
	while(run)
	{
		Update();
		Render();
	}
	Cleanup();
	return 0;
}

void VideoMonitor::Update()
{
	receiver.Discover();
}

void VideoMonitor::Render()
{


}
void VideoMonitor::Cleanup()
{

}

static const EGLint context_attributes[] =
{

};

static const EGLint attribute_list[] =
{

};


bool VideoMonitor::Init()
{

	return true;
}


