#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#pragma comment(lib, "opengl32")
#include "libs/glad/glad.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_win32.h"
#include "libs/imgui/imgui_impl_opengl3.h"
#include "libs/imgui/imgui_internal.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <windows.h>

void cleanarray(int** arrayi, int height)
{
    for (int y = 0; y < height; y++)
    {
        free(arrayi[y]);
    }
    free(arrayi);
}

int processingfix(int color)
{
    if (color > 255)
        return 255;
    else if (color < 0)
        return 0;
    return color;
}

typedef struct PImage
{
    uint8_t* image;
    int width, height;
} PImage;

typedef struct JpegView
{
    PImage* original;
    uint8_t* finalimage;
    float** DCTCosTable;
    int width, height;
    int** red, **green, **blue;
    int mwidth, mheight, blockSize;
} JpegView;

#include "QuadTree.h"

void zoomlayer(GLuint image_texture, JpegView* jpeg, float width, float height, float zoom)
{
    if (ImGui::IsItemHovered())
    {
        ImVec2 cursor = ImGui::GetCurrentContext()->IO.MousePos;
        ImVec2 offset = ImGui::GetItemRectMin();
        float centerx = jpeg->width * (cursor.x - offset.x) / width;
        float centery = jpeg->height * (cursor.y - offset.y) / height;
        float uv0x = (centerx - 100.f / zoom) / jpeg->width;
        float uv0y = (centery - 100.f / zoom) / jpeg->height;
        float uv1x = (centerx + 100.f / zoom) / jpeg->width;
        float uv1y = (centery + 100.f / zoom) / jpeg->height;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowPos(ImVec2(cursor.x - 50.f, cursor.y - 50.f));
        ImGui::BeginTooltip();
        ImGui::Image((void*)(intptr_t)image_texture,
            ImVec2(100.f, 100.f), ImVec2(uv0x, uv0y), ImVec2(uv1x, uv1y));
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

float** generateDCTtable(int size)
{
    float** table = (float**)calloc(size, sizeof(float*));
    for (int x = 0; x < size; x++) 
    {
        table[x] = (float*)calloc(size, sizeof(float));
        for (int u = 0; u < size; u++) 
        {
            table[x][u] = cosf((2.0f * x + 1.0f) * u * 3.1415926f / (2.0f * size));
        }
    }
    return table;
}

int** generateQMatrix(int dimension, int factor)
{
    int** qMatrix = (int**)calloc(dimension, sizeof(int*));
    for (int i = 0; i < dimension; i++)
    {
        qMatrix[i] = (int*)calloc(dimension, sizeof(int));
        for (int j = 0; j < dimension; j++)
        {
            qMatrix[i][j] = 1 + (1 + i + j) * factor;
        }
    }
    return qMatrix;
}

int** quantize(int** DCTMatrix, int** qMatrix, JpegView* jpeg)
{
    int** result = (int**)calloc(jpeg->mheight, sizeof(int*));
    for (int i = 0; i < jpeg->mheight; i++)
    {
        result[i] = (int*)calloc(jpeg->mwidth, sizeof(int));
        for (int j = 0; j < jpeg->mwidth; j++)
        {
            result[i][j] = (DCTMatrix[i][j] / qMatrix[i % jpeg->blockSize][j % jpeg->blockSize]) * qMatrix[i % jpeg->blockSize][j % jpeg->blockSize];
        }
    }
    cleanarray(DCTMatrix, jpeg->mheight);
    return result;
}

int** DCT(int** original, JpegView* jpeg)
{
    float** alpha = (float**)calloc(jpeg->blockSize, sizeof(float*));
    for (int u = 0; u < jpeg->blockSize; u++)
    {
        alpha[u] = (float*)calloc(jpeg->blockSize, sizeof(float));
        for (int v = 0; v < jpeg->blockSize; v++)
        {
            alpha[u][v] = (2.0f * (u == 0.0f ? 1.0f / sqrtf(2.0f) : 1.0f) * (v == 0.0f ? 1.0f / sqrtf(2.0f) : 1.0f)) / jpeg->blockSize;
        }
    }
    int** result = (int**)calloc(jpeg->mheight, sizeof(int*));
    for (int i = 0; i < jpeg->mheight; i++)
    {
        result[i] = (int*)calloc(jpeg->mwidth, sizeof(int));
    }
    for (int bi = 0; bi < jpeg->mheight; bi += jpeg->blockSize)
    {
        for (int bj = 0; bj < jpeg->mwidth; bj += jpeg->blockSize)
        {
            for (int u = 0; u < jpeg->blockSize; u++)
            {
                for (int v = 0; v < jpeg->blockSize; v++)
                {
                    float sum = 0.0f;
                    for (int x = 0; x < jpeg->blockSize; x++)
                    {
                        for (int y = 0; y < jpeg->blockSize; y++)
                        {
                            sum += original[bi + x][bj + y] * jpeg->DCTCosTable[x][u] * jpeg->DCTCosTable[y][v];
                        }
                    }
                    result[bi + u][bj + v] = (int)(alpha[u][v] * sum);
                }
            }
        }
    }
    return result;
}

int** reverseDCT(int** DCT, JpegView* jpeg)
{
    float** alpha = (float**)calloc(jpeg->blockSize, sizeof(float*));
    for (int u = 0; u < jpeg->blockSize; u++)
    {
        alpha[u] = (float*)calloc(jpeg->blockSize, sizeof(float));
        for (int v = 0; v < jpeg->blockSize; v++)
        {
            alpha[u][v] = (u == 0.0f ? 1.0f / sqrtf(2.0f) : 1.0f) * (v == 0.0f ? 1.0f / sqrtf(2.0f) : 1.0f);
        }
    }
    int** result = (int**)calloc(jpeg->mheight, sizeof(int*));
    for (int i = 0; i < jpeg->mheight; i++)
    {
        result[i] = (int*)calloc(jpeg->mwidth, sizeof(int));
    }
    for (int bi = 0; bi < jpeg->mheight; bi += jpeg->blockSize)
    {
        for (int bj = 0; bj < jpeg->mwidth; bj += jpeg->blockSize)
        {
            for (int x = 0; x < jpeg->blockSize; x++)
            {
                for (int y = 0; y < jpeg->blockSize; y++)
                {
                    float sum = 0.0f;
                    for (int u = 0; u < jpeg->blockSize; u++)
                    {
                        for (int v = 0; v < jpeg->blockSize; v++)
                        {
                            sum += alpha[u][v] * DCT[bi + u][bj + v] * jpeg->DCTCosTable[x][u] * jpeg->DCTCosTable[y][v];
                        }
                    }
                    result[bi + x][bj + y] = (int)(sum / sqrtf(2.0f * jpeg->blockSize));
                }
            }
        }
    }
    cleanarray(DCT, jpeg->mheight);
    return result;
}

int** toIntMatrix(PImage* original, int colorSpace, JpegView* jpeg)
{
    int** result = (int**)calloc(jpeg->mheight, sizeof(int*));
    for (int i = 0; i < jpeg->mheight; i++)
    {
        result[i] = (int*)calloc(jpeg->mwidth, sizeof(int));
    }
    switch (colorSpace)
    {
        case 0:
        {
            for (int y = 0; y < original->height; y++)
            {
                for (int x = 0; x < original->width; x++)
                {
                    result[y][x] = (int)(original->image[(original->width * y + x) * 3 + 0]);
                }
            }
            break;
        }
        case 1:
        {
            for (int y = 0; y < original->height; y++)
            {
                for (int x = 0; x < original->width; x++)
                {
                    result[y][x] = (int)(original->image[(original->width * y + x) * 3 + 1]);
                }
            }
            break;
        }
        case 2:
        {
            for (int y = 0; y < original->height; y++)
            {
                for (int x = 0; x < original->width; x++)
                {
                    result[y][x] = (int)(original->image[(original->width * y + x) * 3 + 2]);
                }
            }
            break;
        }
    }
    for (int y = 0; y < original->height; y++)
    {
        for (int x = original->width; x < jpeg->mwidth; x++)
        {
            result[y][x] = 0;
        }
    }
    for (int y = original->height; y < jpeg->mheight; y++)
    {
        for (int x = 0; x < jpeg->mwidth; x++)
        {
            result[y][x] = 0;
        }
    }
    return result;
}

uint8_t* matrixToPImage(int** red, int** green, int** blue, JpegView* jpeg)
{
    uint8_t* result = (uint8_t*)calloc(jpeg->width*jpeg->height*3, 1);
    for (int y = 0; y < jpeg->height; y++)
    {
        for (int x = 0; x < jpeg->width; x++)
        {
            result[(jpeg->width * y + x) * 3 + 0] = processingfix(red[y][x]);
            result[(jpeg->width * y + x) * 3 + 1] = processingfix(green[y][x]);
            result[(jpeg->width * y + x) * 3 + 2] = processingfix(blue[y][x]);
        }
    }
    return result;
}

void compress(JpegView* jpeg, int factor)
{
    if (jpeg->finalimage)
        free(jpeg->finalimage);
    factor = 200 - factor * 2;
    int** quantizationMatrix = generateQMatrix(jpeg->blockSize, factor);
    int** compRed = reverseDCT(quantize(DCT(jpeg->red, jpeg), quantizationMatrix, jpeg), jpeg);
    int** compGreen = reverseDCT(quantize(DCT(jpeg->green, jpeg), quantizationMatrix, jpeg), jpeg);
    int** compBlue = reverseDCT(quantize(DCT(jpeg->blue, jpeg), quantizationMatrix, jpeg), jpeg);
    jpeg->finalimage = matrixToPImage(compRed, compGreen, compBlue, jpeg);
    cleanarray(quantizationMatrix, jpeg->blockSize);
    cleanarray(compRed, jpeg->mheight); 
    cleanarray(compGreen, jpeg->mheight);
    cleanarray(compBlue, jpeg->mheight); 
}

void changeblock(JpegView* jpeg, int block)
{
    jpeg->blockSize = block;
    cleanarray(jpeg->red, jpeg->mheight);
    cleanarray(jpeg->green, jpeg->mheight);
    cleanarray(jpeg->blue, jpeg->mheight);
    jpeg->mwidth = jpeg->width + (jpeg->blockSize - jpeg->width % jpeg->blockSize);
    jpeg->mheight = jpeg->height + (jpeg->blockSize - jpeg->height % jpeg->blockSize);
    jpeg->red = toIntMatrix(jpeg->original, 0, jpeg);
    jpeg->green = toIntMatrix(jpeg->original, 1, jpeg);
    jpeg->blue = toIntMatrix(jpeg->original, 2, jpeg);
    jpeg->DCTCosTable = generateDCTtable(jpeg->blockSize);
}

JpegView* initjpeg(uint8_t* original, int width, int height, int factor)
{
    JpegView* jpeg = (JpegView*)calloc(1, sizeof(JpegView));
    jpeg->width = width;
    jpeg->height = height;
    jpeg->blockSize = 8;
    PImage* originalp = (PImage*)calloc(1, sizeof(PImage));
    originalp->width = width;
    originalp->height = height;
    originalp->image = original;
    jpeg->original = originalp;
    jpeg->mwidth = width + (jpeg->blockSize - width % jpeg->blockSize);
    jpeg->mheight = height + (jpeg->blockSize - height % jpeg->blockSize);
    jpeg->red = toIntMatrix(originalp, 0, jpeg);
    jpeg->green = toIntMatrix(originalp, 1, jpeg);
    jpeg->blue = toIntMatrix(originalp, 2, jpeg);
    jpeg->DCTCosTable = generateDCTtable(jpeg->blockSize);
    return jpeg;
}

void renderjpeg(JpegView* jpeg, int blockn, int value, GLuint image_texturef)
{
    if (jpeg->blockSize != blockn)
        changeblock(jpeg, blockn);
    compress(jpeg, value);
    glBindTexture(GL_TEXTURE_2D, image_texturef);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->finalimage);
}