#include "stdafx.h"
#include "CLUtility.h"
#include "CLHelper.h"
#include "CLImageDownsampler.h"
#include "CLImageDownsampleStack.h"

using namespace std;
using namespace sf;

const cl_int2 MainWindowSize{ 1024, 768 };

RenderWindow mainWindow;

Image imgDownsized;
Texture texDownsized;
Sprite sprDownsized;

cl_context clContext;
cl_command_queue clCommandQueue;
cl_device_id clDevice;

int main()
{
	// Create SFML window
	mainWindow.create(sf::VideoMode(MainWindowSize.x, MainWindowSize.y), "MacroCL");
	mainWindow.setVerticalSyncEnabled(true);

	CLHelp::InitOpenCL(clContext, clCommandQueue, clDevice);

	CLImageDownsampler downsampler(clContext, clDevice, clCommandQueue);
	CLImageDownsampleStack downsamplerStack(L"File downsample stack");

	unique_ptr<CLImage> fullCLImage = CLHelp::CLImageFromFile(clContext, L"C:\\Users\\Castor\\Pictures\\1.jpg", CL_MEM_READ_WRITE);
	//unique_ptr<CLImage> hslfCLImage(new CLImage(clContext, L"Downsampled image (half size)", fullCLImage->GetSizeX() / 2, fullCLImage->GetSizeY() / 2, 0, CL_MEM_READ_WRITE, CL_RGBA, CL_UNORM_INT8));

	//downsampler.DownsampleImageHalfSize(*fullCLImage, *hslfCLImage);
	downsamplerStack.SetBaseCLImage(std::move(fullCLImage));
	downsamplerStack.UpdateDownsampledCLImages(downsampler, cl_int2{ 64, 64 }, 10);

	CLImage& hslfCLImage = downsamplerStack.GetCLImageAtDepthLevel(3);

	std::vector<int> pixelData;
	CLHelp::ReadCLImageData(clCommandQueue, hslfCLImage, pixelData);

	imgDownsized.create((unsigned int)hslfCLImage.GetSizeX(), (unsigned int)hslfCLImage.GetSizeY(), (sf::Uint8*)pixelData.data());
	texDownsized.create(imgDownsized.getSize().x, imgDownsized.getSize().y);
	texDownsized.update(imgDownsized, 0, 0);
	sprDownsized.setTexture(texDownsized);

	while (mainWindow.isOpen())
	{
		sf::Event event;
		while (mainWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				mainWindow.close();
			}

			mainWindow.draw(sprDownsized);
			mainWindow.display();
		}
	}

	return 0;
}

