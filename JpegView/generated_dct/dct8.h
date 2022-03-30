// GENERATED CODE
// FDCT IDCT 8x8

inline void fdct_1d_8x8(const float* src, float *out, int stridea, int strideb)
{
	static const float v_0 =  0.275899f;
	static const float v_1 = -0.785695f;
	static const float v_2 =  0.461940f;
	static const float v_3 =  1.175876f;
	static const float v_4 =  0.353553f;
	static const float v_5 =  0.191342f;
	static const float v_6 =  1.387040f;
	static const float v_7 =  0.785695f;
	static const float v_8 =  0.707107f;

	for (int i = 0; i < 8; i++)
	{
		const float s_0 = src[0 * stridea];
		const float s_1 = src[1 * stridea];
		const float s_2 = src[2 * stridea];
		const float s_3 = src[3 * stridea];
		const float s_4 = src[4 * stridea];
		const float s_5 = src[5 * stridea];
		const float s_6 = src[6 * stridea];
		const float s_7 = src[7 * stridea];

		const float x_00 = s_0 + s_7;
		const float x_01 = s_1 + s_6;
		const float x_02 = s_2 + s_5;
		const float x_03 = s_3 + s_4;
		const float x_04 = s_0 - s_7;
		const float x_05 = s_1 - s_6;
		const float x_06 = s_2 - s_5;
		const float x_07 = s_3 - s_4;
		const float x_08 = x_00 + x_03;
		const float x_09 = x_01 + x_02;
		const float x_0A = x_00 - x_03;
		const float x_0B = x_01 - x_02;
		const float x_0C = v_6 * x_04 + v_0 * x_07;
		const float x_0D = v_3 * x_05 + v_7 * x_06;
		const float x_0E = v_1 * x_05 + v_3 * x_06;
		const float x_0F = v_0 * x_04 - v_6 * x_07;
		const float x_10 = v_4 * (x_0C - x_0D);
		const float x_11 = v_4 * (x_0E - x_0F);

		out[0 * stridea] = v_4 * (x_08 + x_09);
		out[1 * stridea] = v_4 * (x_0C + x_0D);
		out[2 * stridea] = v_2 * x_0A + v_5 * x_0B;
		out[3 * stridea] = v_8 * (x_10 - x_11);
		out[4 * stridea] = v_4 * (x_08 - x_09);
		out[5 * stridea] = v_8 * (x_10 + x_11);
		out[6 * stridea] = v_5 * x_0A - v_2 * x_0B;
		out[7 * stridea] = v_4 * (x_0E + x_0F);

		out += strideb;
		src += strideb;
	}
}

void fdct_8x8(const float* src, float *out)
{
	float* tmp = (float*)calloc(8 * 8, sizeof(float));
	fdct_1d_8x8(src, tmp, 1, 8);
	fdct_1d_8x8(tmp, out, 8, 1);
	free(tmp);
}

inline void idct_1d_8x8(const float* src, float *out, int stridea, int strideb)
{
	static const float v_0 =  0.275899f;
	static const float v_1 = -0.785695f;
	static const float v_2 =  0.785695f;
	static const float v_3 =  1.175876f;
	static const float v_4 =  0.353553f;
	static const float v_5 =  0.250000f;
	static const float v_6 = -0.275899f;
	static const float v_7 =  1.306563f;
	static const float v_8 =  0.500000f;
	static const float v_9 =  0.541196f;
	static const float v_A =  1.387040f;
	static const float v_B =  1.414214f;
	static const float v_C =  0.707107f;

	for (int i = 0; i < 8; i++)
	{
		const float s_0 = src[0 * stridea];
		const float s_1 = src[1 * stridea];
		const float s_2 = src[2 * stridea];
		const float s_3 = src[3 * stridea];
		const float s_4 = src[4 * stridea];
		const float s_5 = src[5 * stridea];
		const float s_6 = src[6 * stridea];
		const float s_7 = src[7 * stridea];

		const float x_00 = v_B * s_0;
		const float x_01 = v_A * s_1 + v_0 * s_7;
		const float x_02 = v_7 * s_2 + v_9 * s_6;
		const float x_03 = v_3 * s_3 + v_2 * s_5;
		const float x_04 = v_B * s_4;
		const float x_05 = v_1 * s_3 + v_3 * s_5;
		const float x_06 = v_9 * s_2 - v_7 * s_6;
		const float x_07 = -v_0 * s_1 + v_A * s_7;
		const float x_09 = x_00 + x_04;
		const float x_0A = x_01 + x_03;
		const float x_0B = v_B * x_02;
		const float x_0C = x_00 - x_04;
		const float x_0D = x_01 - x_03;
		const float x_0E = v_4 * (x_09 - x_0B);
		const float x_0F = v_4 * (x_0C + x_0D);
		const float x_10 = v_4 * (x_0C - x_0D);
		const float x_11 = v_B * x_06;
		const float x_12 = x_05 + x_07;
		const float x_13 = x_05 - x_07;
		const float x_14 = v_4 * (x_11 + x_12);
		const float x_15 = v_4 * (x_11 - x_12);
		const float x_16 = v_8 * x_13;
		const float x_08 = -x_15;

		out[0 * stridea] = v_5 * (x_09 + x_0B) + v_4 * x_0A;
		out[1 * stridea] = v_C * (x_0F - x_08);
		out[2 * stridea] = v_C * (x_0F + x_08);
		out[3 * stridea] = v_C * (x_0E + x_16);
		out[4 * stridea] = v_C * (x_0E - x_16);
		out[5 * stridea] = v_C * (x_10 - x_14);
		out[6 * stridea] = v_C * (x_10 + x_14);
		out[7 * stridea] = v_5 * (x_09 + x_0B) - v_4 * x_0A;

		out += strideb;
		src += strideb;
	}
}

void idct_8x8(const float* src, float *out)
{
	float* tmp = (float*)calloc(8 * 8, sizeof(float));
	idct_1d_8x8(src, tmp, 1, 8);
	idct_1d_8x8(tmp, out, 8, 1);
	free(tmp);
}