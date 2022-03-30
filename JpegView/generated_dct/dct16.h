// GENERATED CODE
// FDCT IDCT 16x16

inline void fdct_1d_16x16(const float* src, float *out, int stridea, int strideb)
{
	static const float v_00 = -0.785695f;
	static const float v_01 =  1.175876f;
	static const float v_02 =  1.093202f;
	static const float v_03 =  1.247225f;
	static const float v_04 =  0.707107f;
	static const float v_05 =  0.326641f;
	static const float v_06 =  0.138617f;
	static const float v_07 =  0.785695f;
	static const float v_08 =  0.250000f;
	static const float v_09 =  0.410525f;
	static const float v_0A =  1.353318f;
	static const float v_0B =  1.387040f;
	static const float v_0C =  0.275899f;
	static const float v_0D =  1.407404f;
	static const float v_0E =  0.897168f;
	static const float v_0F =  0.666656f;
	static const float v_10 = -0.897168f;
	static const float v_11 =  0.135299f;
	static const float v_12 = -0.410525f;

	for (int i = 0; i < 16; i++)
	{
		const float s_00 = src[ 0 * stridea];
		const float s_01 = src[ 1 * stridea];
		const float s_02 = src[ 2 * stridea];
		const float s_03 = src[ 3 * stridea];
		const float s_04 = src[ 4 * stridea];
		const float s_05 = src[ 5 * stridea];
		const float s_06 = src[ 6 * stridea];
		const float s_07 = src[ 7 * stridea];
		const float s_08 = src[ 8 * stridea];
		const float s_09 = src[ 9 * stridea];
		const float s_0A = src[10 * stridea];
		const float s_0B = src[11 * stridea];
		const float s_0C = src[12 * stridea];
		const float s_0D = src[13 * stridea];
		const float s_0E = src[14 * stridea];
		const float s_0F = src[15 * stridea];

		const float x_00 = s_00 + s_0F;
		const float x_01 = s_01 + s_0E;
		const float x_02 = s_02 + s_0D;
		const float x_03 = s_03 + s_0C;
		const float x_04 = s_04 + s_0B;
		const float x_05 = s_05 + s_0A;
		const float x_06 = s_06 + s_09;
		const float x_07 = s_07 + s_08;
		const float x_08 = s_00 - s_0F;
		const float x_09 = s_01 - s_0E;
		const float x_0A = s_02 - s_0D;
		const float x_0B = s_03 - s_0C;
		const float x_0C = s_04 - s_0B;
		const float x_0D = s_05 - s_0A;
		const float x_0E = s_06 - s_09;
		const float x_0F = s_07 - s_08;
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
		const float x_1C = v_0B * x_14 + v_0C * x_17;
		const float x_1D = v_01 * x_15 + v_07 * x_16;
		const float x_1E = v_00 * x_15 + v_01 * x_16;
		const float x_1F = v_0C * x_14 - v_0B * x_17;
		const float x_20 = v_08 * (x_1C - x_1D);
		const float x_21 = v_08 * (x_1E - x_1F);
		const float x_22 = v_0D * x_08 + v_06 * x_0F;
		const float x_23 = v_0A * x_09 + v_09 * x_0E;
		const float x_24 = v_03 * x_0A + v_0F * x_0D;
		const float x_25 = v_02 * x_0B + v_0E * x_0C;
		const float x_26 = -v_0E * x_0B + v_02 * x_0C;
		const float x_27 = v_0F * x_0A - v_03 * x_0D;
		const float x_28 = -v_09 * x_09 + v_0A * x_0E;
		const float x_29 = v_06 * x_08 - v_0D * x_0F;
		const float x_2A = x_22 + x_25;
		const float x_2B = x_23 + x_24;
		const float x_2C = x_22 - x_25;
		const float x_2D = x_23 - x_24;
		const float x_2E = v_08 * (x_2A - x_2B);
		const float x_2F = v_05 * x_2C + v_11 * x_2D;
		const float x_30 = v_11 * x_2C - v_05 * x_2D;
		const float x_31 = x_26 + x_29;
		const float x_32 = x_27 + x_28;
		const float x_33 = x_26 - x_29;
		const float x_34 = x_27 - x_28;
		const float x_35 = v_08 * (x_31 - x_32);
		const float x_36 = v_05 * x_33 + v_11 * x_34;
		const float x_37 = v_11 * x_33 - v_05 * x_34;

		out[ 0 * stridea] = v_08 * (x_18 + x_19);
		out[ 1 * stridea] = v_08 * (x_2A + x_2B);
		out[ 2 * stridea] = v_08 * (x_1C + x_1D);
		out[ 3 * stridea] = v_04 * (x_2F - x_37);
		out[ 4 * stridea] = v_05 * x_1A + v_11 * x_1B;
		out[ 5 * stridea] = v_04 * (x_2F + x_37);
		out[ 6 * stridea] = v_04 * (x_20 - x_21);
		out[ 7 * stridea] = v_04 * (x_2E + x_35);
		out[ 8 * stridea] = v_08 * (x_18 - x_19);
		out[ 9 * stridea] = v_04 * (x_2E - x_35);
		out[10 * stridea] = v_04 * (x_20 + x_21);
		out[11 * stridea] = v_04 * (x_30 - x_36);
		out[12 * stridea] = v_11 * x_1A - v_05 * x_1B;
		out[13 * stridea] = v_04 * (x_30 + x_36);
		out[14 * stridea] = v_08 * (x_1E + x_1F);
		out[15 * stridea] = v_08 * (x_31 + x_32);

		out += strideb;
		src += strideb;
	}
}

