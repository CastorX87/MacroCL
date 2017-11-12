#pragma once

#include "stdafx.h"
#include "CLHelper.h"
#include "CLImage.h"

class CLImageSharpnessCalc : public Util::NonCopyable
{
private:
	cl_program mCSProgram;
	cl_kernel mCSKernel;
	cl_context mContext;
	cl_device_id mDevice;
	cl_command_queue mCommandQueue;

public:
	CLImageSharpnessCalc(std::wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue);
	~CLImageSharpnessCalc();
	void CalcSharpness(const CLImage& inputImage, CLImage& outputImage, int range) const;
	std::unique_ptr<CLImage> CalcSharpness(const CLImage& inputImage, cl_mem_flags memFlags, int range) const;
};

