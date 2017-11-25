__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

__kernel void WeightCalcMain
(
						int				numImages,
						float2		imgSize,
	__global float4*		matRotScales,
	__global float2*		translations,
	__global float*			weights,
	__write_only	image2d_t		outputImage,			// Output image
	[[DEF_IN_IMAGE_ARGS]]
)
{

	float2 gPos = (float2)(get_global_id(0), get_global_id(1));
	if (gPos.x >= imgSize.x || gPos.y >= imgSize.y)
		return;

	float imgWeights[128];
	float4 colorAllImages[128]; // This conatins the current color values (count of images: numImages)
	float4 colorAllImagesE[128]; // Environment AVG color
	float2 imgHalfSize = imgSize * (float2)(0.5f, 0.5f);
	float2 pos;

	[[DEF_FILL_ALL_IMAGE_COLOR]]

	float4 avgValue = (float4)(0,0,0,1);

	for (int i = 0; i < numImages; i++)
	{
		avgValue += colorAllImages[i];
	}

	avgValue = avgValue / (float4)(numImages, numImages, numImages, 1);
	avgValue.w = 1.0;

	// Color Diff
	float maxDiff = 0;
	int maxDiffIndex = 0;
	for (int i = 0; i < numImages; i++)
	{
		float4 diff = fabs(avgValue - colorAllImages[i]);
		if (diff.x > maxDiff) { maxDiff = diff.x; maxDiffIndex = i; }
		if (diff.y > maxDiff) { maxDiff = diff.y; maxDiffIndex = i; }
		if (diff.z > maxDiff) { maxDiff = diff.z; maxDiffIndex = i; }
		
	}
	// Sharpness
	float maxSharp = 0;
	int maxSharpIndex = 0;
	for (int i = 0; i < numImages; i++)
	{
		float4 sharp = fabs(colorAllImagesE[i] / 8.0f - colorAllImages[i]);
		if (sharp.x > maxSharp) { maxSharp = sharp.x; maxSharpIndex = i; }
		if (sharp.y > maxSharp) { maxSharp = sharp.y; maxSharpIndex = i; }
		if (sharp.z > maxSharp) { maxSharp = sharp.z; maxSharpIndex = i; }
	}

	for (int i = 0; i < numImages; i++)
	{
		float4 sharp = fabs(colorAllImagesE[i] / 8.0f - colorAllImages[i]);
		float4 diff = fabs(avgValue - colorAllImages[i]);
		//imgWeights[i] = (sharp.x / (maxSharp +0.001f));
		imgWeights[i] = (diff.x / (maxDiff + 0.001f));
	}

	float totalWeights = 0;
	float4 finalValue = (float4)(0,0,0,0);
	for (int i = 0; i < numImages; i++)
	{
		totalWeights += imgWeights[i];
		finalValue += colorAllImages[i] * imgWeights[i];
	}
	finalValue = finalValue / (totalWeights + 0.0001f);
	finalValue.w = 1;
	
	write_imagef(outputImage, (int2)(gPos.x, gPos.y), finalValue);

	return;
}
