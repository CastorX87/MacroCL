﻿#include "stdafx.h"
#include "CLUtility.h"
#include "CLHelper.h"
#include "CLImageDownsampler.h"
#include "CLImageDownsampleStack.h"
#include "CLImageComparator.h"

using namespace std;
using namespace sf;

const cl_int2 MainWindowSize{ 1280, 900 };

RenderWindow mainWindow;

cl_context clContext;
cl_command_queue clCommandQueue;
cl_device_id clDevice;

void Show(CLImage& clImage)
{
	Image img;
	Texture tex;
	Sprite sprite;
	std::vector<int> pixels;
	CLHelp::ReadCLImageData(clCommandQueue, clImage, pixels);
	img.create((unsigned int)clImage.GetSizeX(), (unsigned int)clImage.GetSizeY(), (sf::Uint8*)pixels.data());
	tex.create(img.getSize().x, img.getSize().y);
	tex.update(img, 0, 0);
	tex.setSmooth(true);
	sprite.setTexture(tex);
	float scale = (float)MainWindowSize.y / clImage.GetSizeY();
	sprite.setScale(scale, scale);
	mainWindow.draw(sprite);
}


MMAligmentData BestAlignmentDirRotate(CLImageComparator& comparator, CLImage& imageBase, CLImage& imageToAlign, MMAligmentData& startAlignment, float startScore, float& score)
{
	MMAligmentData testAlignment = startAlignment;
	MMAligmentData bestAlignment = startAlignment;
	float bestScore = startScore;

	float TRY_VECTOR[]{  -8, -4, -2, -1.5, -0.75, -0.35, 0.35, 0.75, 1.5, 2, 4, 8};
	//float TRY_VECTOR[]{ -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6 };
	for (int ix = 0; ix < (sizeof(TRY_VECTOR) / sizeof(float)); ix++)
	{
		float ds = TRY_VECTOR[ix];
		testAlignment.rotate = startAlignment.rotate + ds;

		float v = comparator.CompareCLImages(testAlignment, imageBase, imageToAlign);
		if (v < bestScore)
		{
			bestScore = v;
			bestAlignment = testAlignment;
		}
	}
	score = bestScore;
	return bestAlignment;
}

MMAligmentData BestAlignmentDirScale(CLImageComparator& comparator, CLImage& imageBase, CLImage& imageToAlign, MMAligmentData& startAlignment, float startScore, float& score)
{
	MMAligmentData testAlignment = startAlignment;
	MMAligmentData bestAlignment = startAlignment;
	float bestScore = startScore;
	float TRY_VECTOR[]{ -0.1, -0.05, -0.025, -0.0125,-0.0065, -0.003, 0.003, 0.0065, 0.0125, 0.025, 0.05, 0.1 };
	//float TRY_VECTOR[]{ -0.05, -0.04, -0.03, -0.02, -0.01, 0.01, 0.02, 0.03, 0.04, 0.05 };
	for (int ix = 0; ix < (sizeof(TRY_VECTOR) / sizeof(float)); ix++)
	{
		float ds = TRY_VECTOR[ix];

		testAlignment.scale = startAlignment.scale + ds;

		float v = comparator.CompareCLImages(testAlignment, imageBase, imageToAlign);
		if (v < bestScore)
		{
			bestScore = v;
			bestAlignment = testAlignment;
		}
	}
	score = bestScore;
	return bestAlignment;
}

MMAligmentData BestAlignmentDirTranslate(CLImageComparator& comparator, CLImage& imageBase, CLImage& imageToAlign, MMAligmentData& startAlignment, float startScore, float& score)
{
	MMAligmentData testAlignment = startAlignment;
	MMAligmentData bestAlignment = startAlignment;
	float bestScore = startScore;
	int TRY_VECTOR[]{ -16, -8, -4, -2, -1, 1, 2, 4, 8, 16 };
	//int TRY_VECTOR[]{ -5, -4, -3, -2, -1, 1, 2, 3, 4, 5 };
	for (int ix = 0; ix < (sizeof(TRY_VECTOR) / sizeof(int)); ix++)
	{
		int dx = TRY_VECTOR[ix];
		for (int iy = 0; iy < (sizeof(TRY_VECTOR) / sizeof(int)); iy++)
		{
			int dy = TRY_VECTOR[iy];
			if (sqrtf((float)dx * dx + (float)dy * dy) <= fabs((float)TRY_VECTOR[0]))
			{
				testAlignment.translation = startAlignment.translation + sf::Vector2f(dx, dy);
				float v = comparator.CompareCLImages(testAlignment, imageBase, imageToAlign);
				if (v < bestScore)
				{
					bestScore = v;
					bestAlignment = testAlignment;
				}
			}
		}
	}
	score = bestScore;
	return bestAlignment;
}

