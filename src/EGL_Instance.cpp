/*
 * EGLInstance.cpp
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#include "../include/EGL_Instance.h"

#include <EGL/egl.h>
#include <cstdlib>
#include <cstring>
#include <cassert>


#include <unistd.h>
#include "GBM_Instance.h"


using namespace std;

EGL_Instance::EGL_Instance()
{
	display = NULL;
	config = NULL;
	context = NULL;
	surface = NULL;
}

EGL_Instance::~EGL_Instance()
{
	// TODO Auto-generated destructor stub
}

bool EGL_Instance::Init (const GBM_Instance &gbm)
{
	EGLint major, minor;

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	const char *egl_exts_client, *egl_exts_dpy, *gl_exts;

#define get_proc_client(ext, name, type) do { \
		if (has_ext(egl_exts_client, #ext)) \
			name = (type)eglGetProcAddress(#name); \
		else \
			name = (type) NULL; \
	} while (0)
#define get_proc_dpy(ext, name, type) do { \
		if (has_ext(egl_exts_dpy, #ext)) \
			name = (type)eglGetProcAddress(#name); \
			else \
				name = (type) NULL; \
	} while (0)

#define get_proc_gl(ext, name, type) do { \
		if (has_ext(gl_exts, #ext)) \
			name = (type)eglGetProcAddress(#name); \
			else \
				name = (type) NULL; \
	} while (0)

	egl_exts_client = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	get_proc_client(EGL_EXT_platform_base, eglGetPlatformDisplayEXT,
			PFNEGLGETPLATFORMDISPLAYEXTPROC);

	if (this->eglGetPlatformDisplayEXT) {
		display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR,
				gbm.dev, NULL);
	} else {
		display = eglGetDisplay((EGLNativeDisplayType)gbm.dev);
	}

	if (!eglInitialize(display, &major, &minor)) {
		cerr << "failed to initialize" << endl;
		return NULL;
	}

	egl_exts_dpy = eglQueryString(display, EGL_EXTENSIONS);
	get_proc_dpy(EGL_KHR_image_base, eglCreateImageKHR, PFNEGLCREATEIMAGEKHRPROC);
	get_proc_dpy(EGL_KHR_image_base, eglDestroyImageKHR, PFNEGLDESTROYIMAGEKHRPROC);
	get_proc_dpy(EGL_KHR_fence_sync, eglCreateSyncKHR, PFNEGLCREATESYNCKHRPROC);
	get_proc_dpy(EGL_KHR_fence_sync, eglDestroySyncKHR, PFNEGLDESTROYSYNCKHRPROC);
	get_proc_dpy(EGL_KHR_fence_sync, eglWaitSyncKHR, PFNEGLWAITSYNCKHRPROC);
	get_proc_dpy(EGL_KHR_fence_sync, eglClientWaitSyncKHR, PFNEGLCLIENTWAITSYNCKHRPROC);
	get_proc_dpy(EGL_ANDROID_native_fence_sync, eglDupNativeFenceFDANDROID, PFNEGLDUPNATIVEFENCEFDANDROIDPROC);

	modifiers_supported = has_ext(egl_exts_dpy,
					"EGL_EXT_image_dma_buf_import_modifiers");

	printf("Using display %p with EGL version %d.%d\n",
			display, major, minor);

	printf("===================================\n");
	printf("EGL information:\n");
	printf("  version: \"%s\"\n", eglQueryString(display, EGL_VERSION));
	printf("  vendor: \"%s\"\n", eglQueryString(display, EGL_VENDOR));
	printf("  client extensions: \"%s\"\n", egl_exts_client);
//	printf("  display extensions: \"%s\"\n", egl_exts_dpy);
	printf("===================================\n");

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("failed to bind api EGL_OPENGL_ES_API\n");
		return NULL;
	}

	if (!egl_choose_config(display, config_attribs, gbm.format,
			&config)) {
		printf("failed to choose config\n");
		return NULL;
	}

	context = eglCreateContext(display, config,
			EGL_NO_CONTEXT, context_attribs);
	if (context == NULL) {
		printf("failed to create context\n");
		return NULL;
	}

	if (!gbm.surface) {
		surface = EGL_NO_SURFACE;
	} else {
		surface = eglCreateWindowSurface(display, config,
				(EGLNativeWindowType)gbm.surface, NULL);
		if (surface == EGL_NO_SURFACE) {
			printf("failed to create EGL surface\n");
			return NULL;
		}
	}

	/* connect the context to the surface */
	eglMakeCurrent(display, surface, surface, context);

	gl_exts = (char *) glGetString(GL_EXTENSIONS);
	printf("OpenGL ES 2.x information:\n");
	printf("  version: \"%s\"\n", glGetString(GL_VERSION));
	printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
