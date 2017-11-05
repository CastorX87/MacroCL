#include "stdafx.h"
#include "CLImageComparator.h"

using namespace std;

CLImageComparator::CLImageComparator(wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue, cl_int2 workBufferSize, cl_int2 minSizeDownsample, int maxDepthDownsample)
	: mCmpImgDsStack(wstring(L"Downsample stack of ") + name, context, device, commandQueue)
{
	Util::PrintLogLineDebug(wstring(L"[+] Creating CLImage comparator '") + name + L"'");
	mContext = context;
	mDevice = device;
	mCommandQueue = commandQueue;
	mMinSizeDownsample = minSizeDownsample;
	mMaxDepthDownsample = maxDepthDownsample;
	mCmpImgDsStack.SetBaseCLImage(unique_ptr<CLImage>(new CLImage(mContext, L"Image comparator result", workBufferSize.x, workBufferSize.y, 0, CL_MEM_READ_WRITE, CL_RGBA, CL_UNORM_INT8)));
	mCmpImgDsStack.UpdateDownsampledCLImages(mMinSizeDownsample, mMaxDepthDownsample);
	mCmpImgPixelBuffer.resize(workBufferSize.x * workBufferSize.y);
	CLHelp::LoadProgram(context, device, L"Compare.cl", mCMProgram);
	CLHelp::CreateKernel(mCMProgram, L"CompareMain", mCMKernel);
}

CLImageComparator::~CLImageComparator()
{
}

float CLImageComparator::CompareCLImages(CLUtil::MMAligmentData al, CLImage& imageBase, CLImage& imageToAlign)
{
	float avgDiff = 0;
	
	//cl_float4 matRotation{ cosf(al.rotate / 57.3) / al.scale, -sinf(al.rotate / 57.3) / al.scale,
	//											 sinf(al.rotate / 57.3) / al.scale,  cosf(al.rotate / 57.3) / al.scale };
	cl_float4 matRotation{
		cosf(al.rotate / 57.3) / al.scale, -sinf(al.rotate / 57.3) / al.scale,
		sinf(al.rotate / 57.3) / al.scale,  cosf(al.rotate / 57.3) / al.scale };

	cl_float2 vecTranslation{ -al.translation.x, -al.translation.y };

	size_t gSize[2];
	size_t lSize[2]{ 16, 16 };
	cl_int2 inputSize = imageBase.GetSizeEven();
	cl_float2 imgASize{ imageBase.GetSizeX(), imageBase.GetSizeY() };
	cl_float2 imgBSize{ imageToAlign.GetSizeX(), imageToAlign.GetSizeY() };
	cl_float2 imgWSize{ mCmpImgDsStack.GetCLImageAtDepthLevel(0).GetSizeX(), mCmpImgDsStack.GetCLImageAtDepthLevel(0).GetSizeY() };

	CLUtil::RoundUpToWGSize(imageBase.GetSizeX(), imageBase.GetSizeY(), lSize[0], lSize[1], gSize[0], gSize[1]);
	clSetKernelArg(mCMKernel, 0, sizeof(cl_mem), (void *)&imageBase.getMemObject());
	clSetKernelArg(mCMKernel, 1, sizeof(cl_mem), (void *)&imageToAlign.getMemObject());
	clSetKernelArg(mCMKernel, 2, sizeof(cl_mem), (void *)&mCmpImgDsStack.GetCLImageAtDepthLevel(0).getMemObject());
	clSetKernelArg(mCMKernel, 3, sizeof(cl_float2), (void *)&imgASize);
	clSetKernelArg(mCMKernel, 4, sizeof(cl_float2), (void *)&imgBSize);
	clSetKernelArg(mCMKernel, 5, sizeof(cl_float2), (void *)&imgWSize);
	clSetKernelArg(mCMKernel, 6, sizeof(cl_float4), (void *)&matRotation);
	clSetKernelArg(mCMKernel, 7, sizeof(cl_float2), (void *)&vecTranslation);
	clEnqueueNDRangeKernel(mCommandQueue, mCMKernel, 2, NULL, gSize, lSize, 0, NULL, NULL);
	clFinish(mCommandQueue);

	mCmpImgDsStack.UpdateDownsampledCLImages(mMinSizeDownsample, mMaxDepthDownsample);

	CLImage& clImageToRead = this->GetCompareCLImageDeepest();
	cl_int2 cmpImgSize = clImageToRead.GetSize();
	
	// Read all pixels
	CLHelp::ReadCLImageData(mCommandQueue, clImageToRead, mCmpImgPixelBuffer);

	// Calculate avg diff
	unsigned int DELTA = 0;
	unsigned int DELTA_PIXELS = 0;
	for (int x = 0; x < cmpImgSize.x; x++)
	{
		for (int y = 0; y < cmpImgSize.y; y++)
		{
			uint32_t pixel = mCmpImgPixelBuffer[y * cmpImgSize.x + x];
			if ((pixel >> 24) != 0x00)
			{
				DELTA_PIXELS++;
				DELTA += (pixel & 0x00FF0000 >> 16) + (pixel & 0x0000FF00 >> 8) + pixel & 0x000000FF;
			}
		}
	}
	avgDiff = (DELTA / (double)(DELTA_PIXELS + 1));
	
	return avgDiff;
}