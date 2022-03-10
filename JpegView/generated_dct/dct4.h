// GENERATED CODE
// FDCT IDCT 4x4

inline void fdct_1d_4x4(float *dst, const float *src,
	int dst_stridea, int dst_strideb, int src_stridea, int src_strideb)
{
	for (int i = 0; i < 4; i++)
	{
		const float x0 = src[0 * src_stridea] + src[3 * src_stridea];
		const float x1 = src[1 * src_stridea] + src[2 * src_stridea];
		const float x2 = src[0 * src_stridea] - src[3 * src_stridea];
		const float x3 = src[1 * src_stridea] - src[2 * src_stridea];
		dst[0 * dst_stridea] =  0.500000f * (x0 + x1);
		dst[1 * dst_stridea] =  0.653281f * x2 +  0.270598f * x3;
		dst[2 * dst_stridea] =  0.500000f * (x0 - x1);
		dst[3 * dst_stridea] =  0.270598f * x2 -  0.653281f * x3;
		dst += dst_strideb;
		src += src_strideb;
	}
}

void fdct_4x4(float *dst, const float *src)
{
	float* tmp = (float*)calloc(4 * 4, 4);
	fdct_1d_4x4(tmp, src, 1, 4, 1, 4);
	fdct_1d_4x4(dst, tmp, 4, 1, 4, 1);
	free(tmp);
}

inline void idct_1d_4x4(float *dst, const float *src,
	int dst_stridea, int dst_strideb, int src_stridea, int src_strideb)
{
	for (int i = 0; i < 4; i++)
	{
		const float x0 =  1.414214f * src[0 * src_stridea];
		const float x1 =  1.306563f * src[1 * src_stridea] +  0.541196f * src[3 * src_stridea];
		const float x2 =  1.414214f * src[2 * src_stridea];
		const float x3 = -0.541196f * src[1 * src_stridea] +  1.306563f * src[3 * src_stridea];
		const float x4 =  0.500000f * (x0 - x2);
		const float x5 =  0.707107f * x3;
		dst[0 * dst_stridea] =  0.353553f * (x0 + x2) +  0.500000f * x1;
		dst[1 * dst_stridea] =  0.707107f * (x4 - x5);
		dst[2 * dst_stridea] =  0.707107f * (x4 + x5);
		dst[3 * dst_stridea] =  0.353553f * (x0 + x2) -  0.500000f * x1;
		dst += dst_strideb;
		src += src_strideb;
	}
}

void idct_4x4(float *dst, const float *src)
{
	float* tmp = (float*)calloc(4 * 4, 4);
	idct_1d_4x4(tmp, src, 1, 4, 1, 4);
	idct_1d_4x4(dst, tmp, 4, 1, 4, 1);
	free(tmp);
}