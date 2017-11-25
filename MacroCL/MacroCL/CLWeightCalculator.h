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
	cl_mem mRotMatrices;
	cl_mem mTranslations;
	cl_mem mWeights;

public:
	CLWeightCalculator(std::wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue, int numImages);
	~CLWeightCalculator();

	void CalcWeights(std::vector<std::unique_ptr<CLImage>>& inputImages, std::vector<CLUtil::MMAligmentData>& transforms, std::vector<float>& outputWeights, CLImage& clImageOutput) const;
};

