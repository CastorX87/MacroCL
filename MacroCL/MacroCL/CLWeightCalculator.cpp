#include "stdafx.h"
#include "CLWeightCalculator.h"
#include "CLImage.h"

using namespace std;
using namespace Util;
using namespace CLUtil;
using namespace CLHelp;

CLWeightCalculator::CLWeightCalculator(std::wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue, int numImages)
{
	Util::PrintLogLineDebug(std::wstring(L"[+] Creating CLImage WeightCalc'") + name + L"'");

	mContext = context;
	mDevice = device;
	mCommandQueue = commandQueue;

	cl_int error;
	mRotMatrices = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float4) * 128, nullptr, &error);
	CLUtil::ResultCheck(error);
	mTranslations = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float2) * 128, nullptr, &error);
	CLUtil::ResultCheck(error);
	mWeights = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float) * 128, nullptr, &error);
	CLUtil::ResultCheck(error);

	string DEF_IN_IMAGE_ARGS = string("");
	for (int i = 0; i < numImages; i++)
	{
		DEF_IN_IMAGE_ARGS += string("__read_only image2d_t img") + to_string(i); // Start numbering at zero
		if (i < numImages - 1)
			DEF_IN_IMAGE_ARGS += string(",\n");
	}

	string DEF_FILL_ALL_IMAGE_COLOR = string("");
	for (int i = 0; i < numImages; i++)
	{
		DEF_FILL_ALL_IMAGE_COLOR += string("pos = gPos + translations[") + to_string(i) + string("] - imgHalfSize;\npos = ((float2)(pos.x * matRotScales[") + to_string(i) + string("].x + pos.y * matRotScales[") + to_string(i) + string("].z, pos.x * matRotScales[") + to_string(i) + string("].y + pos.y * matRotScales[") + to_string(i) + string("].w)) + (imgHalfSize);\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImages[") + to_string(i) + string("] = read_imagef(img") + to_string(i) + string(", sampler, pos);\n");
		
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] = read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(0, -1));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(0, 1));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(-1, 0));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(1, 0));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(-1, -1));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(-1, 1));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(1, -1));\n");
		DEF_FILL_ALL_IMAGE_COLOR += string("colorAllImagesE[") + to_string(i) + string("] += read_imagef(img") + to_string(i) + string(", sampler, pos + (float2)(1, 1));\n");
	}

	vector<tuple<string, string>> replaceData;
	replaceData.push_back(std::make_tuple("[[DEF_IN_IMAGE_ARGS]]", DEF_IN_IMAGE_ARGS));
	replaceData.push_back(std::make_tuple("[[DEF_FILL_ALL_IMAGE_COLOR]]", DEF_FILL_ALL_IMAGE_COLOR));

	LoadProgram(context, device, L"WeightCalc.cl", mWCProgram, &replaceData);
	CreateKernel(mWCProgram, L"WeightCalcMain", mWCKernel);
}

CLWeightCalculator::~CLWeightCalculator()
{
	clReleaseKernel(mWCKernel);
	clReleaseProgram(mWCProgram);
}

void CLWeightCalculator::CalcWeights(std::vector<unique_ptr<CLImage>>& inputImages, std::vector<CLUtil::MMAligmentData>& transforms, std::vector<float>& outputWeights, CLImage& clImageOutput) const
{
	// Create separate vector for rotation and translation
	std::vector<cl_float4> rotMatrices(128);
	std::vector<cl_float2> translations(128);
	
	for(int i = 0; i < transforms.size(); i++)
	{
		transforms[i].ToRotAndTransInverse(rotMatrices[i], translations[i]);
	}

	outputWeights.resize(inputImages.size());

	clEnqueueWriteBuffer(mCommandQueue, mRotMatrices, CL_TRUE, 0, rotMatrices.size() * sizeof(cl_float4), rotMatrices.data(), NULL, NULL, NULL);
	clEnqueueWriteBuffer(mCommandQueue, mTranslations, CL_TRUE, 0, translations.size() * sizeof(cl_float2), translations.data(), NULL, NULL, NULL);

	cl_int numImages = inputImages.size();
	size_t gSize[2];
	size_t lSize[2]{ 16, 16 };
	cl_int2 inputSize = inputImages[0]->GetSize();
	cl_float2 imgSize{ inputSize.x, inputSize.y };

	CLUtil::RoundUpToWGSize(inputSize.x, inputSize.y, lSize[0], lSize[1], gSize[0], gSize[1]);
	clSetKernelArg(this->mWCKernel, 0, sizeof(cl_int), (void *)&numImages);
	clSetKernelArg(this->mWCKernel, 1, sizeof(cl_float2), (void *)&imgSize);
	clSetKernelArg(this->mWCKernel, 2, sizeof(cl_mem), (void *)&mRotMatrices);
	clSetKernelArg(this->mWCKernel, 3, sizeof(cl_mem), (void *)&mTranslations);
	clSetKernelArg(this->mWCKernel, 4, sizeof(cl_mem), (void *)&mWeights);
	clSetKernelArg(this->mWCKernel, 5, sizeof(cl_mem), (void *)&clImageOutput.getMemObject());
	for (int inputImgArgIndex = 0; inputImgArgIndex < inputImages.size(); inputImgArgIndex++)
	{
		clSetKernelArg(this->mWCKernel, 6 + inputImgArgIndex, sizeof(cl_mem), (void *)&inputImages[inputImgArgIndex]->getMemObject());
	}

	clEnqueueNDRangeKernel(mCommandQueue, mWCKernel, 2, NULL, gSize, lSize, 0, NULL, NULL);
	clFinish(mCommandQueue);
}

