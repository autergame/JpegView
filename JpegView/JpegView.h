//author https://github.com/autergame
#pragma once

#include "generated_dct/dct2.h"
#include "generated_dct/dct4.h"
#include "generated_dct/dct8.h"
#include "generated_dct/dct16.h"
#include "generated_dct/dct32.h"
#include "generated_dct/dct64.h"
#include "generated_dct/dct128.h"
#include "generated_dct/dct256.h"

typedef void (*function_dct)(float*, const float*);

function_dct functions_fdct[] = {
	&fdct_2x2,
	&fdct_4x4,
	&fdct_8x8,
	&fdct_16x16,
	&fdct_32x32,
	&fdct_64x64,
	&fdct_128x128,
	&fdct_256x256
};

function_dct functions_idct[] = {
	&idct_2x2,
	&idct_4x4,
	&idct_8x8,
	&idct_16x16,
	&idct_32x32,
	&idct_64x64,
	&idct_128x128,
	&idct_256x256
};

template <class T>
void deletemod(T** ptr)
{
	if (*ptr != nullptr)
	{
		free(*ptr);
		*ptr = nullptr;
	}
}

uint8_t minmaxcolor(float color)
{
	if (color > 255.f)
		return 255;
	else if (color < 0.f)
		return 0;
	return (uint8_t)color;
}

struct JpegView
{
	int width, height;
	uint8_t* original_image;
	uint8_t* final_image;
	uint8_t** image_converted;
	int mwidth, mheight, block_size;
	int subsampling_index;
	bool compression_rate;
	int quality_start;
	bool useycbcr;
};

const ImVec4 tint_col = ImVec4(1.f, 1.f, 1.f, 1.f);
const ImVec4 border_col = ImVec4(.5f, .5f, .5f, 1.f);

