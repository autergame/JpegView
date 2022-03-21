// GENERATED CODE
// FDCT IDCT 16x16

inline void fdct_1d_16x16(const float* src, float *dst, int stridea, int strideb)
{
	static const float v_00 =  1.407404f;
	static const float v_01 =  0.138617f;
	static const float v_02 =  0.250000f;
	static const float v_03 =  0.785695f;
	static const float v_04 =  1.093202f;
	static const float v_05 =  1.353318f;
	static const float v_06 =  0.897168f;
	static const float v_07 =  0.666656f;
	static const float v_08 =  1.247225f;
	static const float v_09 =  1.175876f;
	static const float v_0A = -0.410525f;
	static const float v_0B =  0.410525f;
	static const float v_0C =  0.275899f;
	static const float v_0D =  0.326641f;
	static const float v_0E =  1.387040f;
	static const float v_0F = -0.785695f;
	static const float v_10 =  0.707107f;
	static const float v_11 = -0.897168f;
	static const float v_12 =  0.135299f;

	for (int i = 0; i < 16; i++)
	{
		const float x_00 = src[ 0 * stridea] + src[15 * stridea];
		const float x_01 = src[ 1 * stridea] + src[14 * stridea];
		const float x_02 = src[ 2 * stridea] + src[13 * stridea];
		const float x_03 = src[ 3 * stridea] + src[12 * stridea];
		const float x_04 = src[ 4 * stridea] + src[11 * stridea];
		const float x_05 = src[ 5 * stridea] + src[10 * stridea];
		const float x_06 = src[ 6 * stridea] + src[ 9 * stridea];
		const float x_07 = src[ 7 * stridea] + src[ 8 * stridea];
		const float x_08 = src[ 0 * stridea] - src[15 * stridea];
		const float x_09 = src[ 1 * stridea] - src[14 * stridea];
		const float x_0A = src[ 2 * stridea] - src[13 * stridea];
		const float x_0B = src[ 3 * stridea] - src[12 * stridea];
		const float x_0C = src[ 4 * stridea] - src[11 * stridea];
		const float x_0D = src[ 5 * stridea] - src[10 * stridea];
		const float x_0E = src[ 6 * stridea] - src[ 9 * stridea];
		const float x_0F = src[ 7 * stridea] - src[ 8 * stridea];
		const float x_10 = x_00 + x_07;
		const float x_11 = x_01 + x_06;
		const float x_12 = x_02 + x_05;
		const float x_13 = x_03 + x_04;
		const float x_14 = x_00 - x_07;
		const float x_15 = x_01 - x_06;
		const float x_16 = x_02 - x_05;
		const float x_17 = x_03 - x_04;
		const float x_18 = x_10 + x_13;
		const float x_19 = x_11 + x_12;
		const float x_1A = x_10 - x_13;
		const float x_1B = x_11 - x_12;
		const float x_1C = v_0E * x_14 + v_0C * x_17;
		const float x_1D = v_09 * x_15 + v_03 * x_16;
		const float x_1E = -v_03 * x_15 + v_09 * x_16;
		const float x_1F = v_0C * x_14 - v_0E * x_17;
		const float x_20 = v_02 * (x_1C - x_1D);
		const float x_21 = v_02 * (x_1E - x_1F);
		const float x_22 = v_00 * x_08 + v_01 * x_0F;
		const float x_23 = v_05 * x_09 + v_0B * x_0E;
		const float x_24 = v_08 * x_0A + v_07 * x_0D;
		const float x_25 = v_04 * x_0B + v_06 * x_0C;
		const float x_26 = -v_06 * x_0B + v_04 * x_0C;
		const float x_27 = v_07 * x_0A - v_08 * x_0D;
		const float x_28 = v_0A * x_09 + v_05 * x_0E;
		const float x_29 = v_01 * x_08 - v_00 * x_0F;
		const float x_2A = x_22 + x_25;
		const float x_2B = x_23 + x_24;
		const float x_2C = x_22 - x_25;
		const float x_2D = x_23 - x_24;
		const float x_2E = v_02 * (x_2A - x_2B);
		const float x_2F = v_0D * x_2C + v_12 * x_2D;
		const float x_30 = v_12 * x_2C - v_0D * x_2D;
		const float x_31 = x_26 + x_29;
		const float x_32 = x_27 + x_28;
		const float x_33 = x_26 - x_29;
		const float x_34 = x_27 - x_28;
		const float x_35 = v_02 * (x_31 - x_32);
		const float x_36 = v_0D * x_33 + v_12 * x_34;
		const float x_37 = v_12 * x_33 - v_0D * x_34;
		dst[ 0 * stridea] = v_02 * (x_18 + x_19);
		dst[ 1 * stridea] = v_02 * (x_2A + x_2B);
		dst[ 2 * stridea] = v_02 * (x_1C + x_1D);
		dst[ 3 * stridea] = v_10 * (x_2F - x_37);
		dst[ 4 * stridea] = v_0D * x_1A + v_12 * x_1B;
		dst[ 5 * stridea] = v_10 * (x_2F + x_37);
		dst[ 6 * stridea] = v_10 * (x_20 - x_21);
		dst[ 7 * stridea] = v_10 * (x_2E + x_35);
		dst[ 8 * stridea] = v_02 * (x_18 - x_19);
		dst[ 9 * stridea] = v_10 * (x_2E - x_35);
		dst[10 * stridea] = v_10 * (x_20 + x_21);
		dst[11 * stridea] = v_10 * (x_30 - x_36);
		dst[12 * stridea] = v_12 * x_1A - v_0D * x_1B;
		dst[13 * stridea] = v_10 * (x_30 + x_36);
		dst[14 * stridea] = v_02 * (x_1E + x_1F);
		dst[15 * stridea] = v_02 * (x_31 + x_32);
		dst += strideb;
		src += strideb;
	}
}

