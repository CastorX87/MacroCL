__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;


unsigned int GetPixel(int2 pos, int2 size, __global unsigned int* imgBuffer)
{
	//pos = clamp(pos, (int2)(0,0), size - (int2)(1,1));
	return imgBuffer[pos.y * size.x + pos.x];
}

void SetPixel(unsigned int color, int2 pos, int2 size, __global unsigned int* imgBuffer)
{
	imgBuffer[pos.y * size.x + pos.x] = color;
}

float GetDataF1(int2 pos, int2 size, __global float* dataBuffer)
{
	pos = clamp(pos, (int2)(0, 0), size - (int2)(1, 1));
	return dataBuffer[pos.y * size.x + pos.x];
}

float4 GetDataF4(int2 pos, int2 size, __global float4* dataBuffer)
{
	pos = clamp(pos, (int2)(0, 0), size - (int2)(1, 1));
	return dataBuffer[pos.y * size.x + pos.x];
}

float4 GetDataF4Img(int2 pos, image2d_t image)
{
	return read_imagef(image, sampler, pos);
}

void SetDataF1(float data, int2 pos, int2 size, __global float* dataBuffer)
{
	dataBuffer[pos.y * size.x + pos.x] = data;
}

void SetDataF4(float4 data, int2 pos, int2 size, __global float4* dataBuffer)
{
	dataBuffer[pos.y * size.x + pos.x] = data;
}

void SetDataF4Img(float4 data, int2 pos, image2d_t image)
{
	write_imagef(image, pos, data);
}


uint Float4ToUIntColor(float4 val)
{
	return (
		((uint)(0xFFFFFFFF) << 24) +
		((uint)(clamp(val.x, (float)0.0, (float)1.0) * 255)) << 16 +
		((uint)(clamp(val.y, (float)0.0, (float)1.0) * 255)) << 8 +
		((uint)(clamp(val.z, (float)0.0, (float)1.0) * 255))
		);
}

float4 mul_v4m(const float4 v, const float16 m)
{
	return (float4)(
		v.x * m.s0 + v.y * m.s4 + v.z * m.s8 + v.w * m.sc,
		v.x * m.s1 + v.y * m.s5 + v.z * m.s9 + v.w * m.sd,
		v.x * m.s2 + v.y * m.s6 + v.z * m.sa + v.w * m.se,
		v.x * m.s3 + v.y * m.s7 + v.z * m.sb + v.w * m.sf
		);
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
	transGPos = ((float2)(transGPos.x * rsMat.x + transGPos.y * rsMat.z, transGPos.x * rsMat.y + transGPos.y * rsMat.w) + (float2)(imgSize.x, imgSize.y) / 2.0f) + trans;//mul_v4m(transGPos, transMat);

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

__kernel void ClMainDownsampleHalfSizeAvg(
	__read_only image2d_t imgIn,
	__write_only image2d_t imgOut,
	int2 outImgSize)
{
	int2 gPos = (int2)(get_global_id(0), get_global_id(1));
	int2 gSize = (int2)(get_global_size(0), get_global_size(1));
	if (gPos.x >= outImgSize.x || gPos.y >= outImgSize.y)
	{
		//SetDataF4Img((float4)(1, 1, 1, 1), gPos, imgOut);
		return;
	}
	float4 avgOfFour = AvgOfFour(gPos * (int2)(2, 2), imgIn);
	SetDataF4Img(avgOfFour, gPos, imgOut);
}
