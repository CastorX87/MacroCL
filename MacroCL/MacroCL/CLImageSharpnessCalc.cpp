#include "stdafx.h"
#include "CLImageSharpnessCalc.h"

using namespace std;
using namespace Util;
using namespace CLUtil;
using namespace CLHelp;

CLImageSharpnessCalc::CLImageSharpnessCalc(std::wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue)
{
	Util::PrintLogLineDebug(std::wstring(L"[+] Creating CLImage SharpnessCalc'") + name + L"'");

	mContext = context;
	mDevice = device;
	mCommandQueue = commandQueue;

	LoadProgram(context, device, L"Sharpness.cl", mCSProgram);
	CreateKernel(mCSProgram, L"SharpnessMain", mCSKernel);
}

CLImageSharpnessCalc::~CLImageSharpnessCalc()
{
	clReleaseKernel(mCSKernel);
	clReleaseProgram(mCSProgram);
}

void CLImageSharpnessCalc::CalcSharpness(const CLImage& inputImage, CLImage& outputImage, int range) const
{
	Util::PrintLogLineDebug(wstring(L"[*] Resizing CLImage '") + inputImage.getName() + L"' -> '" + outputImage.getName() + L"'");
	size_t gSize[2];
	size_t lSize[2]{ 16, 16 };
	cl_int2 inputSize = inputImage.GetSizeEven();
	CLUtil::RoundUpToWGSize(inputImage.GetSizeX(), inputImage.GetSizeY(), 16, 16, gSize[0], gSize[1]);
	CLUtil::ResultCheck(clSetKernelArg(mCSKernel, 0, sizeof(cl_mem), (const void *)&inputImage.getMemObject()));
	CLUtil::ResultCheck(clSetKernelArg(mCSKernel, 1, sizeof(cl_mem), (const void *)&outputImage.getMemObject()));
	CLUtil::ResultCheck(clSetKernelArg(mCSKernel, 2, sizeof(cl_int2), (const void *)&inputSize));
	CLUtil::ResultCheck(clSetKernelArg(mCSKernel, 3, sizeof(cl_int), (const void *)&range));
	CLUtil::ResultCheck(clEnqueueNDRangeKernel(mCommandQueue, mCSKernel, 2, NULL, gSize, lSize, 0, NULL, NULL));
	CLUtil::ResultCheck(clFinish(mCommandQueue));
}

unique_ptr<CLImage> CLImageSharpnessCalc::CalcSharpness(const CLImage& inputImage, cl_mem_flags memFlags, int range) const
{
	unique_ptr<CLImage> upRetCLImage(new CLImage(mContext, L"Calculationg sharpness map", inputImage.GetSizeEvenX() / 2, inputImage.GetSizeEvenY() / 2, 0, memFlags, CL_RGBA, CL_UNORM_INT8));
	CalcSharpness(inputImage, *upRetCLImage, range);
	return upRetCLImage;
}
