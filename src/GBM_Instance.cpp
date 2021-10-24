/*
 * GBMInstance.cpp
 *
 *  Created on: 21 окт. 2021 г.
 *      Author: rbalykov
 */

#include "../include/GBM_Instance.h"

#include <iostream>
#include "../include/DRM_Instance.h"

using namespace std;
GBM_Instance::GBM_Instance()
{
	dev = NULL;
	surface = NULL;
	format = 0;
	_width = 1920;
	_height = 1080;
	for (unsigned i = 0; i < ARRAY_SIZE(bos); i++)
	{
		bos[i] = NULL;
	}
}

GBM_Instance::~GBM_Instance()
{
	if (surface)
	{
		gbm_surface_destroy(surface);
		surface = NULL;
	}
	for (unsigned i = 0; i < ARRAY_SIZE(bos); i++)
	{
		if (bos[i])
		{
			gbm_bo_destroy(bos[i]);
			bos[i] = NULL;
		}
	}
}

bool GBM_Instance::Init(const DRM_Instance &drm)
{
	return Init(drm.fd, drm.mode->hdisplay, drm.mode->vdisplay,
			DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR, false);
}

bool GBM_Instance::Init(int drm_fd, int w, int h, uint32_t format,
		uint64_t modifier, bool surfaceless)
{
	dev = gbm_create_device(drm_fd);
	this->format = format;
	surface = NULL;

	_width = w;
	_height = h;

	if (surfaceless)
		return init_surfaceless(modifier);

	return init_surface(modifier);
}

bool GBM_Instance::init_surfaceless(uint64_t modifier)
{
	for (unsigned i = 0; i < ARRAY_SIZE(bos); i++)
	{
		bos[i] = init_bo(modifier);
		if (!bos[i])
			return false;
	}
	return true;
}

struct gbm_bo * GBM_Instance::init_bo(uint64_t modifier)
{
	struct gbm_bo *bo = NULL;

	if (modifier != DRM_FORMAT_MOD_LINEAR)
	{
		cerr << "Modifiers requested but support isn't available" << endl;
		return NULL;
	}

	bo = gbm_bo_create(dev, _width, _height, format,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

	if (!bo) {
		cerr << "failed to create gbm bo" << endl;
		return NULL;
	}

	return bo;
}

bool GBM_Instance::init_surface(uint64_t modifier)
{
	if (modifier != DRM_FORMAT_MOD_LINEAR)
	{
		cerr << "Modifiers requested but support isn't available" << endl;
		return NULL;
	}
	surface = gbm_surface_create(dev, _width, _height, format,
					GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

	if (!surface) {
		cerr << "failed to create GBM surface" << endl;
		return false;
	}

	return true;
}
