#pragma once
class CLImageDownsampler
{
private:
	cl_program mDSProgram;
	cl_program mDSKernel;
public:
	CLImageDownsampler();
	~CLImageDownsampler();
};

