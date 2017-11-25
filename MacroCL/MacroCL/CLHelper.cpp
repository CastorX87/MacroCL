#include "stdafx.h"
#include "CLHelper.h"

void CLHelp::CLErrorNotify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	std::cout << "CLErrorNotify -> " << errinfo << std::endl;
}

void CLHelp::ReadCLImageData(cl_command_queue clCommandQueue, const CLImage& clImage, std::vector<int>& tgtData)
{
	const size_t origin[3]{ 0, 0, 0 };
	const size_t region[3]{ clImage.GetSizeX(), clImage.GetSizeY(), 1 };
	tgtData.resize(clImage.GetSizeX() * clImage.GetSizeY());
	tgtData.shrink_to_fit();
	CLUtil::ResultCheck(clEnqueueReadImage(clCommandQueue, clImage.getMemObject(), CL_TRUE, origin, region, 0, 0, tgtData.data(), 0, NULL, NULL), L"Pixel data of CLImage can't be read!");
	CLUtil::ResultCheck(clFinish(clCommandQueue));
}

void CLHelp::SFImageFromCLImage(cl_command_queue clCommandQueue, const CLImage& clImage, SFImage& sfImageOut)
{
	std::vector<int> pixelData;
	ReadCLImageData(clCommandQueue, clImage, pixelData);
	sfImageOut.create((int)clImage.GetSizeX(), (int)clImage.GetSizeY(), (const sf::Uint8*)pixelData.data());
}

std::unique_ptr<CLImage> CLHelp::CLImageFromSFImage(cl_context context, const SFImage& src, cl_mem_flags clMemFlags)
{
	auto pixels = src.getPixelsPtr();
	std::unique_ptr<CLImage> retCLImage(new CLImage(context, nullptr, (size_t)src.getSize().x, (size_t)src.getSize().y, (size_t)(src.getSize().x * 4), clMemFlags | CL_MEM_COPY_HOST_PTR, CL_RGBA, CL_UNORM_INT8, (void*)pixels));
	return retCLImage;
}

std::unique_ptr<CLImage> CLHelp::CLImageFromFile(cl_context context, std::wstring filePath, cl_mem_flags clMemFlags)
{
	SFImage img;
	img.loadFromFile(Util::WStrToStr(filePath));
	auto pixels = img.getPixelsPtr();
	std::unique_ptr<CLImage> retCLImage(new CLImage(context, std::wstring(L"Image file ") + filePath, (size_t)img.getSize().x, (size_t)img.getSize().y, (size_t)(img.getSize().x * 4), clMemFlags | CL_MEM_COPY_HOST_PTR, CL_RGBA, CL_UNORM_INT8, (void*)pixels));
	return retCLImage;
}

void CLHelp::InitOpenCL(cl_context& contextOut, cl_command_queue& commandQueueOut, cl_device_id& selectedDeviceOut)
{
	cl_uint selectedPlatform = 0;
	cl_uint deviceUsed = 0;
	cl_platform_id platformIds[5];
	cl_uint countPlatforms;
	CLUtil::ResultCheck(clGetPlatformIDs(5, platformIds, &countPlatforms));
	cl_uint numDevices = 0;
	CLUtil::ResultCheck(clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices));
	cl_device_id* devices = new cl_device_id[numDevices];
	CLUtil::ResultCheck(clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, numDevices, devices, NULL));
	cl_context_properties props[10];
	cl_context_properties propsl[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformIds[selectedPlatform], 0 };
	memcpy(props, propsl, sizeof(propsl));
	cl_int errorCode;
	contextOut = clCreateContext(propsl, 1, &devices[deviceUsed], (CLErrorNotify), NULL, &errorCode);
	CLUtil::ResultCheck(errorCode, L"Can't create OpenCL context!");
	commandQueueOut = clCreateCommandQueue(contextOut, devices[deviceUsed], 0, &errorCode);
	CLUtil::ResultCheck(errorCode, L"Can't create OpenCL command queue!");
	selectedDeviceOut = devices[deviceUsed];
	SafeDeleteArray(devices);
}

void CLHelp::LoadProgram(cl_context clContext, cl_device_id clDevice, const std::wstring sourceFilePath, cl_program& programOut, std::vector<std::tuple<std::string, std::string>>* repDict)
{
	cl_int clErrorCode;
	std::string src = Util::WStrToStr(Util::ReadWholeFile(sourceFilePath));

	if (repDict != nullptr)
	{
		for (auto& rpData : *repDict)
		{
			Util::ReplaceStringInPlace(src, std::get<0>(rpData), std::get<1>(rpData));
		}
		std::ofstream fstr("C:\\Programming\\preprocessed.cl");
		fstr << src;
		fstr.close();
	}

	const char* scrPtr = src.c_str();
	size_t clKernelSize = src.length();
	programOut = clCreateProgramWithSource(clContext, 1, (const char **)&(scrPtr), &clKernelSize, &clErrorCode);
	CLUtil::ResultCheck(clErrorCode, L"Can't create OpenCL program!");
	clBuildProgram(programOut, 0, NULL, NULL, NULL, NULL);
	cl_build_status build_status;
	clGetProgramBuildInfo(programOut, clDevice, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
	char *build_log;
	size_t ret_val_size;
	clGetProgramBuildInfo(programOut, clDevice, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
	build_log = new char[ret_val_size + 1];
	clGetProgramBuildInfo(programOut, clDevice, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
	build_log[ret_val_size] = '\0';
	
	std::cout << "BUILD LOG: " << std::endl;
	std::cout << build_log << std::endl;
	delete[] build_log;
}

void CLHelp::CreateKernel(cl_program clProgram, std::wstring kernelName, cl_kernel& clKernelOut)
{
	cl_int clErrorCode;
	clKernelOut = clCreateKernel(clProgram, Util::WStrToStr(kernelName).c_str(), &clErrorCode);
	CLUtil::ResultCheck(clErrorCode, L"Can't create OpenCL kernel!");
}