void fdct_16x16(const float* src, float *dst)
{
	float* tmp = (float*)calloc(16 * 16, sizeof(float));
	fdct_1d_16x16(src, tmp, 1, 16);
	fdct_1d_16x16(tmp, dst, 16, 1);
	free(tmp);
}

inline void idct_1d_16x16(const float* src, float *dst, int stridea, int strideb)
{
	static const float v_00 =  1.407404f;
	static const float v_01 =  0.138617f;
	static const float v_02 =  0.250000f;
	static const float v_03 =  0.785695f;
	static const float v_04 =  1.093202f;
	static const float v_05 =  1.353318f;
	static const float v_06 =  0.541196f;
	static const float v_07 =  0.897168f;
	static const float v_08 =  0.666656f;
	static const float v_09 =  1.247225f;
	static const float v_0A =  1.175876f;
	static const float v_0B = -0.410525f;
	static const float v_0C = -0.541196f;
	static const float v_0D = -0.666656f;
	static const float v_0E =  0.410525f;
	static const float v_0F =  0.275899f;
	static const float v_10 =  1.387040f;
	static const float v_11 =  0.353553f;
	static const float v_12 =  0.707107f;
	static const float v_13 =  1.414214f;
	static const float v_14 =  0.176777f;
	static const float v_15 = -0.897168f;
	static const float v_16 = -0.138617f;
	static const float v_17 =  1.306563f;

	for (int i = 0; i < 16; i++)
	{
		const float x_00 = v_13 * src[ 0 * stridea];
		const float x_01 = v_00 * src[ 1 * stridea] + v_01 * src[15 * stridea];
		const float x_02 = v_10 * src[ 2 * stridea] + v_0F * src[14 * stridea];
		const float x_03 = v_05 * src[ 3 * stridea] + v_0E * src[13 * stridea];
		const float x_04 = v_17 * src[ 4 * stridea] + v_06 * src[12 * stridea];
		const float x_05 = v_09 * src[ 5 * stridea] + v_08 * src[11 * stridea];
		const float x_06 = v_0A * src[ 6 * stridea] + v_03 * src[10 * stridea];
		const float x_07 = v_04 * src[ 7 * stridea] + v_07 * src[ 9 * stridea];
		const float x_08 = v_13 * src[ 8 * stridea];
		const float x_09 = -v_07 * src[ 7 * stridea] + v_04 * src[ 9 * stridea];
		const float x_0A = v_03 * src[ 6 * stridea] - v_0A * src[10 * stridea];
		const float x_0B = -v_08 * src[ 5 * stridea] + v_09 * src[11 * stridea];
		const float x_0C = v_06 * src[ 4 * stridea] - v_17 * src[12 * stridea];
		const float x_0D = v_0B * src[ 3 * stridea] + v_05 * src[13 * stridea];
		const float x_0E = v_0F * src[ 2 * stridea] - v_10 * src[14 * stridea];
		const float x_0F = -v_01 * src[ 1 * stridea] + v_00 * src[15 * stridea];
		const float x_12 = x_00 + x_08;
		const float x_13 = x_01 + x_07;
		const float x_14 = x_02 + x_06;
		const float x_15 = x_03 + x_05;
		const float x_16 = v_13 * x_04;
		const float x_17 = x_00 - x_08;
		const float x_18 = x_01 - x_07;
		const float x_19 = x_02 - x_06;
		const float x_1A = x_03 - x_05;
		const float x_1D = x_12 + x_16;
		const float x_1E = x_13 + x_15;
		const float x_1F = v_13 * x_14;
		const float x_20 = x_12 - x_16;
		const float x_21 = x_13 - x_15;
		const float x_22 = v_02 * (x_1D - x_1F);
		const float x_23 = v_02 * (x_20 + x_21);
		const float x_24 = v_02 * (x_20 - x_21);
		const float x_25 = v_13 * x_17;
		const float x_26 = v_17 * x_18 + v_06 * x_1A;
		const float x_27 = v_13 * x_19;
		const float x_28 = -v_06 * x_18 + v_17 * x_1A;
		const float x_29 = v_14 * (x_25 + x_27) + v_02 * x_26;
		const float x_2A = v_02 * (x_25 - x_27);
		const float x_2B = v_14 * (x_25 + x_27) - v_02 * x_26;
		const float x_2C = v_11 * x_28;
		const float x_1B = v_12 * (x_2A - x_2C);
		const float x_1C = v_12 * (x_2A + x_2C);
		const float x_2D = v_13 * x_0C;
		const float x_2E = x_0B + x_0D;
		const float x_2F = x_0A + x_0E;
		const float x_30 = x_09 + x_0F;
		const float x_31 = x_09 - x_0F;
		const float x_32 = x_0A - x_0E;
		const float x_33 = x_0B - x_0D;
		const float x_37 = v_13 * x_2D;
		const float x_38 = v_17 * x_2E + v_06 * x_30;
		const float x_39 = v_13 * x_2F;
		const float x_3A = -v_06 * x_2E + v_17 * x_30;
		const float x_3B = v_14 * (x_37 + x_39) + v_02 * x_38;
		const float x_3C = v_02 * (x_37 - x_39);
		const float x_3D = v_14 * (x_37 + x_39) - v_02 * x_38;
		const float x_3E = v_11 * x_3A;
		const float x_34 = v_12 * (x_3C - x_3E);
		const float x_35 = v_12 * (x_3C + x_3E);
		const float x_3F = v_13 * x_32;
		const float x_40 = x_31 + x_33;
		const float x_41 = x_31 - x_33;
		const float x_42 = v_02 * (x_3F + x_40);
		const float x_43 = v_02 * (x_3F - x_40);
		const float x_44 = v_11 * x_41;
		const float x_36 = -x_43;
		const float x_10 = -x_34;
		const float x_11 = -x_3D;
		dst[ 0 * stridea] = v_14 * (x_1D + x_1F) + v_02 * x_1E;
		dst[ 1 * stridea] = v_12 * (x_29 - x_11);
		dst[ 2 * stridea] = v_12 * (x_29 + x_11);
		dst[ 3 * stridea] = v_12 * (x_23 + x_36);
		dst[ 4 * stridea] = v_12 * (x_23 - x_36);
		dst[ 5 * stridea] = v_12 * (x_1B - x_35);
		dst[ 6 * stridea] = v_12 * (x_1B + x_35);
		dst[ 7 * stridea] = v_12 * (x_22 + x_44);
		dst[ 8 * stridea] = v_12 * (x_22 - x_44);
		dst[ 9 * stridea] = v_12 * (x_1C - x_10);
		dst[10 * stridea] = v_12 * (x_1C + x_10);
		dst[11 * stridea] = v_12 * (x_24 + x_42);
		dst[12 * stridea] = v_12 * (x_24 - x_42);
		dst[13 * stridea] = v_12 * (x_2B - x_3B);
		dst[14 * stridea] = v_12 * (x_2B + x_3B);
		dst[15 * stridea] = v_14 * (x_1D + x_1F) - v_02 * x_1E;
		dst += strideb;
		src += strideb;
	}
}

void idct_16x16(const float* src, float *dst)
{
	float* tmp = (float*)calloc(16 * 16, sizeof(float));
	idct_1d_16x16(src, tmp, 1, 16);
	idct_1d_16x16(tmp, dst, 16, 1);
	free(tmp);
}