void fdct_16x16(const float* src, float *out)
{
	float* tmp = (float*)calloc(16 * 16, sizeof(float));
	fdct_1d_16x16(src, tmp, 1, 16);
	fdct_1d_16x16(tmp, out, 16, 1);
	free(tmp);
}

inline void idct_1d_16x16(const float* src, float *out, int stridea, int strideb)
{
	static const float v_00 =  1.175876f;
	static const float v_01 =  1.093202f;
	static const float v_02 =  1.247225f;
	static const float v_03 =  0.707107f;
	static const float v_04 = -0.138617f;
	static const float v_05 = -0.666656f;
	static const float v_06 =  0.353553f;
	static const float v_07 =  1.306563f;
	static const float v_08 =  0.541196f;
	static const float v_09 =  0.785695f;
	static const float v_0A =  0.138617f;
	static const float v_0B =  0.176777f;
	static const float v_0C =  0.250000f;
	static const float v_0D =  0.410525f;
	static const float v_0E =  1.353318f;
	static const float v_0F =  1.387040f;
	static const float v_10 =  1.414214f;
	static const float v_11 =  1.407404f;
	static const float v_12 =  0.275899f;
	static const float v_13 =  0.897168f;
	static const float v_14 =  0.666656f;
	static const float v_15 = -0.897168f;
	static const float v_16 = -0.541196f;
	static const float v_17 = -0.410525f;

	for (int i = 0; i < 16; i++)
	{
		const float s_00 = src[ 0 * stridea];
		const float s_01 = src[ 1 * stridea];
		const float s_02 = src[ 2 * stridea];
		const float s_03 = src[ 3 * stridea];
		const float s_04 = src[ 4 * stridea];
		const float s_05 = src[ 5 * stridea];
		const float s_06 = src[ 6 * stridea];
		const float s_07 = src[ 7 * stridea];
		const float s_08 = src[ 8 * stridea];
		const float s_09 = src[ 9 * stridea];
		const float s_0A = src[10 * stridea];
		const float s_0B = src[11 * stridea];
		const float s_0C = src[12 * stridea];
		const float s_0D = src[13 * stridea];
		const float s_0E = src[14 * stridea];
		const float s_0F = src[15 * stridea];

		const float x_00 = v_10 * s_00;
		const float x_01 = v_11 * s_01 + v_0A * s_0F;
		const float x_02 = v_0F * s_02 + v_12 * s_0E;
		const float x_03 = v_0E * s_03 + v_0D * s_0D;
		const float x_04 = v_07 * s_04 + v_08 * s_0C;
		const float x_05 = v_02 * s_05 + v_14 * s_0B;
		const float x_06 = v_00 * s_06 + v_09 * s_0A;
		const float x_07 = v_01 * s_07 + v_13 * s_09;
		const float x_08 = v_10 * s_08;
		const float x_09 = -v_13 * s_07 + v_01 * s_09;
		const float x_0A = v_09 * s_06 - v_00 * s_0A;
		const float x_0B = v_05 * s_05 + v_02 * s_0B;
		const float x_0C = v_08 * s_04 - v_07 * s_0C;
		const float x_0D = -v_0D * s_03 + v_0E * s_0D;
		const float x_0E = v_12 * s_02 - v_0F * s_0E;
		const float x_0F = v_04 * s_01 + v_11 * s_0F;
		const float x_12 = x_00 + x_08;
		const float x_13 = x_01 + x_07;
		const float x_14 = x_02 + x_06;
		const float x_15 = x_03 + x_05;
		const float x_16 = v_10 * x_04;
		const float x_17 = x_00 - x_08;
		const float x_18 = x_01 - x_07;
		const float x_19 = x_02 - x_06;
		const float x_1A = x_03 - x_05;
		const float x_1D = x_12 + x_16;
		const float x_1E = x_13 + x_15;
		const float x_1F = v_10 * x_14;
		const float x_20 = x_12 - x_16;
		const float x_21 = x_13 - x_15;
		const float x_22 = v_0C * (x_1D - x_1F);
		const float x_23 = v_0C * (x_20 + x_21);
		const float x_24 = v_0C * (x_20 - x_21);
		const float x_25 = v_10 * x_17;
		const float x_26 = v_07 * x_18 + v_08 * x_1A;
		const float x_27 = v_10 * x_19;
		const float x_28 = -v_08 * x_18 + v_07 * x_1A;
		const float x_29 = v_0B * (x_25 + x_27) + v_0C * x_26;
		const float x_2A = v_0C * (x_25 - x_27);
		const float x_2B = v_0B * (x_25 + x_27) - v_0C * x_26;
		const float x_2C = v_06 * x_28;
		const float x_1B = v_03 * (x_2A - x_2C);
		const float x_1C = v_03 * (x_2A + x_2C);
		const float x_2D = v_10 * x_0C;
		const float x_2E = x_0B + x_0D;
		const float x_2F = x_0A + x_0E;
		const float x_30 = x_09 + x_0F;
		const float x_31 = x_09 - x_0F;
		const float x_32 = x_0A - x_0E;
		const float x_33 = x_0B - x_0D;
		const float x_37 = v_10 * x_2D;
		const float x_38 = v_07 * x_2E + v_08 * x_30;
		const float x_39 = v_10 * x_2F;
		const float x_3A = -v_08 * x_2E + v_07 * x_30;
		const float x_3B = v_0B * (x_37 + x_39) + v_0C * x_38;
		const float x_3C = v_0C * (x_37 - x_39);
		const float x_3D = v_0B * (x_37 + x_39) - v_0C * x_38;
		const float x_3E = v_06 * x_3A;
		const float x_34 = v_03 * (x_3C - x_3E);
		const float x_35 = v_03 * (x_3C + x_3E);
		const float x_3F = v_10 * x_32;
		const float x_40 = x_31 + x_33;
		const float x_41 = x_31 - x_33;
		const float x_42 = v_0C * (x_3F + x_40);
		const float x_43 = v_0C * (x_3F - x_40);
		const float x_44 = v_06 * x_41;
		const float x_36 = -x_43;
		const float x_10 = -x_34;
		const float x_11 = -x_3D;

		out[ 0 * stridea] = v_0B * (x_1D + x_1F) + v_0C * x_1E;
		out[ 1 * stridea] = v_03 * (x_29 - x_11);
		out[ 2 * stridea] = v_03 * (x_29 + x_11);
		out[ 3 * stridea] = v_03 * (x_23 + x_36);
		out[ 4 * stridea] = v_03 * (x_23 - x_36);
		out[ 5 * stridea] = v_03 * (x_1B - x_35);
		out[ 6 * stridea] = v_03 * (x_1B + x_35);
		out[ 7 * stridea] = v_03 * (x_22 + x_44);
		out[ 8 * stridea] = v_03 * (x_22 - x_44);
		out[ 9 * stridea] = v_03 * (x_1C - x_10);
		out[10 * stridea] = v_03 * (x_1C + x_10);
		out[11 * stridea] = v_03 * (x_24 + x_42);
		out[12 * stridea] = v_03 * (x_24 - x_42);
		out[13 * stridea] = v_03 * (x_2B - x_3B);
		out[14 * stridea] = v_03 * (x_2B + x_3B);
		out[15 * stridea] = v_0B * (x_1D + x_1F) - v_0C * x_1E;

		out += strideb;
		src += strideb;
	}
}

void idct_16x16(const float* src, float *out)
{
	float* tmp = (float*)calloc(16 * 16, sizeof(float));
	idct_1d_16x16(src, tmp, 1, 16);
	idct_1d_16x16(tmp, out, 16, 1);
	free(tmp);
}