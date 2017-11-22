__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

float4 GetDataF4Img(float2 pos, image2d_t image)
{
	return read_imagef(image, sampler, pos);
}

void SetDataF4Img(float4 data, float2 pos, image2d_t image)
{
	write_imagef(image, convert_int2(pos), data);
}

__kernel void WeightCalcMain
(
	__read_only image2d_t imgA,
	
	__global float2 imgSize,
	__global float4* matRotScales,
	__global float2* transitions

	__global float* weights,
)
{
	return;
}
