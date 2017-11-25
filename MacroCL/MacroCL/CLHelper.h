#pragma once
#ifndef _CLHELPER_H_
#define _CLHELPER_H_

#include "CLUtility.h"
#include "CLImage.h"

namespace CLHelp
{
	void CLErrorNotify(const char *errinfo, const void *private_info, size_t cb, void *user_data);

	void ReadCLImageData(cl_command_queue clCommandQueue, const CLImage& clImage, std::vector<int>& tgtData);
	void SFImageFromCLImage(cl_command_queue clCommandQueue, const CLImage& clImage, SFImage& sfImageOut);
	std::unique_ptr<CLImage> CLImageFromSFImage(cl_context context, const SFImage& src, cl_mem_flags clMemFlags);
	std::unique_ptr<CLImage> CLImageFromFile(cl_context context, std::wstring filePath, cl_mem_flags clMemFlags);
	void InitOpenCL(cl_context& contextOut, cl_command_queue& commandQueueOut, cl_device_id& selectedDeviceOut);
	void LoadProgram(cl_context clContext, cl_device_id clDevice, const std::wstring sourceFilePath, cl_program& programOut, std::vector<std::tuple<std::string, std::string>>* repDict = nullptr);
	void CreateKernel(cl_program clProgram, std::wstring kernelName, cl_kernel& clKernelOut);
}

#endif