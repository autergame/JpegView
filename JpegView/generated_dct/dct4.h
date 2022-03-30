// GENERATED CODE
// FDCT IDCT 4x4

inline void fdct_1d_4x4(const float* src, float *out, int stridea, int strideb)
{
	static const float v_0 = 0.653281f;
	static const float v_1 = 0.270598f;
	static const float v_2 = 0.500000f;

	for (int i = 0; i < 4; i++)
	{
		const float s_0 = src[0 * stridea];
		const float s_1 = src[1 * stridea];
		const float s_2 = src[2 * stridea];
		const float s_3 = src[3 * stridea];

		const float x_0 = s_0 + s_3;
		const float x_1 = s_1 + s_2;
		const float x_2 = s_0 - s_3;
		const float x_3 = s_1 - s_2;

		out[0 * stridea] = v_2 * (x_0 + x_1);
		out[1 * stridea] = v_0 * x_2 + v_1 * x_3;
		out[2 * stridea] = v_2 * (x_0 - x_1);
		out[3 * stridea] = v_1 * x_2 - v_0 * x_3;

		out += strideb;
		src += strideb;
	}
}

void fdct_4x4(const float* src, float *out)
{
	float* tmp = (float*)calloc(4 * 4, sizeof(float));
	fdct_1d_4x4(src, tmp, 1, 4);
	fdct_1d_4x4(tmp, out, 4, 1);
	free(tmp);
}

inline void idct_1d_4x4(const float* src, float *out, int stridea, int strideb)
{
	static const float v_0 =  0.353553f;
	static const float v_1 = -0.541196f;
	static const float v_2 =  1.306563f;
	static const float v_3 =  0.500000f;
	static const float v_4 =  0.541196f;
	static const float v_5 =  1.414214f;
	static const float v_6 =  0.707107f;

	for (int i = 0; i < 4; i++)
	{
		const float s_0 = src[0 * stridea];
		const float s_1 = src[1 * stridea];
		const float s_2 = src[2 * stridea];
		const float s_3 = src[3 * stridea];

		const float x_0 = v_5 * s_0;
		const float x_1 = v_2 * s_1 + v_4 * s_3;
		const float x_2 = v_5 * s_2;
		const float x_3 = v_1 * s_1 + v_2 * s_3;
		const float x_4 = v_3 * (x_0 - x_2);
		const float x_5 = v_6 * x_3;

		out[0 * stridea] = v_0 * (x_0 + x_2) + v_3 * x_1;
		out[1 * stridea] = v_6 * (x_4 - x_5);
		out[2 * stridea] = v_6 * (x_4 + x_5);
		out[3 * stridea] = v_0 * (x_0 + x_2) - v_3 * x_1;

		out += strideb;
		src += strideb;
	}
}

void idct_4x4(const float* src, float *out)
{
	float* tmp = (float*)calloc(4 * 4, sizeof(float));
	idct_1d_4x4(src, tmp, 1, 4);
	idct_1d_4x4(tmp, out, 4, 1);
	free(tmp);
}