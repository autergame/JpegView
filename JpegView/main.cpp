//author https://github.com/autergame

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glfw3")

#pragma warning(push, 0)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#define __STDC_LIB_EXT1__
#include <stb/stb_image_write.h>
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

#include "ThreadPool.h"
#include "JpegView.h"
#include "sha512.h"
#include "miniz.h"
#include "QuadTree.h"

double GetTimeSinceStart(LARGE_INTEGER Frequencye, LARGE_INTEGER Starte)
{
	LARGE_INTEGER t_End;
	QueryPerformanceCounter(&t_End);
	return (float)(t_End.QuadPart - Starte.QuadPart) / Frequencye.QuadPart;
}

void glfw_error_callback(int error, const char* description)
{
	char msgTitle[512] = { '\0' };
	sprintf_s(msgTitle, 512, "GLFW Error %d: %s", error, description);
	MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
	exit(1);
}

void strip_filepath(char* fname)
{
	char* end = fname + strlen(fname);
	while (end > fname && *end != '\\')
		--end;
	if (end > fname)
		*end = '\0';
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	LARGE_INTEGER Frequencye, Starte;
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

	char msgTitle[512] = { '\0' };
	char openPath[MAX_PATH] = { '\0' };
	char savePath[MAX_PATH] = { '\0' };
	char openFile[MAX_PATH] = { '\0' };
	char saveFile[MAX_PATH] = { '\0' };
	char openFile_temp[MAX_PATH] = { '\0' };

	char currentPath[MAX_PATH] = { '\0' };
	GetCurrentDirectoryA(MAX_PATH, currentPath);

	LSTATUS regStatus = 0;
	HKEY regkeyresult = nullptr;
	regStatus = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", 0, KEY_ALL_ACCESS, &regkeyresult);
	if (regStatus == ERROR_PATH_NOT_FOUND || regStatus == ERROR_FILE_NOT_FOUND)
	{
		regStatus = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", 0, nullptr,
			REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_QUERY_VALUE, nullptr, &regkeyresult, nullptr);
		if (regStatus != ERROR_SUCCESS)
		{
			sprintf_s(msgTitle, 512, "Creating key failed: %d %d", regStatus, GetLastError());
			MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
			exit(1);
		}
	}
	else if (regStatus != ERROR_SUCCESS)
	{
		sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
		MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
		exit(1);
	}

	DWORD pathSize = MAX_PATH;
	regStatus = RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", "openpath",
		RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, openPath, &pathSize);
	if (regStatus == ERROR_FILE_NOT_FOUND || regStatus == ERROR_FILE_NOT_FOUND)
	{
		regStatus = RegSetValueExA(regkeyresult, "openpath", 0, REG_EXPAND_SZ, (LPCBYTE)currentPath, MAX_PATH);
		if (regStatus != ERROR_SUCCESS)
		{
			sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
			MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
			exit(1);
		}
	}
	else if (regStatus != ERROR_SUCCESS)
	{
		sprintf_s(msgTitle, 512, "Getting key value failed: %d %d", regStatus, GetLastError());
		MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
		exit(1);
	}

	pathSize = MAX_PATH;
	regStatus = RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", "savepath",
		RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, savePath, &pathSize);
	if (regStatus == ERROR_FILE_NOT_FOUND || regStatus == ERROR_FILE_NOT_FOUND)
	{
		regStatus = RegSetValueExA(regkeyresult, "savepath", 0, REG_EXPAND_SZ, (LPCBYTE)currentPath, MAX_PATH);
		if (regStatus != ERROR_SUCCESS)
		{
			sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
			MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
			exit(1);
		}
	}
	else if (regStatus != ERROR_SUCCESS)
	{
		sprintf_s(msgTitle, 512, "Getting key value failed: %d %d", regStatus, GetLastError());
		MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
		exit(1);
	}

	pathSize = 4;
	DWORD vSync = 0;
	regStatus = RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", "vsync",
		RRF_RT_REG_DWORD, nullptr, &vSync, &pathSize);
	if (regStatus == ERROR_FILE_NOT_FOUND || regStatus == ERROR_FILE_NOT_FOUND)
	{
		vSync = 0;
		regStatus = RegSetValueExA(regkeyresult, "vsync", 0, REG_DWORD, (LPCBYTE)&vSync, 4);
		if (regStatus != ERROR_SUCCESS)
		{
			sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
			MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
			exit(1);
		}
	}
	else if (regStatus != ERROR_SUCCESS)
	{
		sprintf_s(msgTitle, 512, "Getting key value failed: %d %d", regStatus, GetLastError());
		MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
		exit(1);
	}

	RECT rectScreen;
	HWND hwndScreen = GetDesktopWindow();
	GetWindowRect(hwndScreen, &rectScreen);
	int windowWidth = (int)((float)rectScreen.right * 0.75f);
	int windowHeight = (int)((float)rectScreen.bottom * 0.75f);
	int windowPosX = ((rectScreen.right - rectScreen.left) / 2 - windowWidth / 2);
	int windowPosY = ((rectScreen.bottom - rectScreen.top) / 2 - windowHeight / 2);

	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();

	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* glfwWindow = glfwCreateWindow(windowWidth, windowHeight, "JpegView", nullptr, nullptr);

	glfwMakeContextCurrent(glfwWindow);
	glfwSwapInterval(vSync);

	glfwSetWindowPos(glfwWindow, windowPosX, windowPosY);
	gladLoadGL();

	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = nullptr;
	ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImVec4* colors = GImGui->Style.Colors;
	colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
	colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
	colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(GImGui->Style.ItemSpacing.x, GImGui->Style.ItemSpacing.x));
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 13);
	int FULL_SCREEN_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |	
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysVerticalScrollbar;

	ImGuiWindowFlags usescroll = ImGuiWindowFlags_NoScrollWithMouse;

	float zoomv = 2.f;
	float magnifiersize = 200.f;

	float zoomvmax = 100.f;
	float magnifiersizemax = 1000.f;

	bool usezoom = true;
	bool jpegcomp = true;
	bool qtablegen = true;

	bool useycbcr = true;
	bool usethreads = true;
	bool usefastdct = true;

	bool drawline = true;
	bool quadtree = false;
	bool quadtreepow2 = false;

	GLuint image_textureo = 0;
	GLuint image_texturef = 0;
	GLuint image_textureo_zoom = 0;
	GLuint image_texturef_zoom = 0;

	ImVec2 uv_min = ImVec2(0.f, 0.f);
	ImVec2 uv_max = ImVec2(1.f, 1.f);

	int quality = 90, block_size = 8;
	int subsampling_index = 0, block_size_index = 2;

	const char* block_size_items[] = {
		"2", "4", "8", "16", "32", "64", "128", "256", "512"
	};
	const char* subsampling_items[] = { 
		"4:4:4", "4:4:0", "4:2:2", "4:2:0", "4:1:1", "4:1:0"
	};

	int max_depth = 50, threshold_error = 5;
	int min_size = 8, max_size = 32;
	int min_size_index = 2, max_size_index = 4;

	int max_depthmax = 100, threshold_errormax = 100;
	int min_sizemax = 128, max_sizemax = 256;

	JpegView* jpeg = nullptr;
	int swidth, sheight, schannels;

	quadnode* rootquad = nullptr;
	std::vector<quadnode*> rootquadlist;

	float Deltatime = 0, Lastedtime = 0;
	char windowTitle[64] = { '\0' };

