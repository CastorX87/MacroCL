#include "stdafx.h"
#include "CLUtility.h"
#include "CLHelper.h"
#include "CLImageDownsampler.h"
#include "CLImageDownsampleStack.h"
#include "CLImageComparator.h"
#include "CLImageSharpnessCalc.h"
#include "CLWeightCalculator.h"

using namespace std;
using namespace sf;

const cl_int2 MainWindowSize{ 1280, 900 };

RenderWindow mainWindow;

vector<wstring> filesA{
	L"C:\\Resized\\A (1).jpeg",
	L"C:\\Resized\\A (2).jpeg",
	L"C:\\Resized\\A (3).jpeg",
	L"C:\\Resized\\A (4).jpeg",
	L"C:\\Resized\\A (5).jpeg",
	L"C:\\Resized\\A (6).jpeg",
	L"C:\\Resized\\A (7).jpeg",
	L"C:\\Resized\\A (8).jpeg",
	L"C:\\Resized\\A (9).jpeg",
	L"C:\\Resized\\A (10).jpeg",
	L"C:\\Resized\\A (11).jpeg",
	L"C:\\Resized\\A (12).jpeg",
	L"C:\\Resized\\A (13).jpeg",
	L"C:\\Resized\\A (14).jpeg",
	L"C:\\Resized\\A (15).jpeg",
	L"C:\\Resized\\A (16).jpeg",
	L"C:\\Resized\\A (17).jpeg",
	L"C:\\Resized\\A (18).jpeg",
	L"C:\\Resized\\A (19).jpeg",
	L"C:\\Resized\\A (20).jpeg",
	L"C:\\Resized\\A (21).jpeg",
	L"C:\\Resized\\A (22).jpeg",
	L"C:\\Resized\\A (23).jpeg",
	L"C:\\Resized\\A (24).jpeg",
	L"C:\\Resized\\A (25).jpeg",
	L"C:\\Resized\\A (26).jpeg",
	L"C:\\Resized\\A (27).jpeg",
	L"C:\\Resized\\A (28).jpeg",
	L"C:\\Resized\\A (29).jpeg",
	L"C:\\Resized\\A (30).jpeg",
	L"C:\\Resized\\A (31).jpeg",
	L"C:\\Resized\\A (32).jpeg",
	L"C:\\Resized\\A (33).jpeg",
};

vector<wstring> filesB{
	L"C:\\Resized\\B (1).jpeg",
	L"C:\\Resized\\B (2).jpeg",
	L"C:\\Resized\\B (3).jpeg",
	L"C:\\Resized\\B (4).jpeg",
	L"C:\\Resized\\B (5).jpeg",
	L"C:\\Resized\\B (6).jpeg",
	L"C:\\Resized\\B (7).jpeg",
	L"C:\\Resized\\B (8).jpeg",
	L"C:\\Resized\\B (9).jpeg",
	L"C:\\Resized\\B (10).jpeg",
	L"C:\\Resized\\B (11).jpeg",
	L"C:\\Resized\\B (12).jpeg",
	L"C:\\Resized\\B (13).jpeg",
	L"C:\\Resized\\B (14).jpeg",
	L"C:\\Resized\\B (15).jpeg",
	L"C:\\Resized\\B (16).jpeg",
	L"C:\\Resized\\B (17).jpeg",
	L"C:\\Resized\\B (18).jpeg",
	L"C:\\Resized\\B (19).jpeg",
	L"C:\\Resized\\B (20).jpeg",
	L"C:\\Resized\\B (21).jpeg",
	L"C:\\Resized\\B (22).jpeg",
	L"C:\\Resized\\B (23).jpeg",
	L"C:\\Resized\\B (24).jpeg",
	L"C:\\Resized\\B (25).jpeg",
	L"C:\\Resized\\B (26).jpeg",
	L"C:\\Resized\\B (27).jpeg",
	L"C:\\Resized\\B (28).jpeg",
	L"C:\\Resized\\B (29).jpeg",
	L"C:\\Resized\\B (30).jpeg",
	L"C:\\Resized\\B (31).jpeg",
	L"C:\\Resized\\B (32).jpeg",
	L"C:\\Resized\\B (33).jpeg",
	L"C:\\Resized\\B (34).jpeg",
	L"C:\\Resized\\B (35).jpeg",
	L"C:\\Resized\\B (36).jpeg",
	L"C:\\Resized\\B (37).jpeg",
	L"C:\\Resized\\B (38).jpeg"
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
std::vector<sf::Color> DEBUG_CONTRASTS_AT_POSITION;

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

int CalcLum(sf::Color color)
{
	return (int)(0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b);
}

void GetContastsOfImages(int x, int y, std::vector<std::unique_ptr<SFImage>>& images, std::vector<sf::Color>& pixels)
{
	if (pixels.size() != images.size())
		pixels.resize(images.size());

	int n = 0;
	for (auto& imgPtr : images)
	{
		x = fmax(fmin(imgPtr->getSize().x - 1, x), 0);
		y = fmax(fmin(imgPtr->getSize().y - 1, y), 0);

		int centerLum = CalcLum(imgPtr->getPixel(x, y));
		int totalLum = 0;
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++)
			{
				if (dx == 0 && dy == 0)
					continue;
				int lX = fmax(fmin(imgPtr->getSize().x - 1, x + dx), 0);
				int lY = fmax(fmin(imgPtr->getSize().y - 1, y + dy), 0);
				totalLum += CalcLum(imgPtr->getPixel(lX, lY));
			}
		}
		int diff = (int)(fabsf(centerLum - totalLum / 8.0f));
		pixels[n] = sf::Color(diff, diff, diff, 255);
		n++;
	}
}

