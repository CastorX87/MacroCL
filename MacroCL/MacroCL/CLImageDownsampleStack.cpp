#include "stdafx.h"
#include "CLImageDownsampleStack.h"

using namespace std;

CLImageDownsampleStack::CLImageDownsampleStack(wstring name)
{
	Util::PrintLog(wstring(L"[+] Creating CLImage downsample stack '") + name + L"': ");
	mName = name;
	Util::PrintLogLine("OK");
}

CLImageDownsampleStack::~CLImageDownsampleStack()
{
	Util::PrintLogLine(std::wstring(L"[-] Releasing CLImage stack '") + mName + L"'");
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
	Util::PrintLogLine(wstring(L"[*] Setting CLImage downsample stack '") + mName + L"' base image to '" + toplevelImage->getName() + L"'");
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

void CLImageDownsampleStack::UpdateDownsampledCLImages(CLImageDownsampler& downsampler, cl_int2 minSizes, int maxDepthLevel)
{
	Util::PrintLogLine(wstring(L"[*] Updating/creating downsampled images in DSStack '") + mName + L"'");

	// Go through all levels
	for (int i = 1; i < maxDepthLevel; i++)
	{
		cl_int2 dsSize = CalcSizeAtDepthLevel(mCLImageStack[0]->GetSize(), i);
		if (dsSize.x < minSizes.x || dsSize.y < minSizes.y)
			break;

		if (mCLImageStack.size() <= i || mCLImageStack[i]->GetSizeX() != dsSize.x || mCLImageStack[i]->GetSizeY() != dsSize.y)
		{
			CLImage* newCLImage =
				new CLImage(downsampler.GetContext(),
					mCLImageStack[0]->getName() + L" downsample lvl " + to_wstring(i),
					dsSize.x, dsSize.y,
					0,
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
		downsampler.DownsampleImageHalfSize(*mCLImageStack[i - 1], *mCLImageStack[i]);
	}
}

size_t CLImageDownsampleStack::GetNumDepthLevels() const
{
	return (size_t)mCLImageStack.size();
}
