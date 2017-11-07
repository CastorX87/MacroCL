#pragma once
#ifndef _CLIMAGEDOWNSAMPLESTACK_H_
#define _CLIMAGEDOWNSAMPLESTACK_H_

#include "CLHelper.h"
#include "CLImage.h"
#include "CLImageDownsampler.h"

using namespace std;
using namespace Util;
using namespace CLUtil;

class CLImageDownsampleStack : Util::NonCopyable
{
private:
	vector<unique_ptr<CLImage>> mCLImageStack;
	wstring mName;
	CLImageDownsampler mDownsampler;

private:
	static cl_int2 CalcSizeAtDepthLevel(cl_int2 size, int depthLevel);

public:
	CLImageDownsampleStack(wstring name,cl_context context, cl_device_id device, cl_command_queue commandQueue);
	~CLImageDownsampleStack();

	void SetBaseCLImage(unique_ptr<CLImage> toplevelImage);
	void UpdateDownsampledCLImages(cl_int2 minSizes, int maxDepthLevel, int startAt); // Create new (when there are no downsampled images), update (when all sizes are ok) or delete and create new (when sizes do not pass)

	CLImage& GetCLImageAtDepthLevel(int depthLevel);
	const CLImage& GetCLImageAtDepthLevel(int depthLevel) const;
	size_t GetNumDepthLevels() const;
};

#endif