int zoom_layer(GLuint image_texture, JpegView* jpeg, float& zoom, float magnifier_size, int width, int height)
{
	if (ImGui::IsItemHovered())
	{
		if (GImGui->IO.MouseWheel > 0.f)
			zoom *= 1.1f;
		else if (GImGui->IO.MouseWheel < 0.f)
			zoom *= (1.f / 1.1f);
		if (zoom < 1.f)
			zoom = 1.f;

		ImVec2 cursor = GImGui->IO.MousePos;
		ImRect last_rect = GImGui->LastItemData.Rect;
		ImVec2 last_rect_size = last_rect.GetSize();

		float half_magnifier = magnifier_size / 2.f;
		float magnifier_zoom = half_magnifier / zoom;

		float last_rect_size_fixed_x = (last_rect.Max.x - 1.f) - (last_rect.Min.x + 1.f);
		float last_rect_size_fixed_y = (last_rect.Max.y - 1.f) - (last_rect.Min.y + 1.f);
		float last_rect_fixed_x = last_rect_size_fixed_x / jpeg->width;
		float last_rect_fixed_y = last_rect_size_fixed_y / jpeg->height;

		float center_x = jpeg->width * ((cursor.x - last_rect.Min.x) / last_rect_size_fixed_x);
		float center_y = jpeg->height * ((cursor.y - last_rect.Min.y) / last_rect_size_fixed_y);
		float uv0_x = (center_x - (magnifier_zoom / last_rect_fixed_x)) / jpeg->width;
		float uv0_y = (center_y - (magnifier_zoom / last_rect_fixed_y)) / jpeg->height;
		float uv1_x = (center_x + (magnifier_zoom / last_rect_fixed_x)) / jpeg->width;
		float uv1_y = (center_y + (magnifier_zoom / last_rect_fixed_y)) / jpeg->height;

		float cursor_box_pos_x = cursor.x - half_magnifier;
		float cursor_box_pos_y = cursor.y - half_magnifier;
		if (cursor_box_pos_x < 0.f)
			cursor_box_pos_x = 0.f;
		if (cursor_box_pos_y < 0.f)
			cursor_box_pos_y = 0.f;
		if (cursor.x + half_magnifier > width)
			cursor_box_pos_x = width - magnifier_size;
		if (cursor.y + half_magnifier > height)
			cursor_box_pos_y = height - magnifier_size;

		ImGui::SetNextWindowPos(ImVec2(cursor_box_pos_x, cursor_box_pos_y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::BeginTooltip();
		ImGui::Image((void*)(intptr_t)image_texture, ImVec2(magnifier_size, magnifier_size),
			ImVec2(uv0_x, uv0_y), ImVec2(uv1_x, uv1_y), tint_col, border_col);
		ImGui::EndTooltip();
		ImGui::PopStyleVar();
		
		return 1;
	}

	return 0;
}

GLuint create_image(uint8_t* image, int width, int height, bool linear)
{
	const GLfloat color[] = { .2f, .2f, .2f, 1.f };
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	return image_texture;
}

const float qMatrix_luma_const[64] = {
	16.f, 11.f, 10.f, 16.f,  24.f,  40.f,  51.f,  61.f,
	12.f, 12.f, 14.f, 19.f,  26.f,  58.f,  60.f,  55.f,
	14.f, 13.f, 16.f, 24.f,  40.f,  57.f,  69.f,  56.f,
	14.f, 17.f, 22.f, 29.f,  51.f,  87.f,  80.f,  62.f,
	18.f, 22.f, 37.f, 56.f,  68.f, 109.f, 103.f,  77.f,
	24.f, 35.f, 55.f, 64.f,  81.f, 104.f, 113.f,  92.f,
	49.f, 64.f, 78.f, 87.f, 103.f, 121.f, 120.f, 101.f,
	72.f, 92.f, 95.f, 98.f, 112.f, 100.f, 103.f,  99.f
};
const float qMatrix_chroma_const[64] = {
	17.f, 18.f, 24.f, 47.f, 99.f, 99.f, 99.f, 99.f,
	18.f, 21.f, 26.f, 66.f, 99.f, 99.f, 99.f, 99.f,
	24.f, 26.f, 56.f, 99.f, 99.f, 99.f, 99.f, 99.f,
	47.f, 66.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f,
	99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f,
	99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f,
	99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f,
	99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f, 99.f
};

float* generate_QMatrix_nofactor(const float* qMatrix_base, int block_size, bool qtablege)
{
	float* qMatrix = new float[block_size * block_size]{};
	if (qtablege)
	{
		for (int y = 0; y < block_size; y++)
		{
			for (int x = 0; x < block_size; x++)
			{
				qMatrix[y * block_size + x] = 1.f + x + y;
			}
		}
		return qMatrix;
	}
	else
	{
		int const_block_size = block_size < 8 ? block_size : 8;
		for (int y = 0; y < const_block_size; y++)
		{
			for (int x = 0; x < const_block_size; x++)
			{
				qMatrix[y * block_size + x] = qMatrix_base[y * 8 + x];
			}
		}
		if (block_size > 8)
		{
			for (int y = 0; y < block_size; y++)
			{
				for (int x = 8; x < block_size; x++)
				{
					qMatrix[y * block_size + x] = 1.f + x + y;
				}
			}
			for (int y = 8; y < block_size; y++)
			{
				for (int x = 0; x < block_size; x++)
				{
					qMatrix[y * block_size + x] = 1.f + x + y;
				}
			}
		}
	}
	return qMatrix;
}

float* generate_QMatrix(const float* qMatrix_base, int block_size, float factor, bool qtablege)
{
	float* qMatrix = new float[block_size * block_size]{};
	if (qtablege)
	{
		for (int y = 0; y < block_size; y++)
		{
			for (int x = 0; x < block_size; x++)
			{
				qMatrix[y * block_size + x] = 1.f + (1.f + x + y) * factor;
			}
		}
		return qMatrix;
	}
	else
	{
		int const_block_size = block_size < 8 ? block_size : 8;
		for (int y = 0; y < const_block_size; y++)
		{
			for (int x = 0; x < const_block_size; x++)
			{
				qMatrix[y * block_size + x] = qMatrix_base[y * 8 + x];
			}
		}
		if (block_size > 8)
		{
			for (int y = 0; y < block_size; y++)
			{
				for (int x = 8; x < block_size; x++)
				{
					qMatrix[y * block_size + x] = 1.f + (1.f + x + y) * factor;
				}
			}
			for (int y = 8; y < block_size; y++)
			{
				for (int x = 0; x < block_size; x++)
				{
					qMatrix[y * block_size + x] = 1.f + (1.f + x + y) * factor;
				}
			}
		}
	}
	return qMatrix;
}

float* generate_DCT_table(int block_size)
{
	float* DCTTable = new float[block_size * block_size]{};
	for (int y = 0; y < block_size; y++)
	{
		for (int x = 0; x < block_size; x++)
		{
			DCTTable[y * block_size + x] = cosf((2.f * y + 1.f) * x * 3.141592f / (2.f * block_size));
		}
	}
	return DCTTable;
}

float* generate_Alpha_table(int block_size)
{
	float sqrt1_2 = 1.f / sqrtf(2.f);
	float* alphaTable = new float[block_size * block_size]{};
	for (int y = 0; y < block_size; y++)
	{
		for (int x = 0; x < block_size; x++)
		{
			alphaTable[y * block_size + x] = (y == 0 ? sqrt1_2 : 1.f) * (x == 0 ? sqrt1_2 : 1.f);
		}
	}
	return alphaTable;
}

int* generate_ZigZag_table(int block_size)
{
	int* ZigZagtable = new int[block_size * block_size]{};

	int j = 0, n = 0, index = 0;;
	for (int i = 0; i < block_size * 2; i++)
	{
		if (i < block_size)
			j = 0;
		else
			j = i - block_size + 1;
		while(j <= i && j < block_size)
		{
			if (i & 1)
				index = j * (block_size - 1) + i;
			else
				index = (i - j) * block_size + j;
			ZigZagtable[index] = n++;
			j++;
		}
	}

	return ZigZagtable;
}

struct jpeg_steps_struct
{
	float* image_block;
	float* DCTMatrix;
	float* DCTTable;
	float* alphaTable;
	int* ZigZagtable;
	int block_size;
	float two_block;
	int mwidth;
	int start_x;
	int start_y;
	int block_size_index;
	bool usefastdct;
	bool compression_rate;
	int quality_start;
	float Q_control;
};

int16_t* ZigZag_function(jpeg_steps_struct* jss)
{
	int16_t* DCTMatrix_zigzag = new int16_t[jss->block_size * jss->block_size]{};
	for (int y = 0; y < jss->block_size; y++)
	{
		for (int x = 0; x < jss->block_size; x++)
		{
			int index = y * jss->block_size + x;
			DCTMatrix_zigzag[jss->ZigZagtable[index]] = (int16_t)jss->DCTMatrix[index];
		}
	}
	return DCTMatrix_zigzag;
}

void DeZigZag_function(jpeg_steps_struct* jss, int16_t* DCTMatrix_zigzag)
{
	for (int y = 0; y < jss->block_size; y++)
	{
		for (int x = 0; x < jss->block_size; x++)
		{
			int index = y * jss->block_size + x;
			jss->DCTMatrix[index] = (float)DCTMatrix_zigzag[jss->ZigZagtable[index]];
		}
	}
}

void DCT_function(jpeg_steps_struct* jss, uint8_t* image_converted)
{
	if (jss->usefastdct)
	{
		for (int y = 0; y < jss->block_size; y++)
		{
			for (int x = 0; x < jss->block_size; x++)
			{
				int index = y * jss->block_size + x;
				int indeximage = (jss->start_y + y) * jss->mwidth + (jss->start_x + x);
				jss->image_block[index] = (image_converted[indeximage] - 128.f);
			}
		}
		functions_fdct[jss->block_size_index](jss->DCTMatrix, jss->image_block);
	}
	else
	{
		for (int v = 0; v < jss->block_size; v++)
		{
			for (int u = 0; u < jss->block_size; u++)
			{
				float sum = 0.0f;
				for (int y = 0; y < jss->block_size; y++)
				{
					for (int x = 0; x < jss->block_size; x++)
					{
						int index = (jss->start_y + y) * jss->mwidth + (jss->start_x + x);

						float xu = jss->DCTTable[x * jss->block_size + u];
						float yv = jss->DCTTable[y * jss->block_size + v];

						sum += (image_converted[index] - 128.f) * xu * yv;
					}
				}

				int index = v * jss->block_size + u;
				jss->DCTMatrix[index] = jss->alphaTable[index] * sum * jss->two_block;
			}
		}
	}
}

void inverse_DCT_function(jpeg_steps_struct* jss, uint8_t* result)
{
	if (jss->usefastdct)
	{
		functions_idct[jss->block_size_index](jss->image_block, jss->DCTMatrix);
		for (int y = 0; y < jss->block_size; y++)
		{
			for (int x = 0; x < jss->block_size; x++)
			{
				int index = y * jss->block_size + x;
				int indexresult = (jss->start_y + y) * jss->mwidth + (jss->start_x + x);
				result[indexresult] = minmaxcolor(jss->image_block[index] + 128.f);
			}
		}
	}
	else
	{
		for (int y = 0; y < jss->block_size; y++)
		{
			for (int x = 0; x < jss->block_size; x++)
			{
				float sum = 0.f;
				for (int v = 0; v < jss->block_size; v++)
				{
					for (int u = 0; u < jss->block_size; u++)
					{
						float xu = jss->DCTTable[x * jss->block_size + u];
						float yv = jss->DCTTable[y * jss->block_size + v];

						int index = v * jss->block_size + u;
						sum += jss->alphaTable[index] * jss->DCTMatrix[index] * xu * yv;
					}
				}

				int index = (jss->start_y + y) * jss->mwidth + (jss->start_x + x);
				result[index] = minmaxcolor((sum * jss->two_block) + 128.f);
			}
		}
	}
}

void Quantize_function(jpeg_steps_struct* jss, float* qMatrix)
{
	for (int y = 0; y < jss->block_size; y++)
	{
		for (int x = 0; x < jss->block_size; x++)
		{
			int index = y * jss->block_size + x;

			float qMatrix_value = 0;
			if (jss->compression_rate)
			{
				float factor = jss->quality_start + ((float)(jss->start_x + x) / (float)jss->mwidth) * jss->Q_control;
				if (factor >= 50.f && factor <= 99.f)
					factor = 200.f - factor * 2.f;
				else if (factor < 50.f)
					factor = 5000.f / factor;
				else if (factor > 99.f)
					factor = 1.f;

				qMatrix_value = 1.f + qMatrix[index] * factor;
			}
			else {
				qMatrix_value = qMatrix[index];
			}

			jss->DCTMatrix[index] = roundf(jss->DCTMatrix[index] / qMatrix_value);
		}
	}
}

void DeQuantize_function(jpeg_steps_struct* jss, float* qMatrix)
{
	for (int y = 0; y < jss->block_size; y++)
	{
		for (int x = 0; x < jss->block_size; x++)
		{
			int index = y * jss->block_size + x;

			float qMatrix_value = 0;
			if (jss->compression_rate)
			{
				float factor = jss->quality_start + ((float)(jss->start_x + x) / (float)jss->mwidth) * jss->Q_control;
				if (factor >= 50.f && factor <= 99.f)
					factor = 200.f - factor * 2.f;
				else if (factor < 50.f)
					factor = 5000.f / factor;
				else if (factor > 99.f)
					factor = 1.f;

				qMatrix_value = 1.f + qMatrix[index] * factor;
			}
			else {
				qMatrix_value = qMatrix[index];
			}

			jss->DCTMatrix[index] = jss->DCTMatrix[index] * qMatrix_value;
		}
	}
}

void JPEG_steps(jpeg_steps_struct* jss, uint8_t* result, uint8_t* image_converted, float* qMatrix)
{
	DCT_function(jss, image_converted);
	Quantize_function(jss, qMatrix);
	DeQuantize_function(jss, qMatrix);
	inverse_DCT_function(jss, result);
}

void Quad_JPEG_steps_decompress_load(jpeg_steps_struct* jss, uint8_t* result, float* qMatrix, int16_t* DCTMatrix_zigzag)
{
	DeZigZag_function(jss, DCTMatrix_zigzag);
	DeQuantize_function(jss, qMatrix);
	inverse_DCT_function(jss, result);
}

int16_t* Quad_JPEG_steps(jpeg_steps_struct* jss, uint8_t* result, uint8_t* image_converted, float* qMatrix)
{
	DCT_function(jss, image_converted);
	Quantize_function(jss, qMatrix);
	int16_t* DCTMatrix_zigzag = ZigZag_function(jss);
	DeQuantize_function(jss, qMatrix);
	inverse_DCT_function(jss, result);
	return DCTMatrix_zigzag;
}

struct jpeg_steps_struct_func
{
	jpeg_steps_struct* jss;
	uint8_t** result;
	uint8_t** image_converted;
	float* qMatrix_luma;
	float* qMatrix_chroma;
};

thread_pool_function(render_jpeg_func, arg_var)
{
	jpeg_steps_struct_func* jssf = (jpeg_steps_struct_func*)arg_var;

	jpeg_steps_struct* jss = jssf->jss;
	uint8_t** image_converted = jssf->image_converted;
	uint8_t** result = jssf->result;
	float* qMatrix_luma = jssf->qMatrix_luma;
	float* qMatrix_chroma = jssf->qMatrix_chroma;

	JPEG_steps(jss, result[0], image_converted[0], qMatrix_luma);
	JPEG_steps(jss, result[1], image_converted[1], qMatrix_chroma);
	JPEG_steps(jss, result[2], image_converted[2], qMatrix_chroma);

	deletemod(&jss->image_block);
	deletemod(&jss->DCTMatrix);

	deletemod(&jss);
	deletemod(&jssf);
}

uint8_t** Encode(uint8_t** image_converted, float* qMatrix_luma, float* qMatrix_chroma,
	int block_size, int block_size_index, int mwidth, int mheight, bool compression_rate,
	int quality_start, bool usethreads, bool usefastdct)
{
	float* DCTTable = nullptr, *alphaTable = nullptr;
	if (!usefastdct)
	{
		DCTTable = generate_DCT_table(block_size);
		alphaTable = generate_Alpha_table(block_size);
	}

	uint8_t** result = new uint8_t*[3]{};
	result[0] = new uint8_t[mheight * mwidth]{};
	result[1] = new uint8_t[mheight * mwidth]{};
	result[2] = new uint8_t[mheight * mwidth]{};

	int cpu_threads = 0;
	if (usethreads)
	{
		cpu_threads = get_cpu_threads();
		if (cpu_threads <= 1)
		{
			usethreads = false;
		}
	}

	if (usethreads)
	{
		thread_pool* threadpool = thread_pool_create(cpu_threads);

		for (int by = 0; by < mheight; by += block_size)
		{
			for (int bx = 0; bx < mwidth; bx += block_size)
			{
				jpeg_steps_struct* jss = new jpeg_steps_struct{};
				jss->image_block = new float[block_size * block_size]{};
				jss->DCTMatrix = new float[block_size * block_size]{};
				jss->mwidth = mwidth;
				jss->block_size = block_size;
				jss->block_size_index = block_size_index;
				jss->usefastdct = usefastdct;
				if (!usefastdct)
				{
					jss->DCTTable = DCTTable;
					jss->alphaTable = alphaTable;
					jss->two_block = 2.f / (float)block_size;
				}
				jss->compression_rate = compression_rate;
				jss->quality_start = quality_start;
				jss->Q_control = 100.f - quality_start;

				jss->start_x = bx;
				jss->start_y = by;

				jpeg_steps_struct_func* jssf = new jpeg_steps_struct_func{};
				jssf->jss = jss;
				jssf->result = result;
				jssf->qMatrix_chroma = qMatrix_chroma;
				jssf->qMatrix_luma = qMatrix_luma;
				jssf->image_converted = image_converted;

				thread_pool_add_work(threadpool, render_jpeg_func, jssf);
			}
		}

		thread_pool_destroy(threadpool);
	}
	else
	{
		float* image_block = new float[block_size * block_size]{};
		float* DCTMatrix = new float[block_size * block_size]{};

		jpeg_steps_struct* jss = new jpeg_steps_struct{};
		jss->image_block = image_block;
		jss->DCTMatrix = DCTMatrix;
		jss->mwidth = mwidth;
		jss->block_size = block_size;
		jss->block_size_index = block_size_index;
		jss->usefastdct = usefastdct;
		if (!usefastdct)
		{
			jss->DCTTable = DCTTable;
			jss->alphaTable = alphaTable;
			jss->two_block = 2.f / (float)block_size;
		}
		jss->compression_rate = compression_rate;
		jss->quality_start = quality_start;
		jss->Q_control = 100.f - quality_start;

		for (int by = 0; by < mheight; by += block_size)
		{
			for (int bx = 0; bx < mwidth; bx += block_size)
			{
				jss->start_x = bx;
				jss->start_y = by;

				JPEG_steps(jss, result[0], image_converted[0], qMatrix_luma);
				JPEG_steps(jss, result[1], image_converted[1], qMatrix_chroma);
				JPEG_steps(jss, result[2], image_converted[2], qMatrix_chroma);
			}
		}

		deletemod(&DCTMatrix);
		deletemod(&image_block);

		deletemod(&jss);
	}

	if (!usefastdct)
	{
		deletemod(&DCTTable);
		deletemod(&alphaTable);
	}
	return result;
}

void subsampling(uint8_t** image_converted, int width, int height, int mwidth, int subsampling_index, bool useycbcr)
{
	int startcomp = useycbcr ? 1 : 0;
	switch (subsampling_index)
	{
		case 0: // 4:4:4
		{
			break;
		}
		case 1: // 4:4:0
		{
			for (int i = startcomp; i < 3; i++)
			{
				for (int y = 0; y < height; y += 2)
				{
					for (int x = 0; x < width; x += 4)
					{
						int index = y * mwidth + x;
						int indexTwo = (y + 1) * mwidth + x;
						image_converted[i][indexTwo + 0] = image_converted[i][index + 0];
						image_converted[i][indexTwo + 1] = image_converted[i][index + 1];
						image_converted[i][indexTwo + 2] = image_converted[i][index + 2];
						image_converted[i][indexTwo + 3] = image_converted[i][index + 3];
					}
				}
			}
			break;
		}
		case 2: // 4:2:2
		{
			for (int i = startcomp; i < 3; i++)
			{
				for (int y = 0; y < height; y += 2)
				{
					for (int x = 0; x < width; x += 4)
					{
						int index = y * mwidth + x;
						image_converted[i][index + 1] = image_converted[i][index];
						image_converted[i][index + 3] = image_converted[i][index + 2];

						int indexTwo = (y + 1) * mwidth + x;
						image_converted[i][indexTwo + 1] = image_converted[i][indexTwo];
						image_converted[i][indexTwo + 3] = image_converted[i][indexTwo + 2];
					}
				}
			}
			break;
		}
		case 3: // 4:2:0
		{
			for (int i = startcomp; i < 3; i++)
			{
				for (int y = 0; y < height; y += 2)
				{
					for (int x = 0; x < width; x += 4)
					{
						int index = y * mwidth + x;
						image_converted[i][index + 1] = image_converted[i][index];
						image_converted[i][index + 3] = image_converted[i][index + 2];

						int indexTwo = (y + 1) * mwidth + x;
						image_converted[i][indexTwo + 0] = image_converted[i][index];
						image_converted[i][indexTwo + 1] = image_converted[i][index];
						image_converted[i][indexTwo + 2] = image_converted[i][index + 2];
						image_converted[i][indexTwo + 3] = image_converted[i][index + 2];
					}
				}
			}
			break;
		}
		case 4: // 4:1:1
		{
			for (int i = startcomp; i < 3; i++)
			{
				for (int y = 0; y < height; y += 2)
				{
					for (int x = 0; x < width; x += 4)
					{
						int index = y * mwidth + x;
						image_converted[i][index + 1] = image_converted[i][index];
						image_converted[i][index + 2] = image_converted[i][index];
						image_converted[i][index + 3] = image_converted[i][index];

						int indexTwo = (y + 1) * mwidth + x;
						image_converted[i][indexTwo + 1] = image_converted[i][indexTwo];
						image_converted[i][indexTwo + 2] = image_converted[i][indexTwo];
						image_converted[i][indexTwo + 3] = image_converted[i][indexTwo];
					}
				}
			}
			break;
		}
		case 5: // 4:1:0
		{
			for (int i = startcomp; i < 3; i++)
			{
				for (int y = 0; y < height; y += 2)
				{
					for (int x = 0; x < width; x += 4)
					{
						int index = y * mwidth + x;
						image_converted[i][index + 1] = image_converted[i][index];
						image_converted[i][index + 2] = image_converted[i][index];
						image_converted[i][index + 3] = image_converted[i][index];

						int indexTwo = (y + 1) * mwidth + x;
						image_converted[i][indexTwo + 0] = image_converted[i][index];
						image_converted[i][indexTwo + 1] = image_converted[i][index];
						image_converted[i][indexTwo + 2] = image_converted[i][index];
						image_converted[i][indexTwo + 3] = image_converted[i][index];
					}
				}
			}
			break;
		}
	}
}

void fill_outbound(uint8_t** image_converted, int width, int height, int mwidth, int mheight)
{
	for (int y = 0; y < mheight; y++)
	{
		for (int x = width; x < mwidth; x++)
		{
			int indexresult = y * mwidth + x;
			image_converted[0][indexresult] = 0x80;
			image_converted[1][indexresult] = 0x80;
			image_converted[2][indexresult] = 0x80;
		}
	}
	for (int y = height; y < mheight; y++)
	{
		for (int x = 0; x < mwidth; x++)
		{
			int indexresult = y * mwidth + x;
			image_converted[0][indexresult] = 0x80;
			image_converted[1][indexresult] = 0x80;
			image_converted[2][indexresult] = 0x80;
		}
	}
}

uint8_t** image_to_matrix_YCbCr(uint8_t* original_image,
	int width, int height, int mwidth, int mheight, int subsampling_index)
{
	uint8_t** YCbCr = new uint8_t*[3]{};
	YCbCr[0] = new uint8_t[mheight * mwidth]{};
	YCbCr[1] = new uint8_t[mheight * mwidth]{};
	YCbCr[2] = new uint8_t[mheight * mwidth]{};
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int indexYCbCr = y * mwidth + x;
			int indexoriginal = (y * width + x) * 3;

			uint8_t r = original_image[indexoriginal + 0];
			uint8_t g = original_image[indexoriginal + 1];
			uint8_t b = original_image[indexoriginal + 2];

			YCbCr[0][indexYCbCr] = minmaxcolor((( 0.299f * r) + ( 0.587f * g) + ( 0.114f * b)));
			YCbCr[1][indexYCbCr] = minmaxcolor(((-0.168f * r) + (-0.331f * g) + ( 0.500f * b)) + 128);
			YCbCr[2][indexYCbCr] = minmaxcolor((( 0.500f * r) + (-0.418f * g) + (-0.081f * b)) + 128);
		}
	}
	subsampling(YCbCr, width, height, mwidth, subsampling_index, true);
	fill_outbound(YCbCr, width, height, mwidth, mheight);
	return YCbCr;
}

