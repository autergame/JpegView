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
    uint8_t* originalimage;
    uint8_t* finalimage;
    uint8_t** YCbCr;
    int mwidth, mheight, blockSize;
    bool compressionrate;
    int qualitystart;
};

#include "QuadTree.h"

void zoomlayer(GLuint image_texture, JpegView* jpeg, float& zoom, float magnifiersize, int width, int height)
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
        ImRect lastrect = GImGui->LastItemData.Rect;

        float halfmagnifier = magnifiersize / 2.f;
        float magnifierzoom = halfmagnifier / zoom;

        float centerx = jpeg->width * ((cursor.x - lastrect.Min.x) / lastrect.GetWidth());
        float centery = jpeg->height * ((cursor.y - lastrect.Min.y) / lastrect.GetHeight());
        float uv0x = (centerx - magnifierzoom) / jpeg->width;
        float uv0y = (centery - magnifierzoom) / jpeg->height;
        float uv1x = (centerx + magnifierzoom) / jpeg->width;
        float uv1y = (centery + magnifierzoom) / jpeg->height;

        float cursorboxposx = cursor.x - halfmagnifier;
        float cursorboxposy = cursor.y - halfmagnifier;
        if (cursorboxposx < 0.f)
            cursorboxposx = 0.f;
        if (cursorboxposy < 0.f)
            cursorboxposy = 0.f;
        if (cursor.x + halfmagnifier > width)
            cursorboxposx = width - magnifiersize;
        if (cursor.y + halfmagnifier > height)
            cursorboxposy = height - magnifiersize;

        ImGui::SetNextWindowPos(ImVec2(cursorboxposx, cursorboxposy));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::BeginTooltip();
        ImGui::Image((void*)(intptr_t)image_texture,
            ImVec2(magnifiersize, magnifiersize), ImVec2(uv0x, uv0y), ImVec2(uv1x, uv1y));
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

GLuint createimage(uint8_t* image, int width, int height)
{
    const GLfloat color[] = { .2f, .2f, .2f, 1.f };
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    return image_texture;
}

float* generateQMatrix_nofactor(int blockSize)
{
    float* qMatrix = new float[blockSize * blockSize]{};
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            qMatrix[y * blockSize + x] = 1.f + x + y;
        }
    }
    return qMatrix;
}

float* generateQMatrix(int blockSize, float factor)
{
    float* qMatrix = new float[blockSize * blockSize]{};
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            qMatrix[y * blockSize + x] = 1.f + (1.f + x + y) * factor;
        }
    }
    return qMatrix;
}

float** quantize(float** DCTMatrix, float factor, JpegView* jpeg)
{
    //for (int i = 0; i < 3; i++)
    //{
    //    for (int y = 0; y < jpeg->mheight; y++)
    //    {
    //        for (int x = 0; x < jpeg->mwidth; x++)
    //        {
    //            int index = (y % jpeg->blockSize) * jpeg->blockSize + (x % jpeg->blockSize);
    //            float qMatrixValue = minmaxq(qMatrix[index]);
    //            DCTMatrix[i][y * jpeg->mwidth + x] =                    
    //                roundf(DCTMatrix[i][y * jpeg->mwidth + x] / qMatrixValue) * qMatrixValue;
    //        }
    //    }
    //}
    float* qMatrix;
    if (!jpeg->compressionrate)
    {
        if (factor >= 50.f)
            factor = 200.f - factor * 2.f;
        else
            factor = 5000.f / factor;
        qMatrix = generateQMatrix(jpeg->blockSize, factor);
        for (int k = 0; k < jpeg->mheight * jpeg->mwidth; k++)
        {
            int x = (k % jpeg->mwidth);
            int y = (k / jpeg->mwidth);
            int index = (y % jpeg->blockSize) * jpeg->blockSize + (x % jpeg->blockSize);

            float qMatrixValue = minmaxq(qMatrix[index]);
            DCTMatrix[0][k] = roundf(DCTMatrix[0][k] / qMatrixValue) * qMatrixValue;
            DCTMatrix[1][k] = roundf(DCTMatrix[1][k] / qMatrixValue) * qMatrixValue;
            DCTMatrix[2][k] = roundf(DCTMatrix[2][k] / qMatrixValue) * qMatrixValue;
        }
    }
    else
    {
        float control = 100.f - jpeg->qualitystart;
        float block = (float)jpeg->mwidth / (float)jpeg->blockSize;
        qMatrix = generateQMatrix_nofactor(jpeg->blockSize);
        for (int k = 0; k < jpeg->mheight * jpeg->mwidth; k++)
        {
            int x = (k % jpeg->mwidth);
            int y = (k / jpeg->mwidth);
            int index = (y % jpeg->blockSize) * jpeg->blockSize + (x % jpeg->blockSize);

            float factor = jpeg->qualitystart + (((float)x / (float)jpeg->blockSize) / block) * control;
            if (factor >= 50.f)
                factor = 200.f - factor * 2.f;
            else
                factor = 5000.f / factor;

            float qMatrixValue = minmaxq(1.f + qMatrix[index] * factor);
            DCTMatrix[0][k] = roundf(DCTMatrix[0][k] / qMatrixValue) * qMatrixValue;
            DCTMatrix[1][k] = roundf(DCTMatrix[1][k] / qMatrixValue) * qMatrixValue;
            DCTMatrix[2][k] = roundf(DCTMatrix[2][k] / qMatrixValue) * qMatrixValue;
        }
    }
    delete qMatrix;
    return DCTMatrix;
}

