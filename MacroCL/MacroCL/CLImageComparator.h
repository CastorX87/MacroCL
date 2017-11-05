#pragma once
#include "stdafx.h"
#include "CLHelper.h"
#include "CLUtility.h"
#include "CLImage.h"
#include "CLImageDownsampler.h"
#include "CLImageDownsampleStack.h"

class CLImageComparator
{
	cl_program mCMProgram;
	cl_kernel mCMKernel;
	cl_context mContext;
	cl_device_id mDevice;
	cl_command_queue mCommandQueue;
	CLImageDownsampleStack mCmpImgDsStack;
	vector<int> mCmpImgPixelBuffer;
	cl_int2 mMinSizeDownsample;
	int mMaxDepthDownsample;

public:
	CLImageComparator(wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue, cl_int2 workBufferSize, cl_int2 minSizeDownsample, int maxDepthDownsample);
	~CLImageComparator();

	float CompareCLImages(CLUtil::MMAligmentData al, CLImage& imageBase, CLImage& imageToAlign);

	CLImage& GetCompareCLImageFullSize() { return mCmpImgDsStack.GetCLImageAtDepthLevel(0); };
	CLImage& GetCompareCLImageDeepest() { return mCmpImgDsStack.GetCLImageAtDepthLevel(mCmpImgDsStack.GetNumDepthLevels() - 1); };
	CLImage& GetCompareCLImageAtDepth(int depthLevel) { return mCmpImgDsStack.GetCLImageAtDepthLevel(depthLevel); };

	inline const CLImageDownsampleStack& GetStack() const { return mCmpImgDsStack; }
};

