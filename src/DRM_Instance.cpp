/*
 * DRMInstance.cpp
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#include "DRM_Instance.h"
#include "EGL_Instance.h"
#include "GBM_Instance.h"
#include "VideoMonitor.h"


#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>


using namespace std;

WEAK union gbm_bo_handle
gbm_bo_get_handle_for_plane(struct gbm_bo *bo, int plane);

WEAK uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo);

WEAK int
gbm_bo_get_plane_count(struct gbm_bo *bo);

WEAK uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane);

WEAK uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane);

static void
drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));
	struct drm_fb *fb = (struct drm_fb *) data;

	if (fb->fb_id)
		drmModeRmFB(drm_fd, fb->fb_id);

	free(fb);
}


DRM_Instance::DRM_Instance()
{
	fd = -1;
	plane = NULL;
	crtc = NULL;
	connector = NULL;
	crtc_index = -1;
	kms_in_fence_fd = -1;
	kms_out_fence_fd = -1;

	mode = NULL;
	crtc_id = 0;
	connector_id = 0;

	/* number of frames to run for: */
	count = ~0;

}

DRM_Instance::~DRM_Instance()
{
	close(fd);
}

int DRM_Instance::get_resources(int fd, drmModeRes **resources)
{
	*resources = drmModeGetResources(fd);
	if (*resources == NULL)
		return -1;
	return 0;
}

int DRM_Instance::find_drm_device(drmModeRes **resources)
{
	drmDevicePtr devices[MAX_DRM_DEVICES] =
	{ NULL };
	int num_devices, fd = -1;

	num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
	if (num_devices < 0)
	{
		cerr << "drmGetDevices2 failed: " << strerror(-num_devices) << endl;
		return -1;
	}

	for (int i = 0; i < num_devices; i++)
	{
		drmDevicePtr device = devices[i];
		int ret;

		if (!(device->available_nodes & (1 << DRM_NODE_PRIMARY)))
			continue;
		/* OK, it's a primary device. If we can get the
		 * drmModeResources, it means it's also a
		 * KMS-capable device.
		 */
		fd = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR);
		if (fd < 0)
			continue;
		ret = get_resources(fd, resources);
		if (!ret)
			break;
		close(fd);
		fd = -1;
	}
	drmFreeDevices(devices, num_devices);

	if (fd < 0)
		cerr << "no drm device found!" << endl;
	return fd;
}

uint32_t DRM_Instance::find_crtc_for_connector(const drmModeRes *resources,
		const drmModeConnector *connector)
{
	int i;

	for (i = 0; i < connector->count_encoders; i++) {
		const uint32_t encoder_id = connector->encoders[i];
		drmModeEncoder *encoder = drmModeGetEncoder(fd, encoder_id);

		if (encoder) {
			const uint32_t crtc_id = find_crtc_for_encoder(resources, encoder);

			drmModeFreeEncoder(encoder);
			if (crtc_id != 0) {
				return crtc_id;
			}
		}
	}

	/* no match found */
	return -1;
}

uint32_t DRM_Instance::find_crtc_for_encoder(const drmModeRes *resources,
		const drmModeEncoder *encoder)
{
	int i;

	for (i = 0; i < resources->count_crtcs; i++) {
		/* possible_crtcs is a bitmask as described here:
		 * https://dvdhrm.wordpress.com/2012/09/13/linux-drm-mode-setting-api
		 */
		const uint32_t crtc_mask = 1 << i;
		const uint32_t crtc_id = resources->crtcs[i];
		if (encoder->possible_crtcs & crtc_mask) {
			return crtc_id;
		}
	}

	/* no match found */
	return -1;
}