MMAligmentData FindBestAlignment(CLImageComparator& comparator, CLImageDownsampleStack& imgBaseStc, CLImageDownsampleStack& imgAlignStc, MMAligmentData& startAlignment)
{
	int dsLevel = min(comparator.GetStack().GetNumDepthLevels(), min(imgBaseStc.GetNumDepthLevels(), imgAlignStc.GetNumDepthLevels())) - 1;
	MMAligmentData currAlignment = startAlignment;
	float bestScore = 255;//= comparator.CompareCLImages(startAlignment, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel));
	std::wcout << L"Starting score " << bestScore << std::endl;
	float lastScore = bestScore;
	while (true)
	{
		currAlignment = BestAlignmentDirTranslate(comparator, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), currAlignment, bestScore, bestScore);
		currAlignment = BestAlignmentDirScale(comparator, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), currAlignment, bestScore, bestScore);
		currAlignment = BestAlignmentDirRotate(comparator, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), currAlignment, bestScore, bestScore);
		//std::cout << "DS Level:" << dsLevel << " -> X:" << currAlignment.translation.x << " Y:" << currAlignment.translation.y << " S:" << currAlignment.scale << " R:" << currAlignment.rotate << "   Score:" << bestScore << std::endl;
		comparator.CompareCLImages(currAlignment, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel));
		if (lastScore == bestScore)
		{
			if (dsLevel == 0)
				break;
			dsLevel--;
			bestScore = 255;
			currAlignment.translation.x *= 2;
			currAlignment.translation.y *= 2;
			std::wcout << L"Local minimal score reched on level " << dsLevel << std::endl;
		}
		lastScore = bestScore;

		/*mainWindow.clear(sf::Color(100, 100, 100, 255));
		Show(comparator.GetCompareCLImageAtDepth(dsLevel));
		mainWindow.display();*/
	}

	mainWindow.clear(sf::Color(100, 100, 100, 255));
	comparator.CompareCLImages(currAlignment, imgBaseStc.GetCLImageAtDepthLevel(0), imgAlignStc.GetCLImageAtDepthLevel(0));
	Show(comparator.GetCompareCLImageAtDepth(0));
	mainWindow.display();

	return currAlignment;
}

int main()
{
	// Create SFML window
	mainWindow.create(sf::VideoMode(MainWindowSize.x, MainWindowSize.y), "MacroCL");
	mainWindow.setVerticalSyncEnabled(true);

	CLHelp::InitOpenCL(clContext, clCommandQueue, clDevice);

	unique_ptr<CLImage> fullCLImageA = CLHelp::CLImageFromFile(clContext, L"C:\\Users\\Castor\\Pictures\\A1.jpg", CL_MEM_READ_WRITE);
	unique_ptr<CLImage> fullCLImageB = CLHelp::CLImageFromFile(clContext, L"C:\\Users\\Castor\\Pictures\\A2.jpg", CL_MEM_READ_WRITE);

	CLImageDownsampleStack stackCLImageA(L"ImgA DS", clContext, clDevice, clCommandQueue);
	CLImageDownsampleStack stackCLImageB(L"ImgB DS", clContext, clDevice, clCommandQueue);

	stackCLImageA.SetBaseCLImage(std::move(fullCLImageA));
	stackCLImageB.SetBaseCLImage(std::move(fullCLImageB));

	stackCLImageA.UpdateDownsampledCLImages(cl_int2{ 32, 32 }, 10);
	stackCLImageB.UpdateDownsampledCLImages(cl_int2{ 32, 32 }, 10);

	CLImageComparator comparator(L"Comparator", clContext, clDevice, clCommandQueue, stackCLImageA.GetCLImageAtDepthLevel(0).GetSize(), cl_int2{ 32, 32 }, 10);
	float r = 0;
	float s = 1;
	int x = 0;
	int y = 0;
	bool controlPressed = false;
	MMAligmentData alignment;
	while (mainWindow.isOpen())
	{
		sf::Event event;
		while (mainWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				mainWindow.close();
			}
			if (event.type == sf::Event::MouseWheelMoved)
			{
				float score = comparator.CompareCLImages(alignment, stackCLImageA.GetCLImageAtDepthLevel(0), stackCLImageB.GetCLImageAtDepthLevel(0));
				Util::PrintLogLine(wstring(L"score=") + to_wstring(score));
				Util::PrintLogLine(wstring(L"alignment=") + alignment.ToString());
				mainWindow.clear(sf::Color(100, 100, 100, 255));
				Show(comparator.GetCompareCLImageAtDepth(0));
				mainWindow.display();
			}
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Key::LControl)
				{
					controlPressed = true;
				}
				if (event.key.code == sf::Keyboard::Key::Space)
				{
					alignment.rotate = 0;
					alignment.scale = 1;
					alignment.translation.x = 0;
					alignment.translation.y = 0;
					alignment = FindBestAlignment(comparator, stackCLImageA, stackCLImageB, alignment);
					Util::PrintLogLine(wstring(L"alignment=") + alignment.ToString());
					Util::PrintLogLine(L"");
				}
			}
			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Key::LControl)
				{
					controlPressed = false;
				}
			}
		}
	}

	return 0;
}

