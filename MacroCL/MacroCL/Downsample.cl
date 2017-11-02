__constant sampler_t downsizeInputImageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

float4 GetDataF4Img(int2 pos, image2d_t image)
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

__kernel void DownsampleMain(
	__read_only		image2d_t		inputImage,				// Input image to be downsized
	__write_only	image2d_t		outputImage,			// Output image
								int2				inputImageSize)		// Size (width/height) of the input image
{
	int2 gPos = (int2)(get_global_id(0), get_global_id(1));
	if (gPos.x >= inputImageSize.x || gPos.y >= inputImageSize.y)
		return;

	float4 avgOfFour = AvgOfFour(gPos, inputImage);
	SetDataF4Img(avgOfFour, (int2)(gPos.x / 2, gPos.y / 2), outputImage);
}
