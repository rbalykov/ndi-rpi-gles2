/*
 * EGLInstance.h
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef SRC_EGLINSTANCE_H_
#define SRC_EGLINSTANCE_H_

#include "GBM_Instance.h"

class GBM_Instance;
class DRM_Instance;

class EGL_Instance
{
	friend class GBM_Instance;
	friend class DRM_Instance;
public:
	EGL_Instance();
	virtual ~EGL_Instance();
	void Draw(uint32_t frame_id);
private:
	bool has_ext(const char *extension_list, const char *ext);
	bool egl_choose_config(EGLDisplay egl_display,
			const EGLint *attribs, EGLint visual_id, EGLConfig *config_out);
	bool create_framebuffer(struct gbm_bo *bo, struct framebuffer *fb);
	int match_config_to_visual(EGLDisplay egl_display, EGLint visual_id,
				EGLConfig *configs, int count);

	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
	struct framebuffer fbs[NUM_BUFFERS];    /* for the surfaceless case */

	PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
	PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
	PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
	PFNEGLWAITSYNCKHRPROC eglWaitSyncKHR;
	PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
	PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID;

	/* AMD_performance_monitor */
	PFNGLGETPERFMONITORGROUPSAMDPROC         glGetPerfMonitorGroupsAMD;
	PFNGLGETPERFMONITORCOUNTERSAMDPROC       glGetPerfMonitorCountersAMD;
	PFNGLGETPERFMONITORGROUPSTRINGAMDPROC    glGetPerfMonitorGroupStringAMD;
	PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC  glGetPerfMonitorCounterStringAMD;
	PFNGLGETPERFMONITORCOUNTERINFOAMDPROC    glGetPerfMonitorCounterInfoAMD;
	PFNGLGENPERFMONITORSAMDPROC              glGenPerfMonitorsAMD;
	PFNGLDELETEPERFMONITORSAMDPROC           glDeletePerfMonitorsAMD;
	PFNGLSELECTPERFMONITORCOUNTERSAMDPROC    glSelectPerfMonitorCountersAMD;
	PFNGLBEGINPERFMONITORAMDPROC             glBeginPerfMonitorAMD;
	PFNGLENDPERFMONITORAMDPROC               glEndPerfMonitorAMD;
	PFNGLGETPERFMONITORCOUNTERDATAAMDPROC    glGetPerfMonitorCounterDataAMD;

	bool modifiers_supported;

public:
	bool Init (const GBM_Instance &gbm);

};

#endif /* SRC_EGLINSTANCE_H_ */
