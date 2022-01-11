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
        return  255.f;
    else if (color <= 0.f)
        return 1.f;
    return color;
}

struct JpegView
{
    int width, height;
    uint8_t* originalimage;
    uint8_t* finalimage;
    float* DCTCosTable;
    uint8_t* red, *green, *blue;
    int mwidth, mheight, blockSize;
};

#include "QuadTree.h"

void zoomlayer(GLuint image_texture, JpegView* jpeg, float zoom, float magnifiersize)
{
    if (ImGui::IsItemHovered())
    {
        ImVec2 cursor = ImGui::GetCurrentContext()->IO.MousePos;
        ImRect lastrect = ImGui::GetCurrentContext()->LastItemData.Rect;

        float centerx = jpeg->width * (cursor.x - lastrect.Min.x) / (lastrect.Max.x - lastrect.Min.x);
        float centery = jpeg->height * (cursor.y - lastrect.Min.y) / (lastrect.Max.y - lastrect.Min.y);
        float uv0x = (centerx - magnifiersize / zoom) / jpeg->width;
        float uv0y = (centery - magnifiersize / zoom) / jpeg->height;
        float uv1x = (centerx + magnifiersize / zoom) / jpeg->width;
        float uv1y = (centery + magnifiersize / zoom) / jpeg->height;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowPos(ImVec2(cursor.x - magnifiersize/2.f, cursor.y - magnifiersize/2.f));
        ImGui::BeginTooltip();
        ImGui::Image((void*)(intptr_t)image_texture,
            ImVec2(magnifiersize, magnifiersize), ImVec2(uv0x, uv0y), ImVec2(uv1x, uv1y));
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

GLuint createimage(uint8_t* image, int width, int height)
{
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    return image_texture;
}

int16_t* quantize(int16_t* DCTMatrix, float* qMatrix, JpegView* jpeg)
{
    for (int y = 0; y < jpeg->mheight; y++)
    {
        for (int x = 0; x < jpeg->mwidth; x++)
        {
            int index = (y % jpeg->blockSize) * jpeg->blockSize + (x % jpeg->blockSize);
            DCTMatrix[y * jpeg->mwidth + x] =
                  roundf(DCTMatrix[y * jpeg->mwidth + x] / minmaxq(qMatrix[index])) * minmaxq(qMatrix[index]);
        }
    }
    return DCTMatrix;
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

float* generateDCTtable(int blockSize)
{
    float* table = new float[blockSize * blockSize]{};
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            table[y * blockSize + x] = cosf((2.f * y + 1.f) * x * 3.1415926f / (2.f * blockSize));
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
            alpha[y * blockSize + x] = (y == 0 ? sqrt1_2 : 1.f) * (x == 0 ? sqrt1_2 : 1.f);
        }
    }
    return alpha;
}

int16_t* DCT(uint8_t* original, JpegView* jpeg)
{
    float* alpha = generateAlphatable(jpeg->blockSize);
    int16_t* result = new int16_t[jpeg->mheight * jpeg->mwidth]{};

    for (int by = 0; by < jpeg->mheight; by += jpeg->blockSize)
    {
        for (int bx = 0; bx < jpeg->mwidth; bx += jpeg->blockSize)
        {
            for (int u = 0; u < jpeg->blockSize; u++)
            {
                for (int v = 0; v < jpeg->blockSize; v++)
                {
                    float sum = 0.f;
                    for (int y = 0; y < jpeg->blockSize; y++)
                    {
                        for (int x = 0; x < jpeg->blockSize; x++)
                        {
                            sum += (original[(by + y) * jpeg->mwidth + (bx + x)] - 128.f) *
                                jpeg->DCTCosTable[y * jpeg->blockSize + u] * jpeg->DCTCosTable[x * jpeg->blockSize + v];
                        }
                    }
                    int indexuv = u * jpeg->blockSize + v;
                    int indexresult = (by + u) * jpeg->mwidth + (bx + v);
                    result[indexresult] = alpha[indexuv] * sum * 2.f / (float)jpeg->blockSize;
                }
            }
        }
    }

    delete alpha;
    return result;
}

uint8_t* reverseDCT(int16_t* DCT, JpegView* jpeg)
{
    float* alpha = generateAlphatable(jpeg->blockSize);
    uint8_t* result = new uint8_t[jpeg->mheight * jpeg->mwidth]{};

    for (int by = 0; by < jpeg->mheight; by += jpeg->blockSize)
    {
        for (int bx = 0; bx < jpeg->mwidth; bx += jpeg->blockSize)
        {
            for (int y = 0; y < jpeg->blockSize; y++)
            {
                for (int x = 0; x < jpeg->blockSize; x++)
                {
                    float sum = 0.f;
                    for (int u = 0; u < jpeg->blockSize; u++)
                    {
                        for (int v = 0; v < jpeg->blockSize; v++)
                        {
                            sum += alpha[u * jpeg->blockSize + v] * DCT[(by + u) * jpeg->mwidth + (bx + v)] *
                                jpeg->DCTCosTable[y * jpeg->blockSize + u] * jpeg->DCTCosTable[x * jpeg->blockSize + v];
                        }
                    }
                    result[(by + y) * jpeg->mwidth + (bx + x)] = minmaxcolor((sum * 2.f / (float)jpeg->blockSize) + 128.f);
                }
            }
        }
    }

    delete DCT;
    delete alpha;
    return result;
}

uint8_t* toFloatMatrix(JpegView* jpeg, int colorSpace)
{
    uint8_t* result = new uint8_t[jpeg->mheight * jpeg->mwidth]{};

    switch (colorSpace)
    {
        case 0:
        {
            for (int y = 0; y < jpeg->height; y++)
            {
                for (int x = 0; x < jpeg->width; x++)
                {
                    int index = (y * jpeg->width + x) * 3;
                    result[y * jpeg->mwidth + x] = ((0.299f * jpeg->originalimage[index + 0])
                                                   +(0.587f * jpeg->originalimage[index + 1])
                                                   +(0.114f * jpeg->originalimage[index + 2]));
                }
            }
            break;
        }
        case 1:
        {
            for (int y = 0; y < jpeg->height; y++)
            {
                for (int x = 0; x < jpeg->width; x++)
                {
                    int index = (y * jpeg->width + x) * 3;
                    result[y * jpeg->mwidth + x] = ((-0.168f * jpeg->originalimage[index + 0])
                                                   +(-0.331f * jpeg->originalimage[index + 1])
                                                   +( 0.500f * jpeg->originalimage[index + 2])) + 128;
                }
            }
            break;
        }
        case 2:
        {
            for (int y = 0; y < jpeg->height; y++)
            {
                for (int x = 0; x < jpeg->width; x++)
                {
                    int index = (y * jpeg->width + x) * 3;
                    result[y * jpeg->mwidth + x] = (( 0.500f * jpeg->originalimage[index + 0])
                                                   +(-0.418f * jpeg->originalimage[index + 1])
                                                   +(-0.081f * jpeg->originalimage[index + 2])) + 128;
                }
            }
            break;
        }
    }
    return result;
}

uint8_t* matrixToImage(uint8_t* Y, uint8_t* Cb, uint8_t* Cr, JpegView* jpeg)
{
    uint8_t* result = new uint8_t[jpeg->height * jpeg->width * 3]{};
    for (int y = 0; y < jpeg->height; y++)
    {
        for (int x = 0; x < jpeg->width; x++)
        {
            int indexycbcr = y * jpeg->mwidth + x;
            int index = (y * jpeg->width + x) * 3;
            result[index + 0] = minmaxcolor(Y[indexycbcr] + ( 1.402f * (Cr[indexycbcr] - 128)));
            result[index + 1] = minmaxcolor(Y[indexycbcr] + (-0.344f * (Cb[indexycbcr] - 128)) + (-0.714f * (Cr[indexycbcr] - 128)));
            result[index + 2] = minmaxcolor(Y[indexycbcr] + ( 1.772f * (Cb[indexycbcr] - 128)));
        }
    }
    //for (int i = 0, id = 0; i < jpeg->height * jpeg->width * 3; i += 3, id++)
    //{
    //    int index = (id / jpeg->height) * jpeg->mwidth + (id % jpeg->width);
    //    result[i + 0] = minmaxcolor(Y[index] + ( 1.402f * (Cr[index] - 128)));
    //    result[i + 1] = minmaxcolor(Y[index] + (-0.344f * (Cb[index] - 128)) + (-0.714f * (Cr[index] - 128)));
    //    result[i + 2] = minmaxcolor(Y[index] + ( 1.772f * (Cb[index] - 128)));
    //}
    return result;
}

void compress(JpegView* jpeg, float factor)
{
    if (jpeg->finalimage)
        delete jpeg->finalimage;

    if (factor >= 50.f)
        factor = 200.f - factor * 2.f;
    else
        factor = 5000.f / factor;

    float* quantizationMatrix = generateQMatrix(jpeg->blockSize, factor);

    uint8_t* compRed = reverseDCT(quantize(DCT(jpeg->red, jpeg), quantizationMatrix, jpeg), jpeg);
    uint8_t* compGreen = reverseDCT(quantize(DCT(jpeg->green, jpeg), quantizationMatrix, jpeg), jpeg);
    uint8_t* compBlue = reverseDCT(quantize(DCT(jpeg->blue, jpeg), quantizationMatrix, jpeg), jpeg);
    
    jpeg->finalimage = matrixToImage(compRed, compGreen, compBlue, jpeg);

    delete quantizationMatrix;
    delete compRed;
    delete compGreen;
    delete compBlue;
}

void changeblock(JpegView* jpeg, int block)
{
    delete jpeg->red;
    delete jpeg->green;
    delete jpeg->blue;
    delete jpeg->DCTCosTable;

    jpeg->blockSize = block;

    jpeg->mwidth = jpeg->width + (jpeg->blockSize - jpeg->width % jpeg->blockSize);
    jpeg->mheight = jpeg->height + (jpeg->blockSize - jpeg->height % jpeg->blockSize);

    jpeg->red = toFloatMatrix(jpeg, 0);
    jpeg->green = toFloatMatrix(jpeg, 1);
    jpeg->blue = toFloatMatrix(jpeg, 2);

    jpeg->DCTCosTable = generateDCTtable(jpeg->blockSize);
}

JpegView* initjpeg(uint8_t* original, int width, int height)
{
    JpegView* jpeg = new JpegView{};
    jpeg->width = width;
    jpeg->height = height;
    jpeg->blockSize = 8;

    jpeg->originalimage = original;

    jpeg->mwidth = width + (jpeg->blockSize - width % jpeg->blockSize);
    jpeg->mheight = height + (jpeg->blockSize - height % jpeg->blockSize);

    jpeg->red = toFloatMatrix(jpeg, 0);
    jpeg->green = toFloatMatrix(jpeg, 1);
    jpeg->blue = toFloatMatrix(jpeg, 2);

    jpeg->finalimage = matrixToImage(jpeg->red, jpeg->green, jpeg->blue, jpeg);

    jpeg->DCTCosTable = generateDCTtable(jpeg->blockSize);

    return jpeg;
}

void renderjpeg(JpegView* jpeg, int blockn, int value, GLuint image_texturef)
{
    if (jpeg->blockSize != blockn)
        changeblock(jpeg, blockn);

    compress(jpeg, (float)value);

    glBindTexture(GL_TEXTURE_2D, image_texturef);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->finalimage);
}