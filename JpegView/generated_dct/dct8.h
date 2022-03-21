// GENERATED CODE
// FDCT IDCT 8x8

inline void fdct_1d_8x8(const float* src, float *dst, int stridea, int strideb)
{
	static const float v_0 =  1.175876f;
	static const float v_1 =  0.353553f;
	static const float v_2 = -0.785695f;
	static const float v_3 =  0.707107f;
	static const float v_4 =  0.785695f;
	static const float v_5 =  0.461940f;
	static const float v_6 =  0.191342f;
	static const float v_7 =  0.275899f;
	static const float v_8 =  1.387040f;

	for (int i = 0; i < 8; i++)
	{
		const float x_00 = src[0 * stridea] + src[7 * stridea];
		const float x_01 = src[1 * stridea] + src[6 * stridea];
		const float x_02 = src[2 * stridea] + src[5 * stridea];
		const float x_03 = src[3 * stridea] + src[4 * stridea];
		const float x_04 = src[0 * stridea] - src[7 * stridea];
		const float x_05 = src[1 * stridea] - src[6 * stridea];
		const float x_06 = src[2 * stridea] - src[5 * stridea];
		const float x_07 = src[3 * stridea] - src[4 * stridea];
		const float x_08 = x_00 + x_03;
		const float x_09 = x_01 + x_02;
		const float x_0A = x_00 - x_03;
		const float x_0B = x_01 - x_02;
		const float x_0C = v_8 * x_04 + v_7 * x_07;
		const float x_0D = v_0 * x_05 + v_4 * x_06;
		const float x_0E = v_2 * x_05 + v_0 * x_06;
		const float x_0F = v_7 * x_04 - v_8 * x_07;
		const float x_10 = v_1 * (x_0C - x_0D);
		const float x_11 = v_1 * (x_0E - x_0F);
		dst[0 * stridea] = v_1 * (x_08 + x_09);
		dst[1 * stridea] = v_1 * (x_0C + x_0D);
		dst[2 * stridea] = v_5 * x_0A + v_6 * x_0B;
		dst[3 * stridea] = v_3 * (x_10 - x_11);
		dst[4 * stridea] = v_1 * (x_08 - x_09);
		dst[5 * stridea] = v_3 * (x_10 + x_11);
		dst[6 * stridea] = v_6 * x_0A - v_5 * x_0B;
		dst[7 * stridea] = v_1 * (x_0E + x_0F);
		dst += strideb;
		src += strideb;
	}
}

void fdct_8x8(const float* src, float *dst)
{
	float* tmp = (float*)calloc(8 * 8, sizeof(float));
	fdct_1d_8x8(src, tmp, 1, 8);
	fdct_1d_8x8(tmp, dst, 8, 1);
	free(tmp);
}

inline void idct_1d_8x8(const float* src, float *dst, int stridea, int strideb)
{
	static const float v_0 = -0.275899f;
	static const float v_1 =  1.175876f;
	static const float v_2 =  0.353553f;
	static const float v_3 = -0.785695f;
	static const float v_4 =  1.306563f;
	static const float v_5 =  0.785695f;
	static const float v_6 =  0.250000f;
	static const float v_7 =  0.500000f;
	static const float v_8 =  0.707107f;
	static const float v_9 =  0.275899f;
	static const float v_A =  1.414214f;
	static const float v_B =  0.541196f;
	static const float v_C =  1.387040f;

	for (int i = 0; i < 8; i++)
	{
		const float x_00 = v_A * src[0 * stridea];
		const float x_01 = v_C * src[1 * stridea] + v_9 * src[7 * stridea];
		const float x_02 = v_4 * src[2 * stridea] + v_B * src[6 * stridea];
		const float x_03 = v_1 * src[3 * stridea] + v_5 * src[5 * stridea];
		const float x_04 = v_A * src[4 * stridea];
		const float x_05 = v_3 * src[3 * stridea] + v_1 * src[5 * stridea];
		const float x_06 = v_B * src[2 * stridea] - v_4 * src[6 * stridea];
		const float x_07 = v_0 * src[1 * stridea] + v_C * src[7 * stridea];
		const float x_09 = x_00 + x_04;
		const float x_0A = x_01 + x_03;
		const float x_0B = v_A * x_02;
		const float x_0C = x_00 - x_04;
		const float x_0D = x_01 - x_03;
		const float x_0E = v_2 * (x_09 - x_0B);
		const float x_0F = v_2 * (x_0C + x_0D);
		const float x_10 = v_2 * (x_0C - x_0D);
		const float x_11 = v_A * x_06;
		const float x_12 = x_05 + x_07;
		const float x_13 = x_05 - x_07;
		const float x_14 = v_2 * (x_11 + x_12);
		const float x_15 = v_2 * (x_11 - x_12);
		const float x_16 = v_7 * x_13;
		const float x_08 = -x_15;
		dst[0 * stridea] = v_6 * (x_09 + x_0B) + v_2 * x_0A;
		dst[1 * stridea] = v_8 * (x_0F - x_08);
		dst[2 * stridea] = v_8 * (x_0F + x_08);
		dst[3 * stridea] = v_8 * (x_0E + x_16);
		dst[4 * stridea] = v_8 * (x_0E - x_16);
		dst[5 * stridea] = v_8 * (x_10 - x_14);
		dst[6 * stridea] = v_8 * (x_10 + x_14);
		dst[7 * stridea] = v_6 * (x_09 + x_0B) - v_2 * x_0A;
		dst += strideb;
		src += strideb;
	}
}

void idct_8x8(const float* src, float *dst)
{
	float* tmp = (float*)calloc(8 * 8, sizeof(float));
	idct_1d_8x8(src, tmp, 1, 8);
	idct_1d_8x8(tmp, dst, 8, 1);
	free(tmp);
}