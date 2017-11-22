#include "stdafx.h"
#include "CLUtility.h"
#include "CLHelper.h"
#include "CLImageDownsampler.h"
#include "CLImageDownsampleStack.h"
#include "CLImageComparator.h"
#include "CLImageSharpnessCalc.h"

using namespace std;
using namespace sf;

const cl_int2 MainWindowSize{ 1280, 900 };

RenderWindow mainWindow;

vector<wstring> files{
	L"C:\\Resized\\DSC06166.jpeg",
	L"C:\\Resized\\DSC06167.jpeg",
	L"C:\\Resized\\DSC06168.jpeg",
	L"C:\\Resized\\DSC06169.jpeg",
	L"C:\\Resized\\DSC06170.jpeg",
	L"C:\\Resized\\DSC06171.jpeg",
	L"C:\\Resized\\DSC06172.jpeg",
	L"C:\\Resized\\DSC06173.jpeg",
	L"C:\\Resized\\DSC06174.jpeg",
	L"C:\\Resized\\DSC06175.jpeg",
	L"C:\\Resized\\DSC06176.jpeg",
	L"C:\\Resized\\DSC06177.jpeg",
	L"C:\\Resized\\DSC06178.jpeg",
	L"C:\\Resized\\DSC06179.jpeg",
	L"C:\\Resized\\DSC06180.jpeg",
	L"C:\\Resized\\DSC06181.jpeg",
	L"C:\\Resized\\DSC06182.jpeg",
	L"C:\\Resized\\DSC06183.jpeg",
	L"C:\\Resized\\DSC06184.jpeg",
	L"C:\\Resized\\DSC06185.jpeg",
	L"C:\\Resized\\DSC06186.jpeg",
	L"C:\\Resized\\DSC06187.jpeg",
	L"C:\\Resized\\DSC06188.jpeg",
	L"C:\\Resized\\DSC06189.jpeg",
	L"C:\\Resized\\DSC06190.jpeg",
	L"C:\\Resized\\DSC06191.jpeg",
	L"C:\\Resized\\DSC06192.jpeg",
	L"C:\\Resized\\DSC06193.jpeg",
	L"C:\\Resized\\DSC06194.jpeg",
	L"C:\\Resized\\DSC06195.jpeg",
	L"C:\\Resized\\DSC06196.jpeg",
	L"C:\\Resized\\DSC06197.jpeg",
	L"C:\\Resized\\DSC06198.jpeg",
	L"C:\\Resized\\DSC06199.jpeg",
	L"C:\\Resized\\DSC06200.jpeg",
	L"C:\\Resized\\DSC06201.jpeg",
	L"C:\\Resized\\DSC06202.jpeg",
	L"C:\\Resized\\DSC06203.jpeg"
};

vector<unique_ptr<SFImage>> sfImagesAll;

cl_context clContext;
cl_command_queue clCommandQueue;
cl_device_id clDevice;

void Show(CLImage& clImage)
{
	Util::PrintLogLine(wstring(L"Showing image: '") + clImage.getName() + L"' " + to_wstring(clImage.GetSizeX()) + L"x" + to_wstring(clImage.GetSizeY()));
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
	float scale = (float)MainWindowSize.y / clImage.GetSizeY() * 0.85f;
	sprite.setScale(scale, scale);
	mainWindow.draw(sprite);
}

MMAligmentData BestAlignmentDirRotate(CLImageComparator& comparator, CLImage& imageBase, CLImage& imageToAlign, MMAligmentData& startAlignment, float startScore, float& score, int dsLevel)
{
	MMAligmentData testAlignment = startAlignment;
	MMAligmentData bestAlignment = startAlignment;
	float bestScore = startScore;

	for (float r = -4; r <= 4; r += 0.5f)
	{
		testAlignment.rotate = startAlignment.rotate + r;
		float v = comparator.CompareCLImages(testAlignment, imageBase, imageToAlign, dsLevel);
		if (v < bestScore)
		{
			bestScore = v;
			bestAlignment = testAlignment;
		}
	}

	score = bestScore;
	return bestAlignment;
}

