#pragma once

#include "stdafx.h"
#include "CLHelper.h"
#include "CLImage.h"

class CLWeightCalculator : public Util::NonCopyable
{
private:
	cl_program mWCProgram;
	cl_kernel mWCKernel;
	cl_context mContext;
	cl_device_id mDevice;
	cl_command_queue mCommandQueue;

public:
	CLWeightCalculator();
	~CLWeightCalculator();
};