struct drm_fb* DRM_Instance::drm_fb_get_from_bo(struct gbm_bo *bo)
{
	int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));
	struct drm_fb *fb = (struct drm_fb *) gbm_bo_get_user_data(bo);
	uint32_t width, height, format,
		 strides[4] = {0}, handles[4] = {0},
		 offsets[4] = {0}, flags = 0;
	int ret = -1;

	if (fb)
		return fb;

	fb = (struct drm_fb *) calloc(1, sizeof *fb); // @suppress("Invalid arguments")
	fb->bo = bo;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	format = gbm_bo_get_format(bo);

	if (gbm_bo_get_handle_for_plane && gbm_bo_get_modifier &&
	    gbm_bo_get_plane_count && gbm_bo_get_stride_for_plane &&
	    gbm_bo_get_offset) {

		uint64_t modifiers[4] = {0};
		modifiers[0] = gbm_bo_get_modifier(bo);
		const int num_planes = gbm_bo_get_plane_count(bo);
		for (int i = 0; i < num_planes; i++) {
			handles[i] = gbm_bo_get_handle_for_plane(bo, i).u32;
			strides[i] = gbm_bo_get_stride_for_plane(bo, i);
			offsets[i] = gbm_bo_get_offset(bo, i);
			modifiers[i] = modifiers[0];
		}

		if (modifiers[0]) {
			flags = DRM_MODE_FB_MODIFIERS;
			cout << "Using modifier " << modifiers[0] << endl;
		}

		ret = drmModeAddFB2WithModifiers(drm_fd, width, height,
				format, handles, strides, offsets,
				modifiers, &fb->fb_id, flags);
	}

	if (ret) {
		if (flags)
			cerr << "Modifiers failed!" << endl;

		uint32_t desc[4] = {0,0,0,0};
		desc[0] = gbm_bo_get_handle(bo).u32;
		memcpy(handles, &desc, 16); // @suppress("Invalid arguments")
		desc[0] = gbm_bo_get_stride(bo);
		memcpy(strides, &desc, 16); // @suppress("Invalid arguments")
		memset(offsets, 0, 16); // @suppress("Invalid arguments")
		ret = drmModeAddFB2(drm_fd, width, height, format,
				handles, strides, offsets, &fb->fb_id, 0);
	}

	if (ret) {
		cerr << "failed to create fb: " << strerror(errno) << endl;
		free(fb);
		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);

	return fb;
}

