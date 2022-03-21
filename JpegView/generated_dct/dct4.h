// GENERATED CODE
// FDCT IDCT 4x4

inline void fdct_1d_4x4(const float* src, float *dst, int stridea, int strideb)
{
	static const float v_0 = 0.653281f;
	static const float v_1 = 0.270598f;
	static const float v_2 = 0.500000f;

	for (int i = 0; i < 4; i++)
	{
		const float x_0 = src[0 * stridea] + src[3 * stridea];
		const float x_1 = src[1 * stridea] + src[2 * stridea];
		const float x_2 = src[0 * stridea] - src[3 * stridea];
		const float x_3 = src[1 * stridea] - src[2 * stridea];
		dst[0 * stridea] = v_2 * (x_0 + x_1);
		dst[1 * stridea] = v_0 * x_2 + v_1 * x_3;
		dst[2 * stridea] = v_2 * (x_0 - x_1);
		dst[3 * stridea] = v_1 * x_2 - v_0 * x_3;
		dst += strideb;
		src += strideb;
	}
}

void fdct_4x4(const float* src, float *dst)
{
	float* tmp = (float*)calloc(4 * 4, sizeof(float));
	fdct_1d_4x4(src, tmp, 1, 4);
	fdct_1d_4x4(tmp, dst, 4, 1);
	free(tmp);
}

inline void idct_1d_4x4(const float* src, float *dst, int stridea, int strideb)
{
	static const float v_0 =  0.353553f;
	static const float v_1 = -0.541196f;
	static const float v_2 =  1.306563f;
	static const float v_3 =  0.707107f;
	static const float v_4 =  0.500000f;
	static const float v_5 =  1.414214f;
	static const float v_6 =  0.541196f;

	for (int i = 0; i < 4; i++)
	{
		const float x_0 = v_5 * src[0 * stridea];
		const float x_1 = v_2 * src[1 * stridea] + v_6 * src[3 * stridea];
		const float x_2 = v_5 * src[2 * stridea];
		const float x_3 = v_1 * src[1 * stridea] + v_2 * src[3 * stridea];
		const float x_4 = v_4 * (x_0 - x_2);
		const float x_5 = v_3 * x_3;
		dst[0 * stridea] = v_0 * (x_0 + x_2) + v_4 * x_1;
		dst[1 * stridea] = v_3 * (x_4 - x_5);
		dst[2 * stridea] = v_3 * (x_4 + x_5);
		dst[3 * stridea] = v_0 * (x_0 + x_2) - v_4 * x_1;
		dst += strideb;
		src += strideb;
	}
}

void idct_4x4(const float* src, float *dst)
{
	float* tmp = (float*)calloc(4 * 4, sizeof(float));
	idct_1d_4x4(src, tmp, 1, 4);
	idct_1d_4x4(tmp, dst, 4, 1);
	free(tmp);
}