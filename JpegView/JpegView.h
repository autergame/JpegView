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

float minmaxq(float color)
{
    if (color > 255.f)
        return 255.f;
    else if (color <= 0.f)
        return 1.f;
    return color;
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

float* generate_QMatrix_nofactor(int block_size)
{
    float* qMatrix = new float[block_size * block_size]{};
    for (int y = 0; y < block_size; y++)
    {
        for (int x = 0; x < block_size; x++)
        {
            qMatrix[y * block_size + x] = 1.f + x + y;
        }
    }
    return qMatrix;
}

float* generate_QMatrix(int block_size, float factor)
{
    float* qMatrix = new float[block_size * block_size]{};
    for (int y = 0; y < block_size; y++)
    {
        for (int x = 0; x < block_size; x++)
        {
            qMatrix[y * block_size + x] = 1.f + (1.f + x + y) * factor;
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
    float* alpha = new float[block_size * block_size]{};
    for (int y = 1; y < block_size; y++)
    {
        for (int x = 1; x < block_size; x++)
        {
            alpha[y * block_size + x] = 1.f;
        }
    }

    float sqrt1_2 = 1.f / sqrtf(2.f);

    for (int x = 1; x < block_size; x++)
        alpha[x] = sqrt1_2;

    for (int y = 1; y < block_size; y++)
        alpha[y * block_size] = sqrt1_2;

    alpha[0] = sqrt1_2 * sqrt1_2;

    return alpha;
}

void DCT_function(float* DCTMatrix, float* DCTTable, float* alpha,
    int block_size, int dctalpha_size, int width, float block, int start_x, int start_y, uint8_t* YCbCr)
{
    for (int k = 0; k < block_size * block_size; k++)
    {
        int u = (k % block_size);
        int v = (k / block_size);

        float sum = 0.0f;
        for (int l = 0; l < block_size * block_size; l++)
        {
            int x = (l % block_size);
            int y = (l / block_size);
            int index = (start_y + y) * width + (start_x + x);

            float xu = DCTTable[x * block_size + u];
            float yv = DCTTable[y * block_size + v];

            sum += (YCbCr[index] - 128.f) * xu * yv;
        }

        int index = v * dctalpha_size + u;
        DCTMatrix[index] = alpha[index] * sum * block;
    }
}

void Quantize_function(float* DCTMatrix, float* qMatrix,
    int block_size, int dctalpha_size, int start_x, int width, bool compression_rate, int quality_start, float Q_control)
{
    for (int k = 0; k < block_size * block_size; k++)
    {
        int x = (k % block_size);
        int y = (k / block_size);
        int index = y * dctalpha_size + x;

        float qMatrix_value = 0;
        if (compression_rate)
        {
            float factor = quality_start + ((float)(start_x + x) / (float)width) * Q_control;
            if (factor >= 50.f)
                factor = 200.f - factor * 2.f;
            else
                factor = 5000.f / factor;

            qMatrix_value = minmaxq(1.f + qMatrix[index] * factor);
        }
        else {
            qMatrix_value = minmaxq(qMatrix[index]);
        }

        DCTMatrix[index] = roundf(DCTMatrix[index] / qMatrix_value) * qMatrix_value;
    }
}

void inverse_DCT_function(float* DCTMatrix, float* DCTTable, float* alpha,
    int block_size, int dctalpha_size, int width, float block, int start_x, int start_y, uint8_t* result)
{
    for (int k = 0; k < block_size * block_size; k++)
    {
        int x = (k % block_size);
        int y = (k / block_size);

        float sum = 0.f;
        for (int l = 0; l < block_size * block_size; l++)
        {
            int u = (l % block_size);
            int v = (l / block_size);

            float xu = DCTTable[x * block_size + u];
            float yv = DCTTable[y * block_size + v];

            int index = v * dctalpha_size + u;
            sum += alpha[index] * DCTMatrix[index] * xu * yv;
        }

        int index = (start_y + y) * width + (start_x + x);
        result[index] = minmaxcolor((sum * block) + 128.f);
    }
}

void JPEG_steps(float* DCTMatrix, float* DCTTable, float* alpha, float* qMatrix,
    int block_size, int dctalpha_size, int width, float block, int start_x, int start_y,
    bool compression_rate, int quality_start, float Q_control, uint8_t* result, uint8_t* YCbCr)
{
    DCT_function(DCTMatrix, DCTTable, alpha,
        block_size, dctalpha_size, width, block, start_x, start_y, YCbCr);

    Quantize_function(DCTMatrix, qMatrix,
        block_size, dctalpha_size, start_x, width, compression_rate, quality_start, Q_control);

    inverse_DCT_function(DCTMatrix, DCTTable, alpha,
        block_size, dctalpha_size, width, block, start_x, start_y, result);
}

uint8_t** Encode(uint8_t** YCbCr, float* qMatrix,
    int block_size, int dctalpha_size, int width, int height,
    bool compression_rate, int quality_start, float Q_control)
{
    float block = 2.f / (float)block_size;
    int width_blocks = width / block_size;
    int height_blocks = height / block_size;

    float* alpha = generate_Alphatable(block_size);
    float* DCTTable = generate_DCTtable(block_size);
    float* DCTMatrix = new float[block_size * block_size]{};

    uint8_t** result = new uint8_t*[3]{};
    for (int i = 0; i < 3; i++)
    {
        result[i] = new uint8_t[height * width]{};

        for (int byx = 0; byx < height_blocks * width_blocks; byx++)
        {
            int bx = (byx % width_blocks) * block_size;
            int by = (byx / width_blocks) * block_size;

            JPEG_steps(DCTMatrix, DCTTable, alpha, qMatrix, block_size, block_size,
                width, block, bx, by, compression_rate, quality_start, Q_control, result[i], YCbCr[i]);
        }
    }

    deletemod(&alpha);
    deletemod(&DCTTable);
    deletemod(&DCTMatrix);
    return result;
}

uint8_t** image_to_matrix(uint8_t* original_image, int width, int height, int mwidth, int mheight)
{
    uint8_t** result = new uint8_t*[3]{};
    for (int i = 0; i < 3; i++) 
    {
        result[i] = new uint8_t[mheight * mwidth]{};
        if (width != mwidth)
        {
            for (int y = 0; y < mheight; y++)
            {
                for (int x = width; x < mwidth; x++)
                {
                    result[i][y * mwidth + x] = 0x80;
                }
            }
        }
        if (height != mheight)
        {
            for (int y = height; y < mheight; y++)
            {
                for (int x = 0; x < mwidth; x++)
                {
                    result[i][y * mwidth + x] = 0x80;
                }
            }
        }
    }
    for (int i = 0; i < height * width; i++)
    {
        int x = (i % width);
        int y = (i / width);
        int index = i * 3;
        int indexresult = y * mwidth + x;

        uint8_t r = original_image[index + 0];
        uint8_t g = original_image[index + 1];
        uint8_t b = original_image[index + 2];

        result[0][indexresult] = minmaxcolor((( 0.299f * r) + ( 0.587f * g) + ( 0.114f * b)));
        result[1][indexresult] = minmaxcolor(((-0.168f * r) + (-0.331f * g) + ( 0.500f * b)) + 128);
        result[2][indexresult] = minmaxcolor((( 0.500f * r) + (-0.418f * g) + (-0.081f * b)) + 128);
    }
    return result;
}

uint8_t* matrix_to_image(uint8_t** YCbCr, int width, int height, int mwidth)
{
    uint8_t* result = new uint8_t[height * width * 3]{};
    for (int i = 0; i < height * width; i++)
    {
        int x = (i % width);
        int y = (i / width);
        int index = i * 3;
        int indexYCbCr = y * mwidth + x;

        uint8_t Y = YCbCr[0][indexYCbCr];
        int8_t Cb = YCbCr[1][indexYCbCr] - 128;
        int8_t Cr = YCbCr[2][indexYCbCr] - 128;

        result[index + 0] = minmaxcolor(Y + ( 1.402f * Cr));
        result[index + 1] = minmaxcolor(Y + (-0.344f * Cb) + (-0.714f * Cr));
        result[index + 2] = minmaxcolor(Y + ( 1.772f * Cb));
    }
    return result;
}

void compress(JpegView* jpeg, float factor)
{
    deletemod(&jpeg->final_image);

    float* qMatrix, Q_control = 0.f;
    if (!jpeg->compression_rate)
    {
        if (factor >= 50.f)
            factor = 200.f - factor * 2.f;
        else
            factor = 5000.f / factor;
        qMatrix = generate_QMatrix(jpeg->block_size, factor);
    }
    else {
        Q_control = 100.f - jpeg->quality_start;
        qMatrix = generate_QMatrix_nofactor(jpeg->block_size);
    }

    uint8_t** YCbCrmodified = Encode(jpeg->YCbCr, qMatrix,
        jpeg->block_size, jpeg->block_size, jpeg->mwidth, jpeg->mheight, 
        jpeg->compression_rate, jpeg->quality_start, Q_control);
    
    jpeg->final_image = matrix_to_image(YCbCrmodified, jpeg->width, jpeg->height, jpeg->mwidth);

    deletemod(&qMatrix);
    for (int i = 0; i < 3; i++)
        deletemod(&YCbCrmodified[i]);
    deletemod(&YCbCrmodified);
}

void change_block(JpegView* jpeg, int block)
{
    for (int i = 0; i < 3; i++)
        deletemod(&jpeg->YCbCr[i]);
    deletemod(&jpeg->YCbCr);

    jpeg->block_size = block;

    jpeg->mwidth = jpeg->width;
    jpeg->mheight = jpeg->height;

    while (jpeg->mwidth % jpeg->block_size != 0)
        jpeg->mwidth++;
    while (jpeg->mheight % jpeg->block_size != 0)
        jpeg->mheight++;

    jpeg->YCbCr = image_to_matrix(jpeg->original_image, jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight);
}

JpegView* init_jpeg(uint8_t* original, int width, int height)
{
    JpegView* jpeg = new JpegView{};
    jpeg->width = width;
    jpeg->height = height;
    jpeg->block_size = 8;
    jpeg->compression_rate = false;
    jpeg->quality_start = 0;

    jpeg->original_image = original;
    jpeg->final_image = nullptr;

    jpeg->mwidth = width;
    jpeg->mheight = height;

    while (jpeg->mwidth % jpeg->block_size != 0)
        jpeg->mwidth++;
    while (jpeg->mheight % jpeg->block_size != 0)
        jpeg->mheight++;

    jpeg->YCbCr = image_to_matrix(jpeg->original_image, jpeg->width, jpeg->height, jpeg->mwidth, jpeg->mheight);

    jpeg->final_image = matrix_to_image(jpeg->YCbCr, jpeg->width, jpeg->height, jpeg->mwidth);

    return jpeg;
}

void render_jpeg(JpegView* jpeg, int block, int quality)
{
    if (jpeg->block_size != block)
        change_block(jpeg, block);

    compress(jpeg, (float)quality);
}

void image_to_opengl(JpegView* jpeg, GLuint image_texturef, GLuint image_texturef_zoom)
{
    glBindTexture(GL_TEXTURE_2D, image_texturef);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);

    glBindTexture(GL_TEXTURE_2D, image_texturef_zoom);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);
}