uint8_t* YCbCr_matrix_to_image(uint8_t** YCbCr, int width, int height, int mwidth)
{
	uint8_t* result = new uint8_t[height * width * 3]{};
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int indexYCbCr = y * mwidth + x;
			int indexresult = (y * width + x) * 3;

			uint8_t Y = YCbCr[0][indexYCbCr];
			int8_t Cb = YCbCr[1][indexYCbCr] - 128;
			int8_t Cr = YCbCr[2][indexYCbCr] - 128;

			result[indexresult + 0] = minmaxcolor(Y + ( 1.402f * Cr));
			result[indexresult + 1] = minmaxcolor(Y + (-0.344f * Cb) + (-0.714f * Cr));
			result[indexresult + 2] = minmaxcolor(Y + ( 1.772f * Cb));
		}
	}
	return result;
}

uint8_t** image_to_matrix_RGB(uint8_t* original_image,
	int width, int height, int mwidth, int mheight, int subsampling_index)
{
	uint8_t** RGB = new uint8_t * [3]{};
	RGB[0] = new uint8_t[mheight * mwidth]{};
	RGB[1] = new uint8_t[mheight * mwidth]{};
	RGB[2] = new uint8_t[mheight * mwidth]{};
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int indexRGB = y * mwidth + x;
			int indexoriginal = (y * width + x) * 3;

			RGB[0][indexRGB] = original_image[indexoriginal + 0];
			RGB[1][indexRGB] = original_image[indexoriginal + 1];
			RGB[2][indexRGB] = original_image[indexoriginal + 2];
		}
	}
	subsampling(RGB, width, height, mwidth, subsampling_index, false);
	fill_outbound(RGB, width, height, mwidth, mheight);
	return RGB;
}

