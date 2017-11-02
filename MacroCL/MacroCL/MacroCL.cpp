#include "stdafx.h"
#include "CLUtility.h"
#include "CLHelper.h"

int main()
{
	cl_context clContext;
	cl_command_queue clCommandQueue;
	cl_device_id clDevice;
	CLHelp::InitOpenCL(clContext, clCommandQueue, clDevice);

	cl_program clProgram;
	CLHelp::LoadProgram(clContext, clDevice, L"Downsample.cl", clProgram);

	cl_kernel clKernel;
	CLHelp::CreateKernel(clProgram, L"DownsampleMain", clKernel);

	
	auto rv = CLHelp::CLImageFromFile(clContext, L"C:\\Users\\Castor\\Pictures\\1.jpg");
	size_t gSize[2];
	size_t lSize[2]{ 16, 16 };
	CLUtil::RoundUpToWGSize(rv->GetSizeX(), rv->GetSizeY(), 16, 16, gSize[0], gSize[1]);

	CLImage clImage(clContext, L"Downsampled image (half size)", rv->GetSizeX() / 2, rv->GetSizeY() / 2, 0, CL_MEM_READ_WRITE, CL_RGBA, CL_UNORM_INT8);

	CLUtil::ResultCheck(clSetKernelArg(clKernel, 0, sizeof(cl_mem), (const void *)&rv->getMemObject()));
	CLUtil::ResultCheck(clSetKernelArg(clKernel, 1, sizeof(cl_mem), (const void *)&clImage.getMemObject()));
	CLUtil::ResultCheck(clSetKernelArg(clKernel, 2, sizeof(cl_int2), (const void *)&rv->GetSize()));
	CLUtil::ResultCheck(clEnqueueNDRangeKernel(clCommandQueue, clKernel, 2, NULL, gSize, lSize, 0, NULL, NULL));
	CLUtil::ResultCheck(clFinish(clCommandQueue));

	std::vector<int> pixelData;
	CLHelp::CopyCLImageToMemory(clCommandQueue, clImage, pixelData);

	return 0;
}

