// GENERATED CODE
// FDCT IDCT 2x2

inline void fdct_1d_2x2(const float* src, float *out, int stridea, int strideb)
{
	static const float v_0 = 0.707107f;

	for (int i = 0; i < 2; i++)
	{
		const float s_0 = src[0 * stridea];
		const float s_1 = src[1 * stridea];

		out[0 * stridea] = v_0 * (s_0 + s_1);
		out[1 * stridea] = v_0 * (s_0 - s_1);

		out += strideb;
		src += strideb;
	}
}

void fdct_2x2(const float* src, float *out)
{
	float* tmp = (float*)calloc(2 * 2, sizeof(float));
	fdct_1d_2x2(src, tmp, 1, 2);
	fdct_1d_2x2(tmp, out, 2, 1);
	free(tmp);
}

inline void idct_1d_2x2(const float* src, float *out, int stridea, int strideb)
{
	static const float v_0 = 0.707107f;

	for (int i = 0; i < 2; i++)
	{
		const float s_0 = src[0 * stridea];
		const float s_1 = src[1 * stridea];

		out[0 * stridea] = v_0 * (s_0 + s_1);
		out[1 * stridea] = v_0 * (s_0 - s_1);

		out += strideb;
		src += strideb;
	}
}

void idct_2x2(const float* src, float *out)
{
	float* tmp = (float*)calloc(2 * 2, sizeof(float));
	idct_1d_2x2(src, tmp, 1, 2);
	idct_1d_2x2(tmp, out, 2, 1);
	free(tmp);
}