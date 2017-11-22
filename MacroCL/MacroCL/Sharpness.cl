__constant sampler_t downsizeInputImageSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_LINEAR;

float4 GetDataF4Img(int2 pos, image2d_t image)
{
	return read_imagef(image, downsizeInputImageSampler, pos);
}

float4 GetDataF4ImgNORM(float2 pos, image2d_t image)
{
	return read_imagef(image, downsizeInputImageSampler, pos);
}

void SetDataF4Img(float4 data, int2 pos, image2d_t image)
{
	write_imagef(image, pos, data);
}

float4 AvgOfFour(int2 pos, image2d_t img)
{
	float4 sum = (float4)(0, 0, 0, 0);
	sum += GetDataF4Img(pos + (int2)(0, 0), img);
	sum += GetDataF4Img(pos + (int2)(0, 1), img);
	sum += GetDataF4Img(pos + (int2)(1, 1), img);
	sum += GetDataF4Img(pos + (int2)(1, 0), img);
	if (sum.w <= 0)
		return (float4)(0, 0, 0, 0);
	return sum / sum.w;
}

__kernel void SharpnessMain(
	__read_only		image2d_t		inputImage,				// Input image to be downsized
	__write_only	image2d_t		outputImage,			// Input image to be downsized
								int2				inputImageSize,		// Size (width and height) of the input image THIS MUST BE AN EVEN NUMBER!
								int					range)
{
	int2 gPos = (int2)(get_global_id(0), get_global_id(1));
	if (gPos.x >= inputImageSize.x || gPos.y >= inputImageSize.y)
		return;

	float2 texCoord = (float2)(gPos.x, gPos.y) / (float2)(inputImageSize.x, inputImageSize.y);
	float4 center = GetDataF4Img(gPos, inputImage);
	float4 others = (float4)(0.01, 0.01, 0.01, 0);
	float increaseX = 1.0f / inputImageSize.x;
	float increaseY = 1.0f / inputImageSize.y;
	int n = 0;
	float rangeF = (float)range / max(inputImageSize.x, inputImageSize.y);
	for (float dx = -rangeF; dx <= rangeF; dx += increaseX)
	{
		for (float dy = -rangeF; dy <= rangeF; dy += increaseY)
		{
			if (sqrt((float)(dx *dx + dy * dy)) <= rangeF)
			{
				others += GetDataF4ImgNORM(texCoord + (float2)(dx, dy), inputImage);
				n++;
			}
		}
	}
	others = others / (float)n;
	float4 sharpness = fabs(center - others) * 10;
	sharpness.x = fmax(sharpness.x, sharpness.y);
	sharpness.x = fmax(sharpness.x, sharpness.z);
	sharpness = (float4)(sharpness.x, sharpness.x, sharpness.x, 1);
	
	SetDataF4Img(sharpness, gPos, outputImage);
}