float* generateDCTtable(int blockSize)
{
    float* table = new float[blockSize * blockSize]{};
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            table[y * blockSize + x] = cosf((2.f * y + 1.f) * x * 3.141592f / (2.f * blockSize));
        }
    }
    return table;
}

float* generateAlphatable(int blockSize)
{
    float sqrt1_2 = 1.f / sqrtf(2.f);
    float* alpha = new float[blockSize * blockSize]{};
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            alpha[y * blockSize + x] = (y == 0 || x == 0) ? sqrt1_2 : 1.f;
        }
    }
    alpha[0] = sqrt1_2 * sqrt1_2;
    return alpha;
}

float** DCT(uint8_t** original, float* DCTTable, JpegView* jpeg)
{
    float* alpha = generateAlphatable(jpeg->blockSize);
    float** result = new float*[3]{};
    for (int i = 0; i < 3; i++)
    {
        result[i] = new float[jpeg->mheight * jpeg->mwidth]{};
        //for (int by = 0; by < jpeg->mheight; by += jpeg->blockSize)
        //{
        //    for (int bx = 0; bx < jpeg->mwidth; bx += jpeg->blockSize)
        //    {
        //        for (int u = 0; u < jpeg->blockSize; u++)
        //        {
        //            for (int v = 0; v < jpeg->blockSize; v++)
        //            {
        //                float sum = 0.f;
        //                for (int y = 0; y < jpeg->blockSize; y++)
        //                {
        //                    for (int x = 0; x < jpeg->blockSize; x++)
        //                    {
        //                        sum += (original[i][(by + y) * jpeg->mwidth + (bx + x)] - 128.f) *
        //                            DCTTable[y * jpeg->blockSize + u] * DCTTable[x * jpeg->blockSize + v];
        //                    }
        //                }
        //                int indexuv = u * jpeg->blockSize + v;
        //                int indexresult = (by + u) * jpeg->mwidth + (bx + v);
        //                result[i][indexresult] = alpha[indexuv] * sum * 2.f / (float)jpeg->blockSize;
        //            }
        //        }
        //    }
        //}
    }
    for (int by = 0; by < jpeg->mheight; by += jpeg->blockSize)
    {
        for (int bx = 0; bx < jpeg->mwidth; bx += jpeg->blockSize)
        {
            for (int j = 0; j < jpeg->blockSize * jpeg->blockSize; j++)
            {
                int u = (j % jpeg->blockSize);
                int v = (j / jpeg->blockSize);

                float sum0 = 0.f;
                float sum1 = 0.f;
                float sum2 = 0.f;
                for (int k = 0; k < jpeg->blockSize * jpeg->blockSize; k++)
                {
                    int x = (k % jpeg->blockSize);
                    int y = (k / jpeg->blockSize);
                    int index = (by + y) * jpeg->mwidth + (bx + x);

                    float yv = DCTTable[y * jpeg->blockSize + v];
                    float xu = DCTTable[x * jpeg->blockSize + u];

                    sum0 += (original[0][index] - 128.f) * yv * xu;
                    sum1 += (original[1][index] - 128.f) * yv * xu;
                    sum2 += (original[2][index] - 128.f) * yv * xu;
                }

                float block = 2.f / (float)jpeg->blockSize;
                int index = (by + v) * jpeg->mwidth + (bx + u);

                result[0][index] = alpha[j] * sum0 * block;
                result[1][index] = alpha[j] * sum1 * block;
                result[2][index] = alpha[j] * sum2 * block;
            }
        }
    }
    delete alpha;
    return result;
}