bool DRM_Instance::Init(const char *device, const char *mode_str,
		unsigned int vrefresh, unsigned int count)
{
	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeEncoder *encoder = NULL;
	int i, ret, area;

	if (device)
	{
		fd = open(device, O_RDWR);
		ret = get_resources(fd, &resources);
		if (ret < 0 && errno == EOPNOTSUPP)
			cerr << device << " does not look like a modeset device." << endl;
	}
	else
	{
		fd = find_drm_device(&resources);
	}

	if (fd < 0)
	{
		cerr << "could not open drm device" << endl;
		return false;
	}

	if (!resources)
	{
		cerr << "drmModeGetResources failed: " << strerror(errno) << endl;
		return false;
	}

	/* find a connected connector: */
	for (i = 0; i < resources->count_connectors; i++)
	{
		connector = drmModeGetConnector(fd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED)
		{
			/* it's connected, let's use this! */
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}

	if (!connector)
	{
		/* we could be fancy and listen for hotplug events and wait for
		 * a connector..
		 */
		cerr << "no drm connector!" << endl;
		return false;
	}

	/* find user requested mode: */
	if (mode_str && *mode_str)
	{
		for (i = 0; i < connector->count_modes; i++)
		{
			drmModeModeInfo *current_mode = &connector->modes[i];

			if (strcmp(current_mode->name, mode_str) == 0)
			{
				if (vrefresh == 0 || current_mode->vrefresh == vrefresh)
				{
					mode = current_mode;
					break;
				}
			}
		}
		if (!mode)
			cerr << "requested mode not found, using default mode." << endl;
	}

	/* find preferred mode or the highest resolution mode: */
	if (!mode)
	{
		for (i = 0, area = 0; i < connector->count_modes; i++)
		{
			drmModeModeInfo *current_mode = &connector->modes[i];

			if (current_mode->type & DRM_MODE_TYPE_PREFERRED)
			{
				mode = current_mode;
				break;
			}

			int current_area = current_mode->hdisplay * current_mode->vdisplay;
			if (current_area > area)
			{
				mode = current_mode;
				area = current_area;
			}
		}
	}

	if (!mode)
	{
		cerr << "could not find video mode!" << endl;
		return false;
	}

	/* find encoder: */
	for (i = 0; i < resources->count_encoders; i++)
	{
		encoder = drmModeGetEncoder(fd, resources->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if (encoder)
	{
		crtc_id = encoder->crtc_id;
	}
	else
	{
		uint32_t crtc_id = find_crtc_for_connector(resources, connector);
		if (crtc_id == 0)
		{
			cerr << "no crtc found!" << endl;
			return false;
		}

		this->crtc_id = crtc_id;
	}

	for (i = 0; i < resources->count_crtcs; i++)
	{
		if (resources->crtcs[i] == crtc_id)
		{
			crtc_index = i;
			break;
		}
	}

	drmModeFreeResources(resources);

	connector_id = connector->connector_id;
	this->count = count;

	return true;
}

void page_flip_handler(int fd, unsigned int frame,
			unsigned int sec, unsigned int usec, void *data)
{
	/* suppress 'unused parameter' warnings */
	(void)fd, (void)frame, (void)sec, (void)usec;

	int *waiting_for_flip = (int *)data;
	*waiting_for_flip = 0;
}

int DRM_Instance::Run (GBM_Instance &gbm, EGL_Instance &egl, VideoMonitor &mon)
{
	fd_set fds;
	drmEventContext evctx = {
			.version = 2,
			.page_flip_handler = page_flip_handler,
	};
	struct gbm_bo *bo;
	struct drm_fb *fb;
	uint32_t i = 0;
//	uint64_t start_time, report_time, cur_time;
	int ret;

	if (gbm.surface)
	{
		eglSwapBuffers(egl.display, egl.surface);
		bo = gbm_surface_lock_front_buffer(gbm.surface);
	} else {
		bo = gbm.bos[0];
	}
	fb = drm_fb_get_from_bo(bo);
	if (!fb) {
		cerr << "Failed to get a new framebuffer BO" << endl;
		return -1;
	}

	/* set mode: */
	ret = drmModeSetCrtc(fd, crtc_id, fb->fb_id, 0, 0,
			&connector_id, 1, mode);
	if (ret) {
		cerr << "failed to set mode: " << strerror(errno) << endl;
		return ret;
	}

//	start_time = report_time = get_time_ns();

	while (i < count) {
		unsigned frame = i;
		struct gbm_bo *next_bo;
		int waiting_for_flip = 1;

		/* Start fps measuring on second frame, to remove the time spent
		 * compiling shader, etc, from the fps:
		 */
//		if (i == 1) {
//			start_time = report_time = get_time_ns();
//		}

		if (!gbm.surface) {
			glBindFramebuffer(GL_FRAMEBUFFER, egl.fbs[frame % NUM_BUFFERS].fb);
		}

		mon.Draw(i);

		if (gbm.surface) {
			eglSwapBuffers(egl.display, egl.surface);
			next_bo = gbm_surface_lock_front_buffer(gbm.surface);
		} else {
			glFinish();
			next_bo = gbm.bos[frame % NUM_BUFFERS];
		}
		fb = drm_fb_get_from_bo(next_bo);
		if (!fb) {
			cerr << "Failed to get a new framebuffer BO" << endl;
			return -1;
		}

		/*
		 * Here you could also update drm plane layers if you want
		 * hw composition
		 */

		ret = drmModePageFlip(fd, crtc_id, fb->fb_id,
				DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
		if (ret) {
			cerr << "failed to queue page flip: " << strerror(errno) << endl;
			return -1;
		}

		while (waiting_for_flip) {
			FD_ZERO(&fds);
			FD_SET(0, &fds);
			FD_SET(fd, &fds);

			ret = select(fd + 1, &fds, NULL, NULL, NULL);
			if (ret < 0) {
				cerr << "select err: " << strerror(errno) << endl;
				return ret;
			} else if (ret == 0) {
				cerr << "select timeout!" << endl;
				return -1;
			} else if (FD_ISSET(0, &fds)) {
				cerr << "user interrupted!" << endl;
				return 0;
			}
			drmHandleEvent(fd, &evctx);
		}

//		cur_time = get_time_ns();
//		if (cur_time > (report_time + 2 * NSEC_PER_SEC)) {
//			double elapsed_time = cur_time - start_time;
//			double secs = elapsed_time / (double)NSEC_PER_SEC;
//			unsigned frames = i - 1;  /* first frame ignored */
//			printf("Rendered %u frames in %f sec (%f fps)\n",
//				frames, secs, (double)frames/secs);
//			report_time = cur_time;
//		}

		/* release last buffer to render on again: */
		if (gbm.surface) {
			gbm_surface_release_buffer(gbm.surface, bo);
		}
		bo = next_bo;
	}

//	finish_perfcntrs();

//	cur_time = get_time_ns();
//	double elapsed_time = cur_time - start_time;
//	double secs = elapsed_time / (double)NSEC_PER_SEC;
//	unsigned frames = i - 1;  /* first frame ignored */
//	printf("Rendered %u frames in %f sec (%f fps)\n",
//		frames, secs, (double)frames/secs);
//
//	dump_perfcntrs(frames, elapsed_time);

	return 0;
}



