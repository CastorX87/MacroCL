#include "stdafx.h"
#include "CLImageDownsampler.h"

using namespace CLHelp;
using namespace CLUtil;
using namespace std;

CLImageDownsampler::CLImageDownsampler(cl_context context, cl_device_id device, cl_command_queue commandQueue)
{
	mContext = context;
	mDevice = device;
	mCommandQueue = commandQueue;

	LoadProgram(context, device, L"Downsample.cl", mDSProgram);
	CreateKernel(mDSProgram, L"DownsampleMain", mDSKernel);
}

CLImageDownsampler::~CLImageDownsampler()
{
	clReleaseKernel(mDSKernel);
	clReleaseProgram(mDSProgram);
}

void CLImageDownsampler::DownsampleImageHalfSize(const CLImage& inputImage, CLImage& outputImage) const
{
	size_t gSize[2];
	size_t lSize[2]{ 16, 16 };
	cl_int2 inputSize = inputImage.GetSizeEven();
	CLUtil::RoundUpToWGSize(inputImage.GetSizeX(), inputImage.GetSizeY(), 16, 16, gSize[0], gSize[1]);
	CLUtil::ResultCheck(clSetKernelArg(mDSKernel, 0, sizeof(cl_mem), (const void *)&inputImage.getMemObject()));
	CLUtil::ResultCheck(clSetKernelArg(mDSKernel, 1, sizeof(cl_mem), (const void *)&outputImage.getMemObject()));
	CLUtil::ResultCheck(clSetKernelArg(mDSKernel, 2, sizeof(cl_int2), (const void *)&inputSize));
	CLUtil::ResultCheck(clEnqueueNDRangeKernel(mCommandQueue, mDSKernel, 2, NULL, gSize, lSize, 0, NULL, NULL));
	CLUtil::ResultCheck(clFinish(mCommandQueue));
}

unique_ptr<CLImage> CLImageDownsampler::DownsampleImageHalfSize(const CLImage& inputImage, cl_mem_flags memFlags) const
{
	unique_ptr<CLImage> upRetCLImage(new CLImage(mContext, L"Downsampled image (half size)", inputImage.GetSizeEvenX() / 2, inputImage.GetSizeEvenY() / 2, 0, memFlags, CL_RGBA, CL_UNORM_INT8));
	DownsampleImageHalfSize(inputImage, *upRetCLImage);
	return upRetCLImage;
}