uint8_t* RGB_matrix_to_image(uint8_t** RGB, int width, int height, int mwidth)
{
	uint8_t* result = new uint8_t[height * width * 3]{};
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int indexRGB = y * mwidth + x;
			int indexresult = (y * width + x) * 3;

			result[indexresult + 0] = RGB[0][indexRGB];
			result[indexresult + 1] = RGB[1][indexRGB];
			result[indexresult + 2] = RGB[2][indexRGB];
		}
	}
	return result;
}

int round_up_block_size(int x, int block_size)
{
	x = (x + (block_size - 1));
	return (x - (x % block_size));
}

inline int log2i(int x)
{
	int log2Val = 0;
	while (x >>= 1) 
		log2Val++;
	return log2Val;
}

JpegView* init_jpeg(uint8_t* original, int width, int height, int block_size, bool useycbcr)
{
	JpegView* jpeg = new JpegView{};
	jpeg->width = width;
	jpeg->height = height;
	jpeg->block_size = block_size;
	jpeg->compression_rate = false;
	jpeg->quality_start = 1;
	jpeg->useycbcr = useycbcr;

	jpeg->original_image = original;
	jpeg->final_image = nullptr;

	jpeg->mwidth = round_up_block_size(width, block_size);
	jpeg->mheight = round_up_block_size(height, block_size);

	if (useycbcr)
	{
		jpeg->image_converted = image_to_matrix_YCbCr(jpeg->original_image,
			jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight, 0);
		jpeg->final_image = YCbCr_matrix_to_image(jpeg->image_converted, jpeg->width, jpeg->height, jpeg->mwidth);
	}
	else 
	{
		jpeg->image_converted = image_to_matrix_RGB(jpeg->original_image,
			jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight, 0);
		jpeg->final_image = RGB_matrix_to_image(jpeg->image_converted, jpeg->width, jpeg->height, jpeg->mwidth);
	}

	return jpeg;
}

