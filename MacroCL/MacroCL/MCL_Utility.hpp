#pragma once
#include "stdafx.h"

#define SafeDelete(x) {if((x) != nullptr) { delete (x); } x = nullptr; }
#define SafeDeleteArray(x) {if((x) != nullptr) { delete[] (x); } x = nullptr; }

namespace MCL
{
	namespace Util
	{
		static std::string ReadWholeFile(const std::string& path)
		{
			std::ifstream in(path);
			return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
		}

		static std::wstring StrToWStr(const std::string& str)
		{
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_typeX, wchar_t> converterX;

			return converterX.from_bytes(str);
		}

		static std::string WStrToStr(const std::wstring& wstr)
		{
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_typeX, wchar_t> converterX;

			return converterX.to_bytes(wstr);
		}
	}
	
	namespace ClUtil
	{
		static const char* MapErrorToString(cl_int error)
		{
			static const char* errorString[] = {
				"CL_SUCCESS",
				"CL_DEVICE_NOT_FOUND",
				"CL_DEVICE_NOT_AVAILABLE",
				"CL_COMPILER_NOT_AVAILABLE",
				"CL_MEM_OBJECT_ALLOCATION_FAILURE",
				"CL_OUT_OF_RESOURCES",
				"CL_OUT_OF_HOST_MEMORY",
				"CL_PROFILING_INFO_NOT_AVAILABLE",
				"CL_MEM_COPY_OVERLAP",
				"CL_IMAGE_FORMAT_MISMATCH",
				"CL_IMAGE_FORMAT_NOT_SUPPORTED",
				"CL_BUILD_PROGRAM_FAILURE",
				"CL_MAP_FAILURE",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"CL_INVALID_VALUE",
				"CL_INVALID_DEVICE_TYPE",
				"CL_INVALID_PLATFORM",
				"CL_INVALID_DEVICE",
				"CL_INVALID_CONTEXT",
				"CL_INVALID_QUEUE_PROPERTIES",
				"CL_INVALID_COMMAND_QUEUE",
				"CL_INVALID_HOST_PTR",
				"CL_INVALID_MEM_OBJECT - EX: WRONG R/W RIGHTS? WRONG STORAGE CLASS? WRONG ARG INDEX?",
				"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
				"CL_INVALID_IMAGE_SIZE",
				"CL_INVALID_SAMPLER",
				"CL_INVALID_BINARY",
				"CL_INVALID_BUILD_OPTIONS",
				"CL_INVALID_PROGRAM",
				"CL_INVALID_PROGRAM_EXECUTABLE",
				"CL_INVALID_KERNEL_NAME",
				"CL_INVALID_KERNEL_DEFINITION",
				"CL_INVALID_KERNEL",
				"CL_INVALID_ARG_INDEX",
				"CL_INVALID_ARG_VALUE",
				"CL_INVALID_ARG_SIZE",
				"CL_INVALID_KERNEL_ARGS",
				"CL_INVALID_WORK_DIMENSION",
				"CL_INVALID_WORK_GROUP_SIZE",
				"CL_INVALID_WORK_ITEM_SIZE",
				"CL_INVALID_GLOBAL_OFFSET",
				"CL_INVALID_EVENT_WAIT_LIST",
				"CL_INVALID_EVENT",
				"CL_INVALID_OPERATION",
				"CL_INVALID_GL_OBJECT",
				"CL_INVALID_BUFFER_SIZE",
				"CL_INVALID_MIP_LEVEL",
				"CL_INVALID_GLOBAL_WORK_SIZE",
			};

			const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

			const int index = -error;

			return (index >= 0 && index < errorCount) ? errorString[index] : "";
		}

		static bool ResCheck(const cl_int val)
		{
			if (val != CL_SUCCESS)
			{
				std::string s;
				s.append("\OpenCL Error -> ");
				s.append(MapErrorToString(val));
				s.append("\n\n");
				std::wcout << s.c_str();
				return true;
			}
			return false;
		}

		static size_t RoundUpToWGSize(int value, int wgSize)
		{
			int remaining = value % wgSize;
			return value + (remaining == 0 ? 0 : (wgSize - remaining));
		}
	}
}