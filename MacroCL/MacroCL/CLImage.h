#pragma once

#ifndef _CLIMAGE_H_
#define _CLIMAGE_H_

#include "stdafx.h"
#include "Utility.h"
#include "CLUtility.h"

class CLImage : public Util::NonCopyable
{
private:
	cl_mem						mMemObject;		// OpenCL image memory object
	std::wstring			mName;				// Readable name of the image object
	cl_image_format		mFormat;			// OpenCL image format
	cl_image_desc			mDescr;				// OpenCL image descriptor
	cl_mem_flags			mMemFlags;		// OpenCL memory flags

public:
	// Destructor
	~CLImage()
	{
		Util::PrintLogLineDebug(std::wstring(L"[-] Releasing CLImage '") + mName + L"'");
		clReleaseMemObject(mMemObject);
	}

	// Creates the OpenCL image object and initializes internal variables. Throws and exception if failed.
	CLImage(cl_context clContext, std::wstring name, size_t w, size_t h, size_t rowPitch, cl_mem_flags memFlags, cl_channel_order channelOrder, cl_channel_type channelType, void* imagePtr = nullptr)
	{
		Util::PrintLogLineDebug(std::wstring(L"[+] Creating CLImage '") + name + L"'");
		mName = name;
		
		mMemFlags = memFlags;
		
		mFormat.image_channel_order = channelOrder;
		mFormat.image_channel_data_type = channelType;
		
		memset(&mDescr, '\0', sizeof(cl_image_desc));
		mDescr.image_type = CL_MEM_OBJECT_IMAGE2D;
		mDescr.image_width = w;
		mDescr.image_row_pitch = rowPitch;
		mDescr.image_height = h;
		mDescr.num_mip_levels = 1;

		cl_int clError;
		mMemObject = clCreateImage(clContext, mMemFlags, &mFormat, &mDescr, (void*)imagePtr, &clError);
		CLUtil::ResultCheck(clError);
	}

	// Returns the internal cl_mem object
	const cl_mem& getMemObject() const
	{
		return mMemObject;
	}

	// Returns the name of the image object
	const std::wstring& getName() const
	{
		return mName;
	}

	// Sets the name of the image object
	const void setName(std::wstring newName)
	{
		mName = newName;
	}

	// Returns the format of the image
	const cl_image_format& getFormat() const
	{
		return mFormat;
	}

	// Returns the description of the image
	const cl_image_desc& getDescription() const
	{
		return mDescr;
	}

	// Returns the memory memory flags of the image object
	const cl_mem_flags& getMemFlags() const
	{
		return mMemFlags;
	}

	size_t GetSizeX() const
	{
		return mDescr.image_width;
	}

	size_t GetSizeEvenX() const
	{
		return mDescr.image_width % 2 == 0 ? mDescr.image_width : mDescr.image_width - 1;
	}

	size_t GetSizeY() const
	{
		return mDescr.image_height;
	}

	size_t GetSizeEvenY() const
	{
		return mDescr.image_height % 2 == 0 ? mDescr.image_height : mDescr.image_height - 1;
	}

	cl_int2 GetSize() const
	{
		return cl_int2{ (cl_int)GetSizeX(), (cl_int)GetSizeY() };
	}

	cl_int2 GetSizeEven() const
	{
		return cl_int2{ (cl_int)GetSizeEvenX(), (cl_int)GetSizeEvenY() };
	}
};

#endif