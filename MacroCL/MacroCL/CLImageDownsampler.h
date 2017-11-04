#pragma once

#ifndef _CLIMAGEDOWNSAMPLER_H_
#define _CLIMAGEDOWNSAMPLER_H_

#include "stdafx.h"
#include "CLHelper.h"
#include "CLImage.h"

using namespace CLHelp;
using namespace CLUtil;
using namespace Util;
using namespace std;

// Class to help to downsample image to W/2, H/2 size.
class CLImageDownsampler : public NonCopyable
{
private:
	cl_program mDSProgram;
	cl_kernel mDSKernel;
	cl_context mContext;
	cl_device_id mDevice;
	cl_command_queue mCommandQueue;
	
public:
	CLImageDownsampler(cl_context context, cl_device_id device, cl_command_queue commandQueue);
	~CLImageDownsampler();
	void DownsampleImageHalfSize(const CLImage& inputImage, CLImage& outputImage) const;
	unique_ptr<CLImage> DownsampleImageHalfSize(const CLImage& inputImage, cl_mem_flags memFlags) const;

	inline cl_context GetContext() { return mContext; }
};

#endif