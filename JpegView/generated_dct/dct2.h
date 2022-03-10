// GENERATED CODE
// FDCT IDCT 2x2

inline void fdct_1d_2x2(float *dst, const float *src,
	int dst_stridea, int dst_strideb, int src_stridea, int src_strideb)
{
	for (int i = 0; i < 2; i++)
	{
		dst[0 * dst_stridea] =  0.707107f * src[0 * src_stridea] +  0.707107f * src[1 * src_stridea];
		dst[1 * dst_stridea] =  0.707107f * src[0 * src_stridea] -  0.707107f * src[1 * src_stridea];
		dst += dst_strideb;
		src += src_strideb;
	}
}

void fdct_2x2(float *dst, const float *src)
{
	float* tmp = (float*)calloc(2 * 2, 4);
	fdct_1d_2x2(tmp, src, 1, 2, 1, 2);
	fdct_1d_2x2(dst, tmp, 2, 1, 2, 1);
	free(tmp);
}

inline void idct_1d_2x2(float *dst, const float *src,
	int dst_stridea, int dst_strideb, int src_stridea, int src_strideb)
{
	for (int i = 0; i < 2; i++)
	{
		dst[0 * dst_stridea] =  0.707107f * src[0 * src_stridea] +  0.707107f * src[1 * src_stridea];
		dst[1 * dst_stridea] =  0.707107f * src[0 * src_stridea] -  0.707107f * src[1 * src_stridea];
		dst += dst_strideb;
		src += src_strideb;
	}
}

void idct_2x2(float *dst, const float *src)
{
	float* tmp = (float*)calloc(2 * 2, 4);
	idct_1d_2x2(tmp, src, 1, 2, 1, 2);
	idct_1d_2x2(dst, tmp, 2, 1, 2, 1);
	free(tmp);
}