#define TESTBIN
#if defined TESTBIN && defined _DEBUG
	const char* teststr = "C:\\Users\\autergame\\Pictures\\jpeg_testpattern.png";
	uint8_t* img = stbi_load(teststr, &swidth, &sheight, &schannels, 3);
	if (img != nullptr)
	{
		memcpy(openFile, teststr, strlen(teststr));
		jpeg = init_jpeg(img, swidth, sheight, block_size, useycbcr);
		image_textureo = create_image(img, swidth, sheight, true);
		image_texturef = create_image(jpeg->final_image, swidth, sheight, true);
		image_textureo_zoom = create_image(img, swidth, sheight, false);
		image_texturef_zoom = create_image(jpeg->final_image, swidth, sheight, false);
	}
	else
		MessageBoxA(nullptr, "Error loading the image", "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
#endif 

	glClearColor(0.f, 0.f, 0.f, 1.f);
	HWND glfwWindowNative = glfwGetWin32Window(glfwWindow);
	while (!glfwWindowShouldClose(glfwWindow))
	{
		Deltatime = float(GetTimeSinceStart(Frequencye, Starte) - Lastedtime);
		Lastedtime = (float)GetTimeSinceStart(Frequencye, Starte);

		glfwPollEvents();
		if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(glfwWindow, true);

		glfwGetFramebufferSize(glfwWindow, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT);

		sprintf_s(windowTitle, 64,
			"JpegView - Fps: %1.0f / Ms: %1.3f", GImGui->IO.Framerate, 1000.0f / GImGui->IO.Framerate);
		glfwSetWindowTitle(glfwWindow, windowTitle);

		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));
		ImGui::Begin("Main", 0, FULL_SCREEN_FLAGS | usescroll);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Open image"))
			{
				OPENFILENAMEA ofn;
				memset(&ofn, 0, sizeof(ofn));
				memset(openPath, 0, MAX_PATH);
				memset(openFile_temp, 0, MAX_PATH);

				pathSize = MAX_PATH;
				regStatus = RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", "openpath",
					RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, openPath, &pathSize);
				if (regStatus != ERROR_SUCCESS)
				{
					sprintf_s(msgTitle, 512, "Getting key value failed: %d %d", regStatus, GetLastError());
					MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
					exit(1);
				}

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = glfwWindowNative;
				ofn.lpstrFile = openFile_temp;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter =
					"Image Files (*.jpg;*.jpeg;*.png;*.bmp;*.qmi)\0*.jpg;*.jpeg;*.png;*.bmp;*.qmi\0"
					"JPG JPEG Image (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0"
					"PNG Image (*.png)\0*.png\0"
					"BMP Image (*.bmp)\0*.bmp\0"
					"QUADMIND Image (*.qmi)\0*.qmi\0";
					//"All Files (*.*)\0*.*\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = nullptr;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = openPath;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileNameA(&ofn) == TRUE)
				{
					if (openFile_temp[0] != '\0')
					{
						memcpy(openFile, openFile_temp, MAX_PATH);
						memcpy(openPath, openFile, MAX_PATH);
						
						strip_filepath(openPath);
						size_t openPathLen = strlen(openPath);

						regStatus = RegSetValueExA(regkeyresult, "openpath", 0, REG_EXPAND_SZ,
							(LPCBYTE)openPath, (DWORD)openPathLen);
						if (regStatus != ERROR_SUCCESS)
						{
							sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
							MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
							exit(1);
						}

						for (size_t i = ofn.nFileExtension; i < strlen(openFile_temp); i++)
							openFile_temp[i] = tolower(openFile_temp[i]);

						uint8_t* img;
						if (ofn.nFilterIndex != 5 && strcmp(openFile_temp + ofn.nFileExtension, "qmi") != 0)
							img = stbi_load(openFile, &swidth, &sheight, &schannels, 3);
						else
							img = loadquad(openFile, &swidth, &sheight, usethreads, usefastdct);
						if (img != nullptr)
						{
							clean_node(&rootquad);
							if (jpeg != nullptr)
							{
								deletemod(&jpeg->final_image);
								deletemod(&jpeg->original_image);

								for (int i = 0; i < 3; i++)
									deletemod(&jpeg->image_converted[i]);

								deletemod(&jpeg->image_converted);

								deletemod(&jpeg);

								glDeleteTextures(1, &image_textureo);
								glDeleteTextures(1, &image_texturef);
								glDeleteTextures(1, &image_textureo_zoom);
								glDeleteTextures(1, &image_texturef_zoom);
							}

							jpeg = init_jpeg(img, swidth, sheight, block_size, useycbcr);
							image_textureo = create_image(img, swidth, sheight, true);
							image_texturef = create_image(jpeg->final_image, swidth, sheight, true);
							image_textureo_zoom = create_image(img, swidth, sheight, false);
							image_texturef_zoom = create_image(jpeg->final_image, swidth, sheight, false);
						}
						else
							MessageBoxA(nullptr, "Error in loading the image", "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
					}
				}
			}
			if (openFile[0] != '\0')
			{
				if (ImGui::MenuItem("Save image"))
				{
					OPENFILENAMEA ofn;
					memset(&ofn, 0, sizeof(ofn));
					memset(saveFile, 0, MAX_PATH);
					memset(savePath, 0, MAX_PATH);

					pathSize = MAX_PATH;
					regStatus = RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\JpegView", "savepath",
						RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, savePath, &pathSize);
					if (regStatus != ERROR_SUCCESS)
					{
						sprintf_s(msgTitle, 512, "Getting key value failed: %d %d", regStatus, GetLastError());
						MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
						exit(1);
					}

					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = glfwWindowNative;
					ofn.lpstrFile = saveFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "PNG Image (*.png)\0*.png\0"
									  "QUADMIND Image (*.qmi)\0*.qmi\0";
					ofn.lpstrDefExt = "png\0qmi";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = nullptr;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = savePath;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

					if (GetSaveFileNameA(&ofn) == TRUE)
					{
						if (openFile[0] != '\0')
						{				
							memcpy(savePath, saveFile, MAX_PATH);

							strip_filepath(savePath);
							size_t savePathLen = strlen(savePath);

							regStatus = RegSetValueExA(regkeyresult, "savepath", 0, REG_EXPAND_SZ,
								(LPCBYTE)savePath, (DWORD)savePathLen);
							if (regStatus != ERROR_SUCCESS)
							{
								sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
								MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
								exit(1);
							}

							int saved = 0;
							if (ofn.nFilterIndex == 1)
								saved = stbi_write_png(saveFile, jpeg->width, jpeg->height, 3, jpeg->final_image, 0);
							else if (ofn.nFilterIndex == 2)
								saved = savequad(saveFile, rootquadlist, jpeg->width, jpeg->height, quality, qtablegen, useycbcr);
							if (!saved)
								MessageBoxA(nullptr, "Error in saving the image", "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Enable Vsync", (bool*)&vSync))
				{
					regStatus = RegSetValueExA(regkeyresult, "vsync", 0, REG_DWORD, (LPCBYTE)&vSync, 4);
					if (regStatus != ERROR_SUCCESS)
					{
						sprintf_s(msgTitle, 512, "Setting key value failed: %d %d", regStatus, GetLastError());
						MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
						exit(1);
					}

					glfwSwapInterval(vSync);
				}
			}
			ImGui::EndMenuBar();
		}
		Start:
		if (openFile[0] != '\0')
		{
			if (jpeg != nullptr)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("File: %s", openFile); ImGui::SameLine();
				ImGui::Text("Image Size: %d %d", jpeg->width, jpeg->height); ImGui::SameLine();
				if (ImGui::Button("Close Image"))
				{
					clean_node(&rootquad);
					if (jpeg != nullptr)
					{
						memset(openFile, 0, MAX_PATH);
	
						deletemod(&jpeg->final_image);
						deletemod(&jpeg->original_image);

						for (int i = 0; i < 3; i++)
							deletemod(&jpeg->image_converted[i]);

						deletemod(&jpeg->image_converted);

						deletemod(&jpeg);

						glDeleteTextures(1, &image_textureo);
						glDeleteTextures(1, &image_texturef);
						glDeleteTextures(1, &image_textureo_zoom);
						glDeleteTextures(1, &image_texturef_zoom);

						goto Start;
					}
				}
			}

			ImGui::Separator();

			ImGui::Columns(2);

			float firstcolumn = ImGui::GetColumnWidth(0) * 0.90f;

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use Jpeg?", &jpegcomp);
			if (jpegcomp)
			{
				ImGui::Indent();
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Quality Factor:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(firstcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::DragInt("##inputf", &quality, 1.f, 1, 100);
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Block Size:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(firstcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				if (ImGui::Combo("##list_block", &block_size_index, block_size_items, IM_ARRAYSIZE(block_size_items)))
					block_size = 1 << (block_size_index + 1);
				ImGui::AlignTextToFramePadding();
				ImGui::Checkbox("Use Generated Quantization Table?", &qtablegen);
				ImGui::Checkbox("Show Compression Rate?", &jpeg->compression_rate);
				if (jpeg->compression_rate)
				{
					ImGui::Indent();
					ImGui::AlignTextToFramePadding(); ImGui::Bullet();
					ImGui::Text("Quality Start:"); ImGui::SameLine();
					ImGui::SetNextItemWidth(firstcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
					ImGui::DragInt("##inputc", &jpeg->quality_start, 1.f, 1, 100);
					ImGui::Unindent();
				}
				ImGui::Unindent();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use Zoom?", &usezoom);
			if (usezoom)
			{
				ImGui::Indent();
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Zoom:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(firstcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::DragFloat("##inputz", &zoomv, 1.f, 1.f, zoomvmax, "%g");
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Lupe Size:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(firstcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::DragFloat("##inputl", &magnifiersize, 1.f, 10.f, magnifiersizemax, "%g");
				ImGui::Unindent();
			}

			if (zoomv >= zoomvmax)
			{
				zoomvmax += 100.f;
				zoomv -= 10.f;
			}
			if (magnifiersize >= magnifiersizemax)
			{
				magnifiersizemax += 1000.f;
				magnifiersize -= 100.f;
			}

			ImGui::NextColumn();

			float secondcolumn = ImGui::GetColumnWidth(0) + (ImGui::GetColumnWidth(1) * 0.90f);

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use QuadTree?", &quadtree);
			if (quadtree)
			{
				ImGui::Indent();
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Max Depth:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(secondcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::DragInt("##inputd", &max_depth, 1.f, 1, max_depthmax);
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Error Threshold:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(secondcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::DragInt("##inpute", &threshold_error, 1.f, 0, threshold_errormax);
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Min Quad Size:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(secondcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				if (ImGui::Combo("##list_block_min", &min_size_index, block_size_items, IM_ARRAYSIZE(block_size_items)))
					min_size = 1 << (min_size_index + 1);
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Max Quad Size:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(secondcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				if (ImGui::Combo("##list_block_max", &max_size_index, block_size_items, IM_ARRAYSIZE(block_size_items)))
					max_size = 1 << (max_size_index + 1);
				ImGui::AlignTextToFramePadding();
				ImGui::Checkbox("Use Quad Size Power Of 2", &quadtreepow2);
				ImGui::Checkbox("Draw Quadrant Line?", &drawline);
				ImGui::Unindent();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use Fast DCT Algorithm?", &usefastdct);
			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use Multi-Threading?", &usethreads);
			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use YCbCr Colors?", &useycbcr);
			ImGui::AlignTextToFramePadding(); ImGui::Bullet();
			ImGui::Text("Chroma Subsampling:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(secondcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
			ImGui::Combo("##list_sub", &subsampling_index, subsampling_items, IM_ARRAYSIZE(subsampling_items));
			
			if (max_depth >= max_depthmax)
			{
				max_depthmax += 100;
				max_depth -= 10;
			}
			if (threshold_error >= threshold_errormax)
			{
				threshold_errormax += 100;
				threshold_error -= 10;
			}

			ImGui::Columns();

			ImGui::Separator();

			if (ImGui::Button("Compress", ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
			{
				if (quadtree && jpegcomp)
				{
					clean_node(&rootquad);
					rootquadlist = render_quadtree_jpeg(&rootquad, jpeg, max_depth,
						threshold_error, min_size, max_size, drawline, quality, qtablegen, 
						subsampling_index, useycbcr, usethreads, usefastdct);
				}
				else if (!quadtree && !jpegcomp)
				{
					deletemod(&jpeg->final_image);

					if (useycbcr)
					{
						render_ycbcr(jpeg, block_size, subsampling_index);
						jpeg->final_image = YCbCr_matrix_to_image(jpeg->image_converted,
							jpeg->width, jpeg->height, jpeg->mwidth);
					} 
					else
					{
						render_rgb(jpeg, block_size, subsampling_index);
						jpeg->final_image = RGB_matrix_to_image(jpeg->image_converted,
							jpeg->width, jpeg->height, jpeg->mwidth);
					}
				}
				else 
				{
					if (quadtree)
					{
						clean_node(&rootquad);
						rootquadlist = render_quadtree(&rootquad, jpeg, max_depth, threshold_error,
							min_size, max_size, drawline, quadtreepow2, subsampling_index);
					}
					else if (jpegcomp) {
						render_jpeg(jpeg, block_size, quality, qtablegen,
							subsampling_index, block_size_index, useycbcr, usethreads, usefastdct);
					}
				}
				image_to_opengl(jpeg, image_texturef, image_texturef_zoom);
			}

			float newwidth = ImGui::GetContentRegionAvail().x / 2.f - GImGui->Style.ItemSpacing.x;
			float newheight = newwidth * ((float)jpeg->height / (float)jpeg->width);
			
			int dontusescroll = 0;
			ImGui::Image((void*)(intptr_t)image_textureo, ImVec2(newwidth, newheight),
				uv_min, uv_max, tint_col, border_col);
			if (usezoom)
				dontusescroll += zoom_layer(image_textureo_zoom, jpeg,
					zoomv, magnifiersize, windowWidth, windowHeight);

			ImGui::SameLine();
			ImGui::Image((void*)(intptr_t)image_texturef, ImVec2(newwidth, newheight),
				uv_min, uv_max, tint_col, border_col);
			if (usezoom)
				dontusescroll += zoom_layer(image_texturef_zoom, jpeg,
					zoomv, magnifiersize, windowWidth, windowHeight);

			if (dontusescroll > 0)
				usescroll = ImGuiWindowFlags_NoScrollWithMouse;
			else
				usescroll = ImGuiWindowFlags_None;
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(glfwWindow);
	}

	clean_node(&rootquad);
	if (jpeg != nullptr)
	{
		deletemod(&jpeg->final_image);
		deletemod(&jpeg->original_image);

		for (int i = 0; i < 3; i++)

			deletemod(&jpeg->image_converted[i]);
		deletemod(&jpeg->image_converted);

		deletemod(&jpeg);

		glDeleteTextures(1, &image_textureo);
		glDeleteTextures(1, &image_texturef);
		glDeleteTextures(1, &image_textureo_zoom);
		glDeleteTextures(1, &image_texturef_zoom);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfwWindow);
	glfwTerminate();

	RegCloseKey(regkeyresult);

	return 0;
}