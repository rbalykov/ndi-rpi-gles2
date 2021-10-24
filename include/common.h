/*
 * common.h
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_



#ifndef GL_ES_VERSION_2_0
#include <GLES2/gl2.h>
#endif
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <gbm.h>
#include <drm_fourcc.h>

#include <stdint.h>
#include <iostream>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* from mesa's util/macros.h: */
#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )
#define MAX2( A, B )   ( (A)>(B) ? (A) : (B) )
#define MIN3( A, B, C ) ((A) < (B) ? MIN2(A, C) : MIN2(B, C))
#define MAX3( A, B, C ) ((A) > (B) ? MAX2(A, C) : MAX2(B, C))

static inline unsigned
u_minify(unsigned value, unsigned levels)
{
	return MAX2(1, value >> levels);
}

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

#ifndef DRM_FORMAT_MOD_INVALID
#define DRM_FORMAT_MOD_INVALID ((((__u64)0) << 56) | ((1ULL << 56) - 1))
#endif

#ifndef EGL_KHR_platform_gbm
#define EGL_KHR_platform_gbm 1
#define EGL_PLATFORM_GBM_KHR              0x31D7
#endif /* EGL_KHR_platform_gbm */

#ifndef EGL_EXT_platform_base
#define EGL_EXT_platform_base 1
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC) (EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC) (EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list);
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT (EGLenum platform, void *native_display, const EGLint *attrib_list);
EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT (EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT (EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list);
#endif
#endif /* EGL_EXT_platform_base */

#ifndef EGL_VERSION_1_5
#define EGLImage EGLImageKHR
#endif /* EGL_VERSION_1_5 */

#define WEAK __attribute__((weak))

/* Define tokens from EGL_EXT_image_dma_buf_import_modifiers */
#ifndef EGL_EXT_image_dma_buf_import_modifiers
#define EGL_EXT_image_dma_buf_import_modifiers 1
#define EGL_DMA_BUF_PLANE3_FD_EXT         0x3440
#define EGL_DMA_BUF_PLANE3_OFFSET_EXT     0x3441
#define EGL_DMA_BUF_PLANE3_PITCH_EXT      0x3442
#define EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT 0x3443
#define EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT 0x3444
#define EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT 0x3445
#define EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT 0x3446
#define EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT 0x3447
#define EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT 0x3448
#define EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT 0x3449
#define EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT 0x344A
#endif

#define NUM_BUFFERS 2

struct gbm {
	struct gbm_device *dev;
	struct gbm_surface *surface;
	struct gbm_bo *bos[NUM_BUFFERS];    /* for the surfaceless case */
	uint32_t format;
	int width, height;
};

struct framebuffer {
	EGLImageKHR image;
	GLuint tex;
	GLuint fb;
};

struct egl {
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

	void (*draw)(uint64_t start_time, unsigned frame);
};

static inline int __egl_check(void *ptr, const char *name)
{
	if (!ptr) {
		std::cerr << "Has no " << name << std::endl;
		return -1;
	}
	return 0;
}

#define egl_check(egl, name) __egl_check((egl)->name, #name)

struct plane {
	drmModePlane *plane;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct crtc {
	drmModeCrtc *crtc;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct connector {
	drmModeConnector *connector;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct drm {
	int fd;

	/* only used for atomic: */
	struct plane *plane;
	struct crtc *crtc;
	struct connector *connector;
	int crtc_index;
	int kms_in_fence_fd;
	int kms_out_fence_fd;

	drmModeModeInfo *mode;
	uint32_t crtc_id;
	uint32_t connector_id;

	/* number of frames to run for: */
	unsigned int count;

	int (*run)(const struct gbm *gbm, const struct egl *egl);
};

struct drm_fb {
	struct gbm_bo *bo;
	uint32_t fb_id;
};

#define MAX_DRM_DEVICES 64


#endif /* INCLUDE_COMMON_H_ */
