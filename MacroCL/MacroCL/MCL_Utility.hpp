#pragma once
#include "stdafx.h"

#define SafeDelete(x) {if((x) != nullptr) { delete (x); } x = nullptr; }
#define SafeDeleteArray(x) {if((x) != nullptr) { delete[] (x); } x = nullptr; }
#define SafeReleaseClMemObject(x) {if((x) != MUtil::CL::INVALID_OBJECT) { clReleaseMemObject(x); x =  MUtil::CL::INVALID_OBJECT; } }

typedef sf::Image			SFImage;
typedef sf::Texture		SFTexture;

namespace MUtil
{
	namespace Gen
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
		
		class NonCopyable()
		{
			public:
				NonCopyable() {};
				NonCopyable(const NonCopyable&) = delete;
				NonCopyable& (const NonCopyable&) = delete;
		}
	}
	
	namespace CL
	{
		static unsigned int INVALID_OBJECT = 0xFFFFFFFF;
		static const wcahr_t* INVALID_NAME = L"[INVALID]";
		
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

		static bool ResultCheck(const cl_int val)
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
		
		class CLImage2D :
			public MUtil::Gen::NonCopyable
		{
		private:
			cl_mem						mMemObject;		// OpenCL image memory object
			std::wstring			mName;				// Readable name of the image object
			cl_image_format		mFormat;			// OpenCL image format
			cl_image_desc			mDescr;				// OpenCL image descriptor
			
		public:

			// Destructor
			~CLImage2D()
			{
				SafeReleaseClMemObject(mMemObject)
			}
			
			// Creates the OpenCL image object and initializes internal variables. Throws and exception if failed.
			void CLImage2D(cl_context clContext, const std::wstring& name, size_t w, size_t h, size_t rowPitch, cl_channel_order channelOrder, cl_channel_type channelType, void* imagePtr = nullptr)
			{
				mName = name;
				mFormat.image_channel_order = channelOrder;
				mFormat.image_channel_data_type = channelType;
				// Fill descriptor here
				
				// Create object here
				
			}
			
			// Returns true if the object is alread created and returns false if it is not initialized yet.
			bool IsValid() const
			{
				return mMemObject == INVALID_OBJECT ? false : true;
			}
			
			// Returns the internal cl_mem object
			cl_mem getMemObject() const
			{
				return mMemObject;
			}
			
			// Returns the name of the image object
			std::wstring getName() const
			{
				return mName;
			}
			
			// Returns the format of the image
			cl_image_format getFormat() const
			{
				return mFormat;
			}
			
			// Returns the description of the image
			cl_image_desc getDescription() const
			{
				return mDescr;
			}
		}
		
		// CLImage utility functions
		inline void SFImageToCLImage(const SFImage& src, CLImage& dst)
		{
		}
	}
}
