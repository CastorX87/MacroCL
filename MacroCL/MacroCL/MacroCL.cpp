#include "stdafx.h"
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
	Image imgDownsized;
	Texture texDownsized;
	Sprite sprDownsized;
	std::vector<int> pixelData;
	CLHelp::ReadCLImageData(clCommandQueue, clImage, pixelData);
	imgDownsized.create((unsigned int)clImage.GetSizeX(), (unsigned int)clImage.GetSizeY(), (sf::Uint8*)pixelData.data());
	texDownsized.create(imgDownsized.getSize().x, imgDownsized.getSize().y);
	texDownsized.update(imgDownsized, 0, 0);
	sprDownsized.setTexture(texDownsized);
	sprDownsized.setScale(1, 1);
	mainWindow.draw(sprDownsized);
}

int main()
{
	// Create SFML window
	mainWindow.create(sf::VideoMode(MainWindowSize.x, MainWindowSize.y), "MacroCL");
	mainWindow.setVerticalSyncEnabled(true);

	CLHelp::InitOpenCL(clContext, clCommandQueue, clDevice);

	unique_ptr<CLImage> fullCLImage1 = CLHelp::CLImageFromFile(clContext, L"C:\\Users\\Castor\\Pictures\\1b.jpg", CL_MEM_READ_WRITE);
	unique_ptr<CLImage> fullCLImage2 = CLHelp::CLImageFromFile(clContext, L"C:\\Users\\Castor\\Pictures\\2b.jpg", CL_MEM_READ_WRITE);
	CLImageComparator comparator(L"Comparator", clContext, clDevice, clCommandQueue, fullCLImage1->GetSize(), cl_int2{ 32, 32 }, 10);
	float r = 0;
	float s = 1;
	int x = 0;
	int y = 0;
	bool controlPressed = false;
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
				x = event.mouseMove.x - comparator.GetCompareCLImageFullSize().GetSizeX() / 2;
				y = event.mouseMove.y - comparator.GetCompareCLImageFullSize().GetSizeY() / 2;
			}
			if (event.type == sf::Event::MouseWheelScrolled)
			{
				if(controlPressed)
					s += event.mouseWheelScroll.delta * 0.0025f;
				else
					r += event.mouseWheelScroll.delta * 0.25f;
			}
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Key::LControl)
				{
					controlPressed = true;
				}
			}
			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Key::LControl)
				{
					controlPressed = false;
				}
			}

			mainWindow.clear(sf::Color(100,100,100,255));

			MMAligmentData alignment;
			
			sf::Clock clk;
			float cmpResult;
			alignment.rotate = r;
			alignment.scale = s;
				alignment.translation.x = x;
				alignment.translation.y = y;
				cmpResult = comparator.CompareCLImages(alignment, *fullCLImage1, *fullCLImage2);
				Show(comparator.GetCompareCLImageFullSize());
				mainWindow.display();
			//Util::PrintLogLine(wstring(L"deltaTime=") + to_wstring(clk.getElapsedTime().asMicroseconds() / 1000.0f) + L"ms");
			Util::PrintLogLine(wstring(L"cmpResult=") + to_wstring(cmpResult));
			Util::PrintLogLine(wstring(L"alignment=") + alignment.ToString());
			//Util::PrintLogLine(wstring(L"depth=") + to_wstring(comparator.GetStack().GetNumDepthLevels()));
			//Util::PrintLogLine(wstring(L"minSize=") + to_wstring(comparator.GetCompareCLImageDeepest().GetSizeX()) + L"x" + to_wstring(comparator.GetCompareCLImageDeepest().GetSizeY()));
			Util::PrintLogLine(L"");

		}
	}

	return 0;
}