uint8_t** reverseDCT(float** DCT, float* DCTTable, JpegView* jpeg)
{
    float* alpha = generateAlphatable(jpeg->blockSize);
    uint8_t** result = new uint8_t*[3]{};
    for (int i = 0; i < 3; i++)
    {
        result[i] = new uint8_t[jpeg->mheight * jpeg->mwidth]{};
        //for (int by = 0; by < jpeg->mheight; by += jpeg->blockSize)
        //{
        //    for (int bx = 0; bx < jpeg->mwidth; bx += jpeg->blockSize)
        //    {
        //        for (int y = 0; y < jpeg->blockSize; y++)
        //        {
        //            for (int x = 0; x < jpeg->blockSize; x++)
        //            {
        //                float sum = 0.f;
        //                for (int u = 0; u < jpeg->blockSize; u++)
        //                {
        //                    for (int v = 0; v < jpeg->blockSize; v++)
        //                    {
        //                        sum += alpha[u * jpeg->blockSize + v] * DCT[i][(by + u) * jpeg->mwidth + (bx + v)] *
        //                            DCTTable[y * jpeg->blockSize + u] * DCTTable[x * jpeg->blockSize + v];
        //                    }
        //                }
        //                result[i][(by + y) * jpeg->mwidth + (bx + x)] = 
        //                    minmaxcolor((sum * 2.f / (float)jpeg->blockSize) + 128.f);
        //            }
        //        }
        //    }
        //}        
    }
    for (int by = 0; by < jpeg->mheight; by += jpeg->blockSize)
    {
        for (int bx = 0; bx < jpeg->mwidth; bx += jpeg->blockSize)
        {
            for (int j = 0; j < jpeg->blockSize * jpeg->blockSize; j++)
            {
                int x = (j % jpeg->blockSize);
                int y = (j / jpeg->blockSize);

                float sum0 = 0.f;
                float sum1 = 0.f;
                float sum2 = 0.f;
                for (int k = 0; k < jpeg->blockSize * jpeg->blockSize; k++)
                {
                    int u = (k % jpeg->blockSize);
                    int v = (k / jpeg->blockSize);
                    int index = (by + v) * jpeg->mwidth + (bx + u);

                    float yv = DCTTable[y * jpeg->blockSize + v];
                    float xu = DCTTable[x * jpeg->blockSize + u];

                    sum0 += alpha[k] * DCT[0][index] * yv * xu;
                    sum1 += alpha[k] * DCT[1][index] * yv * xu;
                    sum2 += alpha[k] * DCT[2][index] * yv * xu;
                }

                float block = 2.f / (float)jpeg->blockSize;
                int index = (by + y) * jpeg->mwidth + (bx + x);

                result[0][index] = minmaxcolor((sum0 * block) + 128.f);
                result[1][index] = minmaxcolor((sum1 * block) + 128.f);
                result[2][index] = minmaxcolor((sum2 * block) + 128.f);
            }
        }
    }
    for (int i = 0; i < 3; i++)
        delete DCT[i];
    delete DCT;
    delete alpha;
    return result;
}