void render_ycbcr(JpegView* jpeg, int block_size, int subsampling_index)
{
	for (int i = 0; i < 3; i++)
		deletemod(&jpeg->image_converted[i]);
	deletemod(&jpeg->image_converted);

	jpeg->block_size = block_size;
	jpeg->subsampling_index = subsampling_index;

	jpeg->mwidth = round_up_block_size(jpeg->width, block_size);
	jpeg->mheight = round_up_block_size(jpeg->height, block_size);

	jpeg->image_converted = image_to_matrix_YCbCr(jpeg->original_image,
		jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight, subsampling_index);
}

void render_rgb(JpegView* jpeg, int block_size, int subsampling_index) 
{
	for (int i = 0; i < 3; i++)
		deletemod(&jpeg->image_converted[i]);
	deletemod(&jpeg->image_converted);

	jpeg->block_size = block_size;
	jpeg->subsampling_index = subsampling_index;

	jpeg->mwidth = round_up_block_size(jpeg->width, block_size);
	jpeg->mheight = round_up_block_size(jpeg->height, block_size);

	jpeg->image_converted = image_to_matrix_RGB(jpeg->original_image,
		jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight, subsampling_index);
}

void render_jpeg(JpegView* jpeg, int block_size, int quality, bool qtablege,
	int subsampling_index, int block_size_index, bool useycbcr, bool usethreads, bool usefastdct)
{
	if (jpeg->block_size != block_size || jpeg->subsampling_index != subsampling_index || jpeg->useycbcr != useycbcr)
	{
		if (useycbcr)
			render_ycbcr(jpeg, block_size, subsampling_index);
		else
			render_rgb(jpeg, block_size, subsampling_index);
		jpeg->useycbcr = useycbcr;
	}

	float* qMatrix_luma, *qMatrix_chroma;
	if (!jpeg->compression_rate)
	{
		float factor = (float)quality;
		if (factor >= 50.f && factor <= 99.f)
			factor = 200.f - factor * 2.f;
		else if (factor < 50.f)
			factor = 5000.f / factor;
		else if (factor == 100.f)
			factor = 1.f;
		qMatrix_luma = generate_QMatrix(qMatrix_luma_const, jpeg->block_size, factor, qtablege);
		qMatrix_chroma = generate_QMatrix(qMatrix_chroma_const, jpeg->block_size, factor, qtablege);
	}
	else {
		qMatrix_luma = generate_QMatrix_nofactor(qMatrix_luma_const, jpeg->block_size, qtablege);
		qMatrix_chroma = generate_QMatrix_nofactor(qMatrix_chroma_const, jpeg->block_size, qtablege);
	}


	uint8_t** image_convertedmodified = Encode(jpeg->image_converted, qMatrix_luma, qMatrix_chroma,
		jpeg->block_size, block_size_index, jpeg->mwidth, jpeg->mheight, jpeg->compression_rate,
		jpeg->quality_start, usethreads, usefastdct);

	deletemod(&jpeg->final_image);
	if (useycbcr)
		jpeg->final_image = YCbCr_matrix_to_image(image_convertedmodified, jpeg->width, jpeg->height, jpeg->mwidth);
	else
		jpeg->final_image = RGB_matrix_to_image(image_convertedmodified, jpeg->width, jpeg->height, jpeg->mwidth);

	deletemod(&qMatrix_luma);
	deletemod(&qMatrix_chroma);
	for (int i = 0; i < 3; i++)
		deletemod(&image_convertedmodified[i]);
	deletemod(&image_convertedmodified);
}

void image_to_opengl(JpegView* jpeg, GLuint image_texturef, GLuint image_texturef_zoom)
{
	glBindTexture(GL_TEXTURE_2D, image_texturef);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);

	glBindTexture(GL_TEXTURE_2D, image_texturef_zoom);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);
}