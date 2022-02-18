//author https://github.com/autergame
#pragma once

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glfw3")

#pragma warning(push, 0)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#define __STDC_LIB_EXT1__
#include "stb_image_write.h"
#undef __STDC_LIB_EXT1__

#include <windows.h>

#include <glad/glad.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#pragma warning(pop)

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <algorithm>

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
    uint8_t** YCbCr;
    int mwidth, mheight, block_size;
    bool compression_rate;
    int quality_start;
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
    if (qtablege == true)
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
    return qMatrix;
}

float* generate_QMatrix(const float* qMatrix_base, int block_size, float factor, bool qtablege)
{
    float* qMatrix = new float[block_size * block_size]{};
    if (qtablege == true)
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
    return qMatrix;
}

float* generate_DCTtable(int block_size)
{
    float* table = new float[block_size * block_size]{};
    for (int y = 0; y < block_size; y++)
    {
        for (int x = 0; x < block_size; x++)
        {
            table[y * block_size + x] = cosf((2.f * y + 1.f) * x * 3.141592f / (2.f * block_size));
        }
    }
    return table;
}

float* generate_Alphatable(int block_size)
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

struct jpeg_steps_struct
{
    float* DCTMatrix;
    float* DCTTable;
    float* alphaTable;
    int block_size;
    int dctalpha_size;
    int mwidth;
    float block;
    int start_x;
    int start_y;
    bool compression_rate;
    int quality_start;
    float Q_control;
};

void DCT_function(jpeg_steps_struct* jss, uint8_t* YCbCr)
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

                    sum += (YCbCr[index] - 128.f) * xu * yv;
                }
            }

            int index = v * jss->dctalpha_size + u;
            jss->DCTMatrix[index] = jss->alphaTable[index] * sum * jss->block;
        }
    }
}

void Quantize_function(jpeg_steps_struct* jss, float* qMatrix)
{
    for (int y = 0; y < jss->block_size; y++)
    {
        for (int x = 0; x < jss->block_size; x++)
        {
            int index = y * jss->dctalpha_size + x;

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

            jss->DCTMatrix[index] = roundf(jss->DCTMatrix[index] / qMatrix_value) * qMatrix_value;
        }
    }
}

void inverse_DCT_function(jpeg_steps_struct* jss, uint8_t* result)
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

                    int index = v * jss->dctalpha_size + u;
                    sum += jss->alphaTable[index] * jss->DCTMatrix[index] * xu * yv;
                }
            }

            int index = (jss->start_y + y) * jss->mwidth + (jss->start_x + x);
            result[index] = minmaxcolor((sum * jss->block) + 128.f);
        }
    }
}

void JPEG_steps(jpeg_steps_struct* jss, uint8_t* result, uint8_t* YCbCr, float* qMatrix)
{
    DCT_function(jss, YCbCr);
    Quantize_function(jss, qMatrix);
    inverse_DCT_function(jss, result);
}

uint8_t** Encode(uint8_t** YCbCr, float* qMatrix_luma, float* qMatrix_chroma,
    int block_size, int mwidth, int mheight, bool compression_rate, int quality_start)
{
    float block = 2.f / (float)block_size;

    float* DCTMatrix = new float[block_size * block_size]{};
    float* DCTTable = generate_DCTtable(block_size);
    float* alphaTable = generate_Alphatable(block_size);

    jpeg_steps_struct* jss = new jpeg_steps_struct{};
    jss->DCTMatrix = DCTMatrix;
    jss->DCTTable = DCTTable;
    jss->alphaTable = alphaTable;
    jss->dctalpha_size = block_size;
    jss->mwidth = mwidth;
    jss->compression_rate = compression_rate;
    jss->quality_start = quality_start;
    jss->Q_control = 100.f - quality_start;
    jss->block_size = block_size;
    jss->block = 2.f / (float)block_size;

    uint8_t** result = new uint8_t*[3]{};
    result[0] = new uint8_t[mheight * mwidth]{};
    result[1] = new uint8_t[mheight * mwidth]{};
    result[2] = new uint8_t[mheight * mwidth]{};

    for (int by = 0; by < mheight; by += block_size)
    {
        for (int bx = 0; bx < mwidth; bx += block_size)
        {
            jss->start_x = bx;
            jss->start_y = by;

            JPEG_steps(jss, result[0], YCbCr[0], qMatrix_luma);
            JPEG_steps(jss, result[1], YCbCr[1], qMatrix_chroma);
            JPEG_steps(jss, result[2], YCbCr[2], qMatrix_chroma);
        }
    }

    deletemod(&jss);
    deletemod(&DCTTable);
    deletemod(&alphaTable);
    deletemod(&DCTMatrix);
    return result;
}

