#pragma once

#ifndef _CLUTILITY_H_
#define _CLUTILITY_H_

#include "stdafx.h"
#include "Utility.h"

namespace CLUtil
{
	class CLException : public std::exception
	{
	private:
		std::string errMsg;
	public:
		CLException(const std::wstring& msg)
			: exception(std::string(msg.begin(), msg.end()).c_str())
		{
			errMsg = Util::WStrToStr(msg);
			throw *this;
		}
	};

	inline const char* MapErrorToString(cl_int error)
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

	inline bool ResultCheck(const cl_int val, const wchar_t* exceptionMsgIfFailed = nullptr)
	{
		if (val != CL_SUCCESS)
		{
			std::string s;
			s.append("OpenCL Error -> ");
			s.append(MapErrorToString(val));
			s.append("\n\n");
			std::wcout << s.c_str();
			if (exceptionMsgIfFailed != nullptr)
			{
				throw(CLException(exceptionMsgIfFailed));
			}
			return false;
		}
		return true;
	}

	inline size_t RoundUpToWGSize(size_t value, size_t wgSize)
	{
		size_t remaining = value % wgSize;
		return value + (remaining == 0 ? 0 : (wgSize - remaining));
	}

	inline void RoundUpToWGSize(size_t elemX, size_t elemY, size_t wgSizeX, size_t wgSizeY, size_t& gSizeX, size_t& gSizeY)
	{
		gSizeX = (size_t)RoundUpToWGSize(elemX, wgSizeX);
		gSizeY = (size_t)RoundUpToWGSize(elemY, wgSizeY);
	}
}

#endif