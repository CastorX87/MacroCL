#include "stdafx.h"
#include "CLImageDownsampleStack.h"

using namespace std;

CLImageDownsampleStack::CLImageDownsampleStack(wstring name, cl_context context, cl_device_id device, cl_command_queue commandQueue)
: mDownsampler(wstring(L"Downsampler of ") + name, context, device, commandQueue)
{
	Util::PrintLogLineDebug(wstring(L"[+] Creating CLImage downsample stack '") + name + L"'");
	mName = name;
}

CLImageDownsampleStack::~CLImageDownsampleStack()
{
	Util::PrintLogLineDebug(std::wstring(L"[-] Releasing CLImage stack '") + mName + L"'");
	mCLImageStack.clear();
}

cl_int2 CLImageDownsampleStack::CalcSizeAtDepthLevel(cl_int2 size, int depthLevel)
{
	cl_int2 retSize = size;
	for (int dl = 0; dl < depthLevel; dl++)
	{
		retSize.x /= 2;
		retSize.y /= 2;
	}
	return retSize;
}

void CLImageDownsampleStack::SetBaseCLImage(unique_ptr<CLImage> toplevelImage)
{
	Util::PrintLogLineDebug(wstring(L"[*] Setting CLImage downsample stack '") + mName + L"' base image to '" + toplevelImage->getName() + L"'");
	if (mCLImageStack.size() == 0)
	{
		mCLImageStack.push_back(std::move(toplevelImage));
		return;
	}
	mCLImageStack[0] = std::move(toplevelImage);
}

CLImage& CLImageDownsampleStack::GetCLImageAtDepthLevel(int depthLevel)
{
	return *mCLImageStack[depthLevel];
}

const CLImage& CLImageDownsampleStack::GetCLImageAtDepthLevel(int depthLevel) const
{
	return *mCLImageStack[depthLevel];
}

void CLImageDownsampleStack::UpdateDownsampledCLImages(cl_int2 minSizes, int maxDepthLevel)
{
	Util::PrintLogLineDebug(wstring(L"[*] Updating/creating downsampled images in DSStack '") + mName + L"'");

	int lastUsedDepthLevel = 0;
	// Go through all levels
	for (int i = 1; i < maxDepthLevel; i++)
	{
		cl_int2 dsSize = CalcSizeAtDepthLevel(mCLImageStack[0]->GetSize(), i);
		if (dsSize.x < minSizes.x || dsSize.y < minSizes.y)
			break;

		if (mCLImageStack.size() <= i || mCLImageStack[i]->GetSizeX() != dsSize.x || mCLImageStack[i]->GetSizeY() != dsSize.y)
		{
			CLImage* newCLImage =
				new CLImage(mDownsampler.GetContext(),
					L"Temp", // Name will be updated later
					dsSize.x, dsSize.y,	0,
					CL_MEM_READ_WRITE,
					mCLImageStack[0]->getFormat().image_channel_order,
					mCLImageStack[0]->getFormat().image_channel_data_type,
					nullptr);

			if (mCLImageStack.size() <= i)
			{
				// Image at the requested depth was missing
				mCLImageStack.push_back(unique_ptr<CLImage>(newCLImage));
			}
			else
			{
				// Different size was used before so we delete and replace the old one (by resetting the smart pointer)
				mCLImageStack[i].reset(newCLImage);
			}
		}
		// Set name
		mCLImageStack[i]->setName(mCLImageStack[0]->getName() + L" Dowsample[" + to_wstring(i) + L"]");
		mDownsampler.DownsampleImageHalfSize(*mCLImageStack[i - 1], *mCLImageStack[i]);
		lastUsedDepthLevel = i;
	}

	// Release unnecessary levels
	if (lastUsedDepthLevel >= mCLImageStack.size())
	{
		mCLImageStack.resize(lastUsedDepthLevel + 1);
	}
}

size_t CLImageDownsampleStack::GetNumDepthLevels() const
{
	return (size_t)mCLImageStack.size();
}