int main()
{
	// Create SFML window
	mainWindow.create(sf::VideoMode(MainWindowSize.x, MainWindowSize.y), "MacroCL");
	mainWindow.setVerticalSyncEnabled(true);

	CLHelp::InitOpenCL(clContext, clCommandQueue, clDevice);

	auto& files = filesB;

	unique_ptr<CLImage> fullA = CLHelp::CLImageFromFile(clContext, files[0], CL_MEM_READ_WRITE);
	unique_ptr<CLImage> fullB = CLHelp::CLImageFromFile(clContext, files[1], CL_MEM_READ_WRITE);

	unique_ptr<CLImage> sharpnessA = unique_ptr<CLImage>(new CLImage(clContext, L"Sharpness map of ImageA", fullA->GetSizeX(), fullA->GetSizeY(), 0, CL_MEM_READ_WRITE, fullA->getFormat().image_channel_order, fullA->getFormat().image_channel_data_type, nullptr));
	unique_ptr<CLImage> tmpFinalImg = unique_ptr<CLImage>(new CLImage(clContext, L"TempSharpImage", fullA->GetSizeX(), fullA->GetSizeY(), 0, CL_MEM_READ_WRITE, fullA->getFormat().image_channel_order, fullA->getFormat().image_channel_data_type, nullptr));
	CLImageSharpnessCalc sharpnessCalc(L"Sharpness calculator", clContext, clDevice, clCommandQueue);

	CLWeightCalculator weightCalc(L"Weight calculator", clContext, clDevice, clCommandQueue, files.size());

	CLImageDownsampleStack stackCLImageA(L"ImgA DS", clContext, clDevice, clCommandQueue);
	CLImageDownsampleStack stackCLImageB(L"ImgB DS", clContext, clDevice, clCommandQueue);

	stackCLImageA.SetBaseCLImage(std::move(fullA));
	stackCLImageB.SetBaseCLImage(std::move(fullB));

	stackCLImageA.UpdateDownsampledCLImages(cl_int2{ 16, 16 }, 10, 0);
	stackCLImageB.UpdateDownsampledCLImages(cl_int2{ 16, 16 }, 10, 0);

	CLImageComparator comparator(L"Comparator", clContext, clDevice, clCommandQueue, stackCLImageA.GetCLImageAtDepthLevel(0).GetSize(), cl_int2{ 16, 16 }, 10);

	vector<unique_ptr<CLImage>> allClImages;
	vector<MMAligmentData> allAlignments;
	vector<float> allWeights;
	// ---------------- DEBUG ---------------- //
	for (auto& file : files)
	{
		Util::PrintLogLine(wstring(L"Loading image ") + file);
		sfImagesAll.push_back(unique_ptr<SFImage>(new SFImage()));
		sfImagesAll.back()->loadFromFile(Util::WStrToStr(file));

		allClImages.push_back(std::move(CLHelp::CLImageFromFile(clContext, file, CL_MEM_READ_ONLY)));
		allAlignments.push_back(MMAligmentData()); // Use default alignemnt for the tests!
		allWeights.push_back(0.0f);
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
					lineVertices.push_back(sf::Vertex(sf::Vector2f(i * 10, 256 - (float)a), sf::Color(200, 250, 250, 255)));
					lineVertices.push_back(sf::Vertex(sf::Vector2f((i + 1) * 10, 256 - (float)b), sf::Color(200, 250, 250, 255)));
				}

				GetContastsOfImages(x, y, sfImagesAll, DEBUG_CONTRASTS_AT_POSITION);
				vector<sf::Vertex> lineVertices2;
				for (int i = 0; i < DEBUG_CONTRASTS_AT_POSITION.size() - 1; i++)
				{
					auto& pixValueA = DEBUG_CONTRASTS_AT_POSITION[i];
					auto& pixValueB = DEBUG_CONTRASTS_AT_POSITION[i + 1];
					int a = (pixValueA.r + pixValueA.g + pixValueA.b) / 3;
					int b = (pixValueB.r + pixValueB.g + pixValueB.b) / 3;
					lineVertices2.push_back(sf::Vertex(sf::Vector2f(i * 10, 256 - (float)a), sf::Color(200, 150, 100, 255)));
					lineVertices2.push_back(sf::Vertex(sf::Vector2f((i + 1) * 10, 256 - (float)b), sf::Color(200, 150, 100, 255)));
				}
				mainWindow.clear(sf::Color(100, 100, 100, 255));
				Show(*tmpFinalImg);
				mainWindow.draw(lineVertices.data(), lineVertices.size(), sf::Lines);
				mainWindow.draw(lineVertices2.data(), lineVertices2.size(), sf::Lines);
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
				if (event.key.code == sf::Keyboard::Key::W)
				{
					mainWindow.clear(sf::Color(255, 50, 50, 255));
					weightCalc.CalcWeights(allClImages, allAlignments, allWeights, *tmpFinalImg);
					Show(*tmpFinalImg);
					mainWindow.display();
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

