__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

float4 GetDataF4Img(int2 pos, image2d_t image)
{
	return read_imagef(image, sampler, pos);
}

void SetDataF4Img(float4 data, int2 pos, image2d_t image)
{
	write_imagef(image, pos, data);
}

__kernel void ClMainCalc(__write_only image2d_t imgDifference,
	__read_only image2d_t imgA,
	__read_only image2d_t imgB,
	uint2 imgSize,
	uint2 backSize,
	float4 rsMat,
	float2 trans)
{
	int2 gPos = (int2)(get_global_id(0), get_global_id(1));
	int2 gSize = (int2)(get_global_size(0), get_global_size(1));
	if (gPos.x >= imgSize.x || gPos.y >= imgSize.y)
	{
		if (gPos.x < backSize.x || gPos.y < backSize.y)
		{
			SetDataF4Img((float4)(0, 0, 0, 0), gPos, imgDifference);
		}
		return;
	}

	float4 pixelImgA = read_imagef(imgA, sampler, gPos);
	float2 transGPos = (float2)((float)gPos.x - imgSize.x / 2, (float)gPos.y - imgSize.y / 2);
	transGPos = ((float2)(transGPos.x * rsMat.x + transGPos.y * rsMat.z, transGPos.x * rsMat.y + transGPos.y * rsMat.w) + (float2)(imgSize.x, imgSize.y) / 2.0f) + trans;

	float4 pixelImgB = read_imagef(imgB, sampler, transGPos.xy);
	float4 diff = fabs(pixelImgA - pixelImgB);

	if (transGPos.x < 0 || transGPos.y < 0 || transGPos.x >= imgSize.x || transGPos.y >= imgSize.y)
	{
		diff.z = 0;
		diff.w = 0;
	}
	else
		diff.w = 1;

	SetDataF4Img(diff, gPos, imgDifference);
	return;
}
