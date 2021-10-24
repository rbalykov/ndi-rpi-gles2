/*
 * GBMInstance.h
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#ifndef SRC_GBMINSTANCE_H_
#define SRC_GBMINSTANCE_H_

#include <gbm.h>
#include <common.h>

class DRM_Instance;
class EGL_Instance;
class GBM_Instance
{
	friend class EGL_Instance;
	friend class DRM_Instance;
public:
	GBM_Instance();
	virtual ~GBM_Instance();

	bool Init(int drm_fd, int w, int h, uint32_t format,
			uint64_t modifier, bool surfaceless);
	bool Init(const DRM_Instance &drm);

	constexpr int w() {return _width;};
	constexpr int h() {return _height;};

private:
	struct gbm_device *dev;
	struct gbm_surface *surface;
	struct gbm_bo *bos[NUM_BUFFERS];    /* for the surfaceless case */
	uint32_t format;
	int _width, _height;

	bool init_surfaceless(uint64_t modifier);
	struct gbm_bo * init_bo(uint64_t modifier);
	bool init_surface(uint64_t modifier);

};

#endif /* SRC_GBMINSTANCE_H_ */