uint8_t** imageToMatrix(JpegView* jpeg)
{
    uint8_t** result = new uint8_t*[3]{};
    for (int i = 0; i < 3; i++)
        result[i] = new uint8_t[jpeg->mheight * jpeg->mwidth]{};
    //for (int y = 0; y < jpeg->height; y++)
    //{
    //    for (int x = 0; x < jpeg->width; x++)
    //    {
    //        int index = (y * jpeg->width + x) * 3;
    //        int indexresult = y * jpeg->mwidth + x;
    //        uint8_t r = jpeg->originalimage[index + 0];
    //        uint8_t g = jpeg->originalimage[index + 1];
    //        uint8_t b = jpeg->originalimage[index + 2];
    //        result[0][indexresult] = minmaxcolor((( 0.299f * r) + ( 0.587f * g) + ( 0.114f * b)));
    //        result[1][indexresult] = minmaxcolor(((-0.168f * r) + (-0.331f * g) + ( 0.500f * b)) + 128);
    //        result[2][indexresult] = minmaxcolor((( 0.500f * r) + (-0.418f * g) + (-0.081f * b)) + 128);
    //    }
    //}
    for (int i = 0; i < jpeg->height * jpeg->width; i++)
    {
        int x = (i % jpeg->width);
        int y = (i / jpeg->width);
        int index = i * 3;
        int indexresult = y * jpeg->mwidth + x;

        uint8_t r = jpeg->originalimage[index + 0];
        uint8_t g = jpeg->originalimage[index + 1];
        uint8_t b = jpeg->originalimage[index + 2];

        result[0][indexresult] = minmaxcolor((( 0.299f * r) + ( 0.587f * g) + ( 0.114f * b)));
        result[1][indexresult] = minmaxcolor(((-0.168f * r) + (-0.331f * g) + ( 0.500f * b)) + 128);
        result[2][indexresult] = minmaxcolor((( 0.500f * r) + (-0.418f * g) + (-0.081f * b)) + 128);
    }
    return result;
}

uint8_t* matrixToImage(uint8_t** YCbCr, JpegView* jpeg)
{
    uint8_t* result = new uint8_t[jpeg->height * jpeg->width * 3]{};
    //for (int y = 0; y < jpeg->height; y++)
    //{
    //    for (int x = 0; x < jpeg->width; x++)
    //    {
    //        int index = (y * jpeg->width + x) * 3;
    //        int indexYCbCr = y * jpeg->mwidth + x;
    //        uint8_t Y = YCbCr[0][indexYCbCr];
    //        int8_t Cb = YCbCr[1][indexYCbCr] - 128;
    //        int8_t Cr = YCbCr[2][indexYCbCr] - 128;
    //        result[index + 0] = minmaxcolor(Y + ( 1.402f * Cr));
    //        result[index + 1] = minmaxcolor(Y + (-0.344f * Cb) + (-0.714f * Cr));
    //        result[index + 2] = minmaxcolor(Y + ( 1.772f * Cb));
    //    }
    //}
    for (int i = 0; i < jpeg->height * jpeg->width; i++)
    {
        int x = (i % jpeg->width);
        int y = (i / jpeg->width);
        int index = i * 3;
        int indexYCbCr = y * jpeg->mwidth + x;

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
    if (jpeg->finalimage)
        delete jpeg->finalimage;

    float* DCTTable = generateDCTtable(jpeg->blockSize);

    uint8_t** YCbCrmodified = reverseDCT(quantize(DCT(jpeg->YCbCr, DCTTable, jpeg), factor, jpeg), DCTTable, jpeg);
    
    jpeg->finalimage = matrixToImage(YCbCrmodified, jpeg);

    delete DCTTable;
    for (int i = 0; i < 3; i++)
        delete YCbCrmodified[i];
    delete YCbCrmodified;
}

void changeblock(JpegView* jpeg, int block)
{
    for (int i = 0; i < 3; i++)
        delete jpeg->YCbCr[i];
    delete jpeg->YCbCr;

    jpeg->blockSize = block;

    jpeg->mwidth  = jpeg->width + (jpeg->blockSize - jpeg->width % jpeg->blockSize);
    jpeg->mheight = jpeg->height + (jpeg->blockSize - jpeg->height % jpeg->blockSize);

    jpeg->YCbCr = imageToMatrix(jpeg);
}

JpegView* initjpeg(uint8_t* original, int width, int height)
{
    JpegView* jpeg = new JpegView{};
    jpeg->width = width;
    jpeg->height = height;
    jpeg->blockSize = 8;
    jpeg->compressionrate = false;
    jpeg->qualitystart = 1;

    jpeg->originalimage = original;

    jpeg->mwidth  = width + (jpeg->blockSize - width % jpeg->blockSize);
    jpeg->mheight = height + (jpeg->blockSize - height % jpeg->blockSize);

    jpeg->YCbCr = imageToMatrix(jpeg);

    jpeg->finalimage = matrixToImage(jpeg->YCbCr, jpeg);

    return jpeg;
}

void renderjpeg(JpegView* jpeg, int block, int quality, GLuint image_texturef)
{
    if (jpeg->blockSize != block)
        changeblock(jpeg, block);

    compress(jpeg, (float)quality);

    glBindTexture(GL_TEXTURE_2D, image_texturef);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->finalimage);
}