uint8_t** image_to_matrix(uint8_t* original_image, int width, int height, int mwidth, int mheight)
{
    uint8_t** result = new uint8_t*[3]{};
    result[0] = new uint8_t[mheight * mwidth]{};
    result[1] = new uint8_t[mheight * mwidth]{};
    result[2] = new uint8_t[mheight * mwidth]{};
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int indexresult = y * mwidth + x;
            int indexoriginal = (y * width + x) * 3;

            uint8_t r = original_image[indexoriginal + 0];
            uint8_t g = original_image[indexoriginal + 1];
            uint8_t b = original_image[indexoriginal + 2];

            result[0][indexresult] = minmaxcolor((( 0.299f * r) + ( 0.587f * g) + ( 0.114f * b)));
            result[1][indexresult] = minmaxcolor(((-0.168f * r) + (-0.331f * g) + ( 0.500f * b)) + 128);
            result[2][indexresult] = minmaxcolor((( 0.500f * r) + (-0.418f * g) + (-0.081f * b)) + 128);
        }
    }
    for (int y = 0; y < mheight; y++)
    {
        for (int x = width; x < mwidth; x++)
        {
            int indexresult = y * mwidth + x;
            result[0][indexresult] = 0x80;
            result[1][indexresult] = 0x80;
            result[2][indexresult] = 0x80;
        }
    }
    for (int y = height; y < mheight; y++)
    {
        for (int x = 0; x < mwidth; x++)
        {
            int indexresult = y * mwidth + x;
            result[0][indexresult] = 0x80;
            result[1][indexresult] = 0x80;
            result[2][indexresult] = 0x80;
        }
    }
    return result;
}

uint8_t* matrix_to_image(uint8_t** YCbCr, int width, int height, int mwidth)
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

            result[indexresult + 0] = minmaxcolor(Y + (1.402f * Cr));
            result[indexresult + 1] = minmaxcolor(Y + (-0.344f * Cb) + (-0.714f * Cr));
            result[indexresult + 2] = minmaxcolor(Y + (1.772f * Cb));
        }
    }
    return result;
}

void compress(JpegView* jpeg, float factor, bool qtablege)
{
    deletemod(&jpeg->final_image);

    float* qMatrix_luma, *qMatrix_chroma;
    if (!jpeg->compression_rate)
    {
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

    uint8_t** YCbCrmodified = Encode(jpeg->YCbCr, qMatrix_luma, qMatrix_chroma,
        jpeg->block_size, jpeg->mwidth, jpeg->mheight, jpeg->compression_rate, jpeg->quality_start);
    
    jpeg->final_image = matrix_to_image(YCbCrmodified, jpeg->width, jpeg->height, jpeg->mwidth);

    deletemod(&qMatrix_luma);
    deletemod(&qMatrix_chroma);
    for (int i = 0; i < 3; i++)
        deletemod(&YCbCrmodified[i]);
    deletemod(&YCbCrmodified);
}

int round_up_block_size(int x, int block_size)
{
    x = (x + (block_size - 1));
    return (x - (x % block_size));
}

JpegView* init_jpeg(uint8_t* original, int width, int height, int block_size)
{
    JpegView* jpeg = new JpegView{};
    jpeg->width = width;
    jpeg->height = height;
    jpeg->block_size = block_size;
    jpeg->compression_rate = false;
    jpeg->quality_start = 1;

    jpeg->original_image = original;
    jpeg->final_image = nullptr;

    jpeg->mwidth = round_up_block_size(width, block_size);
    jpeg->mheight = round_up_block_size(height, block_size);

    jpeg->YCbCr = image_to_matrix(jpeg->original_image, jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight);

    jpeg->final_image = matrix_to_image(jpeg->YCbCr, jpeg->width, jpeg->height, jpeg->mwidth);

    return jpeg;
}

void render_jpeg(JpegView* jpeg, int block_size, int quality, bool qtablege)
{
    if (jpeg->block_size != block_size)
    {
        for (int i = 0; i < 3; i++)
            deletemod(&jpeg->YCbCr[i]);
        deletemod(&jpeg->YCbCr);

        jpeg->block_size = block_size;

        jpeg->mwidth = round_up_block_size(jpeg->width, block_size);
        jpeg->mheight = round_up_block_size(jpeg->height, block_size);

        jpeg->YCbCr = image_to_matrix(jpeg->original_image, jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight);
    }

    compress(jpeg, (float)quality, qtablege);
}

void image_to_opengl(JpegView* jpeg, GLuint image_texturef, GLuint image_texturef_zoom)
{
    glBindTexture(GL_TEXTURE_2D, image_texturef);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);

    glBindTexture(GL_TEXTURE_2D, image_texturef_zoom);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);
}