MMAligmentData BestAlignmentDirScale(CLImageComparator& comparator, CLImage& imageBase, CLImage& imageToAlign, MMAligmentData& startAlignment, float startScore, float& score, int dsLevel)
{
	MMAligmentData testAlignment = startAlignment;
	MMAligmentData bestAlignment = startAlignment;
	float bestScore = startScore;
	float TRY_VECTOR[]{ -0.05, -0.025, -0.0125,-0.0065, -0.003, 0.003, 0.0065, 0.0125, 0.025, 0.05 };
	for (int ix = 0; ix < (sizeof(TRY_VECTOR) / sizeof(float)); ix++)
	{
		float ds = TRY_VECTOR[ix];

		testAlignment.scale = startAlignment.scale + ds;

		float v = comparator.CompareCLImages(testAlignment, imageBase, imageToAlign, dsLevel);
		if (v < bestScore)
		{
			bestScore = v;
			bestAlignment = testAlignment;
		}
	}
	score = bestScore;
	return bestAlignment;
}

MMAligmentData BestAlignmentDirTranslate(CLImageComparator& comparator, CLImage& imageBase, CLImage& imageToAlign, MMAligmentData& startAlignment, float startScore, float& score, int dsLevel)
{
	MMAligmentData testAlignment = startAlignment;
	MMAligmentData bestAlignment = startAlignment;
	float bestScore = startScore;
	int TRY_VECTOR[]{ -5, -2, -1, 1, 2, 5 };
	for (int ix = 0; ix < (sizeof(TRY_VECTOR) / sizeof(int)); ix++)
	{
		int dx = TRY_VECTOR[ix];
		for (int iy = 0; iy < (sizeof(TRY_VECTOR) / sizeof(int)); iy++)
		{
			int dy = TRY_VECTOR[iy];
			if (sqrtf((float)dx * dx + (float)dy * dy) <= fabs((float)TRY_VECTOR[0]))
			{
				testAlignment.translation = startAlignment.translation + sf::Vector2f(dx, dy);
				float v = comparator.CompareCLImages(testAlignment, imageBase, imageToAlign, dsLevel);

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

MMAligmentData FindBestAlignment(CLImageComparator& comparator, CLImageDownsampleStack& imgBaseStc, CLImageDownsampleStack& imgAlignStc, MMAligmentData& startAlignment, float& score)
{
	int dsLevel = min(comparator.GetStack().GetNumDepthLevels(), min(imgBaseStc.GetNumDepthLevels(), imgAlignStc.GetNumDepthLevels())) - 1;
	MMAligmentData currAlignment = startAlignment;
	float bestScore = 255;//= comparator.CompareCLImages(startAlignment, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel));
	std::wcout << L"Starting score " << bestScore << std::endl;
	float lastScore = bestScore;
	while (true)
	{
		currAlignment = BestAlignmentDirTranslate(comparator, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), currAlignment, bestScore, bestScore, dsLevel);
		currAlignment = BestAlignmentDirScale(comparator, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), currAlignment, bestScore, bestScore, dsLevel);
		currAlignment = BestAlignmentDirRotate(comparator, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), currAlignment, bestScore, bestScore, dsLevel);
		comparator.CompareCLImages(currAlignment, imgBaseStc.GetCLImageAtDepthLevel(dsLevel), imgAlignStc.GetCLImageAtDepthLevel(dsLevel), dsLevel);

		Show(comparator.GetCompareCLImageAtDepth(dsLevel));
		mainWindow.display();

		if (lastScore == bestScore)
		{
			if (dsLevel == 0)
			{
				score = bestScore;
				break;
			}
			dsLevel--;
			bestScore = 255;
			currAlignment.translation.x *= 2;
			currAlignment.translation.y *= 2;
			std::wcout << L"Local minimal score reched on level " << dsLevel << std::endl;
		}
		lastScore = bestScore;

	}

	mainWindow.clear(sf::Color(100, 100, 100, 255));
	comparator.CompareCLImages(currAlignment, imgBaseStc.GetCLImageAtDepthLevel(0), imgAlignStc.GetCLImageAtDepthLevel(0), dsLevel);
	Show(comparator.GetCompareCLImageAtDepth(0));
	mainWindow.display();

	return currAlignment;
}

std::vector<sf::Color> DEBUG_PIXELS_AT_POSITION;
void GetPixelsOfImages(int x, int y, std::vector<std::unique_ptr<SFImage>>& images, std::vector<sf::Color>& pixels)
{
	if (pixels.size() != images.size())
		pixels.resize(images.size());
	
	
	int n = 0;
	for (auto& imgPtr : images)
	{
		x = fmax(fmin(imgPtr->getSize().x - 1, x), 0);
		y = fmax(fmin(imgPtr->getSize().y - 1, y), 0);

		sf::Color color = imgPtr->getPixel(x, y);
		pixels[n] = color;
		n++;
	}
}

int main()
{
	// Create SFML window
	mainWindow.create(sf::VideoMode(MainWindowSize.x, MainWindowSize.y), "MacroCL");
	mainWindow.setVerticalSyncEnabled(true);

	CLHelp::InitOpenCL(clContext, clCommandQueue, clDevice);

	unique_ptr<CLImage> fullA = CLHelp::CLImageFromFile(clContext, files[0], CL_MEM_READ_WRITE);
	unique_ptr<CLImage> fullB = CLHelp::CLImageFromFile(clContext, files[1], CL_MEM_READ_WRITE);

	unique_ptr<CLImage> sharpnessA = unique_ptr<CLImage>(new CLImage(clContext, L"Sharpness map of ImageA", fullA->GetSizeX(), fullA->GetSizeY(), 0, CL_MEM_READ_WRITE, fullA->getFormat().image_channel_order, fullA->getFormat().image_channel_data_type, nullptr));
	CLImageSharpnessCalc sharpnessCalc(L"Sharpness calculator", clContext, clDevice, clCommandQueue);


	CLImageDownsampleStack stackCLImageA(L"ImgA DS", clContext, clDevice, clCommandQueue);
	CLImageDownsampleStack stackCLImageB(L"ImgB DS", clContext, clDevice, clCommandQueue);

	stackCLImageA.SetBaseCLImage(std::move(fullA));
	stackCLImageB.SetBaseCLImage(std::move(fullB));

	stackCLImageA.UpdateDownsampledCLImages(cl_int2{ 16, 16 }, 10, 0);
	stackCLImageB.UpdateDownsampledCLImages(cl_int2{ 16, 16 }, 10, 0);

	CLImageComparator comparator(L"Comparator", clContext, clDevice, clCommandQueue, stackCLImageA.GetCLImageAtDepthLevel(0).GetSize(), cl_int2{ 16, 16 }, 10);

	// ---------------- DEBUG ---------------- //
	for (auto& file : files)
	{
		Util::PrintLogLine(wstring(L"Loading image ") + file);
		sfImagesAll.push_back(unique_ptr<SFImage>(new SFImage()));
		sfImagesAll.back()->loadFromFile(Util::WStrToStr(file));
	}
	// --------------------------------------- //


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
			if (event.type == sf::Event::MouseMoved)
			{
				x = event.mouseMove.x;
				y = event.mouseMove.y;

				// ---------------- DEBUG ---------------- //
				GetPixelsOfImages(x, y, sfImagesAll, DEBUG_PIXELS_AT_POSITION);
				vector<sf::Vertex> lineVertices;
				for(int i = 0; i < DEBUG_PIXELS_AT_POSITION.size() - 1; i++)
				{
					auto& pixValueA = DEBUG_PIXELS_AT_POSITION[i];
					auto& pixValueB = DEBUG_PIXELS_AT_POSITION[i + 1];
					int a = (pixValueA.r + pixValueA.g + pixValueA.b) / 3;
					int b = (pixValueB.r + pixValueB.g + pixValueB.b) / 3;
					lineVertices.push_back(sf::Vertex(sf::Vector2f(i * 10, 256 - (float)a)));
					lineVertices.push_back(sf::Vertex(sf::Vector2f((i + 1) * 10, 256 - (float)b)));
				}
				mainWindow.clear(sf::Color(100, 100, 100, 255));
				Show(comparator.GetCompareCLImageAtDepth(0));
				mainWindow.draw(lineVertices.data(), lineVertices.size(), sf::Lines);
				mainWindow.display();
				// --------------------------------------- //
			}
			if (event.type == sf::Event::MouseWheelMoved)
			{
				float score = comparator.CompareCLImages(alignment, stackCLImageA.GetCLImageAtDepthLevel(0), stackCLImageB.GetCLImageAtDepthLevel(0), 0);
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
				if (event.key.code == sf::Keyboard::Key::S)
				{
					mainWindow.clear(sf::Color(100, 100, 100, 255));
					sharpnessCalc.CalcSharpness(stackCLImageA.GetCLImageAtDepthLevel(0), *sharpnessA, x / 10);

					Show(*sharpnessA);
					mainWindow.display();
				}
				if (event.key.code == sf::Keyboard::Key::Space)
				{
					alignment.rotate = 0;
					alignment.scale = 1;
					alignment.translation.x = 0;
					alignment.translation.y = 0;
					float score = 0;
					sf::Clock clock;
					alignment = FindBestAlignment(comparator, stackCLImageA, stackCLImageB, alignment, score);

					Util::PrintLogLine(wstring(L"time=") + to_wstring(clock.getElapsedTime().asMilliseconds()));
					Util::PrintLogLine(wstring(L"alignment=") + alignment.ToString() + L" score=" + to_wstring(score));
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

