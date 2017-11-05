__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

float4 GetDataF4Img(float2 pos, image2d_t image)
{
	return read_imagef(image, sampler, pos);
}

void SetDataF4Img(float4 data, float2 pos, image2d_t image)
{
	write_imagef(image, convert_int2(pos), data);
}

__kernel void CompareMain
(
	__read_only  image2d_t imgA,	// Base image
	__read_only  image2d_t imgB,	// Aligned image
	__write_only image2d_t imgW, // Workspace image (difference)
	float2 imgASize,
	float2 imgBSize,
	float2 imgWSize,
	float4 matRotScale,
	float2 trans
)
{
	float2 texelPosA = (float2)(get_global_id(0), get_global_id(1));
	if (texelPosA.x >= imgASize.x || texelPosA.y >= imgASize.y)
		return;

	float4 pixelImgA = read_imagef(imgA, sampler, texelPosA);
	float2 imgBHalfSize = imgBSize * (float2)(0.5f, 0.5f);
	float2 imgAHalfSize = imgASize * (float2)(0.5f, 0.5f);
	float2 texelPosB = texelPosA + trans - imgAHalfSize;
	texelPosB = ((float2)
		(texelPosB.x * matRotScale.x + texelPosB.y * matRotScale.z,
			texelPosB.x * matRotScale.y + texelPosB.y * matRotScale.w)) + (imgBHalfSize);

	float4 pixelImgB = read_imagef(imgB, sampler, texelPosB);
	float4 diff = fabs(pixelImgA - pixelImgB);

	if ( texelPosB.x <= 0 || texelPosB.y <= 0 || texelPosB.x >= imgBSize.x || texelPosB.y >= imgBSize.y
		|| texelPosA.x <= 0 || texelPosA.y <= 0 || texelPosA.x >= imgASize.x || texelPosA.y >= imgASize.y)
	{
		diff.x = 0;
		diff.y = 0;
		diff.z = 0;
		diff.w = 0;
	}
	else
	{
		diff.w = 1;
	}

	SetDataF4Img(diff, texelPosA, imgW);
	return;
}
