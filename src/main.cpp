
//#include <iostream>

#include "../include/DRM_Instance.h"
#include "../include/EGL_Instance.h"
#include "../include/GBM_Instance.h"
#include "VideoMonitor.h"
#include "NDI_Receiver.h"

using namespace std;

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

//	NDI_Receiver ndi;
//	if (ndi.Init())
//		{cout << "NDI init done" << endl;}
//	else
//		{cout << "NDI init failed" << endl; return -1; };

	try
	{
	DRM_Instance drm;
	if (drm.Init("/dev/dri/card0", NULL, 60, 100))
		{cout << "DRM init done" << endl;}
	else
		{cout << "DRM init failed" << endl; return -1; };

	GBM_Instance gbm;
//	gbm.Init(drm_fd, w, h, format, modifier, surfaceless)
	if (gbm.Init(drm))
		{cout << "GBM init done" << endl;}
	else
		{cout << "GBM init failed" << endl; return -1; };

	EGL_Instance egl;
	if (egl.Init(gbm))
		{cout << "EGL init done" << endl;}
	else
		{cout << "EGL init failed" << endl; return -1; };

	VideoMonitor monitor;
	if (monitor.Init(gbm, egl))
		{cout << "Monitor init done" << endl;}
	else
		{cout << "Monitor init failed" << endl; return -1; };

		drm.Run(gbm, egl, monitor);
	}
	catch (const std::system_error& e)
	{
		cerr << "system error: " << e.code() << " : " << e.what() << endl;
	}

	return 0;
}