//	printf("  extensions: \"%s\"\n", gl_exts);
	printf("===================================\n");

	get_proc_gl(GL_OES_EGL_image, glEGLImageTargetTexture2DOES, PFNGLEGLIMAGETARGETTEXTURE2DOESPROC);

	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorGroupsAMD, PFNGLGETPERFMONITORGROUPSAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCountersAMD, PFNGLGETPERFMONITORCOUNTERSAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorGroupStringAMD, PFNGLGETPERFMONITORGROUPSTRINGAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCounterStringAMD, PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCounterInfoAMD, PFNGLGETPERFMONITORCOUNTERINFOAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glGenPerfMonitorsAMD, PFNGLGENPERFMONITORSAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glDeletePerfMonitorsAMD, PFNGLDELETEPERFMONITORSAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glSelectPerfMonitorCountersAMD, PFNGLSELECTPERFMONITORCOUNTERSAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glBeginPerfMonitorAMD, PFNGLBEGINPERFMONITORAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glEndPerfMonitorAMD, PFNGLENDPERFMONITORAMDPROC);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCounterDataAMD, PFNGLGETPERFMONITORCOUNTERDATAAMDPROC);

	if (!gbm.surface) {
		for (unsigned i = 0; i < ARRAY_SIZE(gbm.bos); i++) {
			if (!create_framebuffer(gbm.bos[i], &fbs[i])) {
				printf("failed to create framebuffer\n");
				return NULL;
			}
		}
	}

	return true;
}


bool EGL_Instance::has_ext(const char *extension_list, const char *ext)
{
	const char *ptr = extension_list;
	int len = strlen(ext);

	if (ptr == NULL || *ptr == '\0')
		return false;

	while (true) {
		ptr = strstr(ptr, ext);
		if (!ptr)
			return false;

		if (ptr[len] == ' ' || ptr[len] == '\0')
			return true;

		ptr += len;
	}
}

bool EGL_Instance::egl_choose_config(EGLDisplay egl_display,
		const EGLint *attribs, EGLint visual_id, EGLConfig *config_out)
{
	EGLint count = 0;
	EGLint matched = 0;
	EGLConfig *configs;
	int config_index = -1;

	if (!eglGetConfigs(egl_display, NULL, 0, &count) || count < 1) {
		cerr << "No EGL configs to choose from." << endl;
		return false;
	}
	configs = (EGLConfig *)malloc(count * sizeof *configs);
	if (!configs)
		return false;

	if (!eglChooseConfig(egl_display, attribs, configs,
					count, &matched) || !matched) {
		cerr << "No EGL configs with appropriate attributes." << endl;
		goto out;
	}

	if (!visual_id)
		config_index = 0;

	if (config_index == -1)
		config_index = match_config_to_visual(egl_display,
							visual_id,
							configs,
							matched);

	if (config_index != -1)
		*config_out = configs[config_index];

out:
	free(configs);
	if (config_index == -1)
		return false;

	return true;
}

bool EGL_Instance::create_framebuffer(struct gbm_bo *bo, struct framebuffer *fb)
{
	assert(eglCreateImageKHR);
	assert(bo);
	assert(fb);

	// 1. Create EGLImage.
	int fd = gbm_bo_get_fd(bo);
	if (fd < 0) {
		cerr << "failed to get fd for bo: " << fd << endl;
		return false;
	}

	EGLint khr_image_attrs[17] = {
		EGL_WIDTH, (EGLint)gbm_bo_get_width(bo),
		EGL_HEIGHT, (EGLint)gbm_bo_get_height(bo),
		EGL_LINUX_DRM_FOURCC_EXT, (int)gbm_bo_get_format(bo),
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint)gbm_bo_get_stride(bo),
		EGL_NONE, EGL_NONE,	/* modifier lo */
		EGL_NONE, EGL_NONE,	/* modifier hi */
		EGL_NONE,
	};

	if (modifiers_supported) {
		const uint64_t modifier = gbm_bo_get_modifier(bo);
		if (modifier != DRM_FORMAT_MOD_LINEAR) {
			size_t attrs_index = 12;
			khr_image_attrs[attrs_index++] =
				EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
			khr_image_attrs[attrs_index++] = modifier & 0xfffffffful;
			khr_image_attrs[attrs_index++] =
				EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
			khr_image_attrs[attrs_index++] = modifier >> 32;
		}
	}

	fb->image = eglCreateImageKHR(display, EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, NULL /* no client buffer */,
			khr_image_attrs);

	if (fb->image == EGL_NO_IMAGE_KHR) {
		 cerr << "failed to make image from buffer object" << endl;
		return false;
	}

	// EGLImage takes the fd ownership
	close(fd);

	// 2. Create GL texture and framebuffer
	glGenTextures(1, &fb->tex);
	glBindTexture(GL_TEXTURE_2D, fb->tex);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, fb->image);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fb->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			fb->tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cerr << "failed framebuffer check for created target buffer" << endl;
		glDeleteFramebuffers(1, &fb->fb);
		glDeleteTextures(1, &fb->tex);
		return false;
	}

	return true;
}

int EGL_Instance::match_config_to_visual(EGLDisplay egl_display,
			EGLint visual_id,
			EGLConfig *configs,
			int count)
{
	int i;

	for (i = 0; i < count; ++i) {
		EGLint id;

		if (!eglGetConfigAttrib(egl_display,
				configs[i], EGL_NATIVE_VISUAL_ID,
				&id))
			continue;

		if (id == visual_id)
			return i;
	}

	return -1;
}




