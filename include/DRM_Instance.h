/*
 * DRMInstance.h
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef SRC_DRMINSTANCE_H_
#define SRC_DRMINSTANCE_H_

#include "common.h"
//#include <iostream>
//#include <fstream>

class GBM_Instance;
class EGL_Instance;
class VideoMonitor;

class DRM_Instance
{
	friend class GBM_Instance;
	friend class EGL_Instance;
public:
	DRM_Instance();
	virtual ~DRM_Instance();
	virtual bool Init(const char *device, const char *mode_str,
			unsigned int vrefresh, unsigned int count);
//	virtual int Run (const struct gbm *gbm, const struct egl *egl);
	int Run (GBM_Instance &gbm, EGL_Instance &egl, VideoMonitor &mon);

private:
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

//	void page_flip_handler(int fd, unsigned int frame,
//				unsigned int sec, unsigned int usec, void *data);

	static int get_resources(int fd, drmModeRes **resources);
	static int find_drm_device(drmModeRes **resources);
	uint32_t find_crtc_for_connector(const drmModeRes *resources,
			const drmModeConnector *connector);
	static uint32_t find_crtc_for_encoder(const drmModeRes *resources,
			const drmModeEncoder *encoder);
	static struct drm_fb* drm_fb_get_from_bo(struct gbm_bo *bo);

};





#endif /* SRC_DRMINSTANCE_H_ */
