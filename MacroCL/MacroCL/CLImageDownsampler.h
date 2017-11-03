#pragma once
#include "stdafx.h"

using namespace std;

// Class to help to downsample image to W/2, H/2 size.
class CLImageDownsampler
{
private:
	cl_program mDSProgram;
	cl_kernel mDSKernel;
	cl_context mContext;
	cl_command_queue mCommandQueue;
	
public:
	CLImageDownsampler(cl_context context, cl_command_queue commandQueue);
	~CLImageDownsampler();
	
	unique_ptr<CLImage> DownsampleImageHalfSize(const CLImage& inputImage) const;
	void DownsampleImageHalfSize(const CLImage& inputImage, CLImage& outputImage) const;
};
