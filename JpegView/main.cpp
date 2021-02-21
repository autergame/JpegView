#pragma once
//author https://github.com/autergame
#include "JpegView.h"

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
	const int* attribList);
wglCreateContextAttribsARB_type* wglCreateContextAttribsARB;

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef bool WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int* piAttribIList,
	const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
wglChoosePixelFormatARB_type* wglChoosePixelFormatARB;

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_SAMPLE_BUFFERS_ARB                    0x2041
#define WGL_SAMPLES_ARB                           0x2042

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

void* GetAnyGLFuncAddress(const char* name)
{
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	return p;
}

bool touch[256];
int width, height;
bool active = true;
LARGE_INTEGER Frequencye, Starte;

double GetTimeSinceStart()
{
	LARGE_INTEGER t_End;
	QueryPerformanceCounter(&t_End);
	return (float)(t_End.QuadPart - Starte.QuadPart) / Frequencye.QuadPart;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		glViewport(0, 0, width, height);
		PostMessage(hWnd, WM_PAINT, 0, 0);
		break;

	case WM_CLOSE:
		active = FALSE;
		break;

	case WM_KEYDOWN:
		touch[wParam] = TRUE;
		break;

	case WM_KEYUP:
		touch[wParam] = FALSE;
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


int main()
{
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

	WNDCLASSA window_class;
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = (WNDPROC)WindowProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = GetModuleHandle(0);
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = 0;
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "jpegviewer";

	if (!RegisterClassA(&window_class)) {
		printf("Failed to RegisterClassA: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	RECT rectScreen;
	width = 1024, height = 600;
	HWND hwndScreen = GetDesktopWindow();
	GetWindowRect(hwndScreen, &rectScreen);
	int PosX = ((rectScreen.right - rectScreen.left) / 2 - width / 2);
	int PosY = ((rectScreen.bottom - rectScreen.top) / 2 - height / 2);

	HWND window = CreateWindowExA(
		0,
		window_class.lpszClassName,
		"OpenGL Window",
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		PosX,
		PosY,
		width,
		height,
		0,
		0,
		window_class.hInstance,
		0);

	if (!window) {
		printf("Failed to CreateWindowExA: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	WNDCLASSA window_classe;
	window_classe.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_classe.lpfnWndProc = DefWindowProcA;
	window_classe.cbClsExtra = 0;
	window_classe.cbWndExtra = 0;
	window_classe.hInstance = GetModuleHandle(0);
	window_classe.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_classe.hCursor = LoadCursor(0, IDC_ARROW);
	window_classe.hbrBackground = 0;
	window_classe.hbrBackground = NULL;
	window_classe.lpszMenuName = NULL;
	window_classe.lpszClassName = "Dummy_jpegviewer";

	if (!RegisterClassA(&window_classe)) {
		printf("Failed to RegisterClassA: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	HWND dummy_window = CreateWindowExA(
		0,
		window_classe.lpszClassName,
		"Dummy OpenGL Window",
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		window_classe.hInstance,
		0);

	if (!dummy_window) {
		printf("Failed to CreateWindowExA dummy: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.cDepthBits = 0;

	HDC dummy_dc = GetDC(dummy_window);
	int pixel_formate = ChoosePixelFormat(dummy_dc, &pfd);
	if (!pixel_formate) {
		printf("Failed to ChoosePixelFormat: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}
	if (!SetPixelFormat(dummy_dc, pixel_formate, &pfd)) {
		printf("Failed to SetPixelFormat: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context) {
		printf("Failed to wglCreateContext dummy: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	if (!wglMakeCurrent(dummy_dc, dummy_context)) {
		printf("Failed to wglMakeCurrent dummy: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
		"wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
		"wglChoosePixelFormatARB");

	int pixel_format_attribs[] = {
	  WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
	  WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
	  WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
	  WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
	  WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
	  WGL_COLOR_BITS_ARB,         32,
	  WGL_DEPTH_BITS_ARB,         0,
	  WGL_STENCIL_BITS_ARB,       0,
	  WGL_SAMPLE_BUFFERS_ARB,     GL_FALSE,
	  WGL_SAMPLES_ARB,			  0,
	  0
	};

	int pixel_format;
	UINT num_formats;
	HDC real_dc = GetDC(window);
	wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	if (!num_formats) {
		printf("Failed to wglChoosePixelFormatARB: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	PIXELFORMATDESCRIPTOR pfde;
	DescribePixelFormat(real_dc, pixel_format, sizeof(pfde), &pfde);
	if (!SetPixelFormat(real_dc, pixel_format, &pfde)) {
		printf("Failed to SetPixelFormat: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	int gl33_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};

	HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
	if (!gl33_context) {
		printf("Failed to wglCreateContextAttribsARB: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	if (!wglMakeCurrent(real_dc, gl33_context)) {
		printf("Failed to wglMakeCurrent: %d.\n", GetLastError());
		scanf("press enter to exit.");
		return 1;
	}

	if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
		printf("Failed to gladLoadGLLoader.\n");
		scanf("press enter to exit.");
		return 1;
	}

	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = NULL;
	ImGui_ImplWin32_Init(window);
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
	int FULL_SCREEN_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

	MSG msg = { 0 };
	float zoomv = 2.f;
	char tmp[64] = { 0 };
	bool jpegcode = true;
	bool drawline = false;
	bool quadtree = false;
	JpegView* jpeg = NULL;
	GLuint image_textureo;
	GLuint image_texturef;
	int value = 90, blockn = 8;
	int depth = 10, error = 5;
	ShowWindow(window, TRUE);
	UpdateWindow(window);
	HDC gldc = GetDC(window);
	char openfile[260] = { 0 };
	char savefile[260] = { 0 };
	struct quadnode* root = NULL;
	int swidth, sheight, schannels;
	float Deltatime = 0, Lastedtime = 0;
	while (active)
	{
		if (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			continue;
		}

		if (touch[VK_ESCAPE])
			active = FALSE;

		Deltatime = float(GetTimeSinceStart() - Lastedtime);
		Lastedtime = (float)GetTimeSinceStart();

		sprintf(tmp, "JpegView - FPS: %1.0f", 1 / Deltatime);
		SetWindowTextA(window, tmp);

		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
		ImGui::Begin("Main", 0, FULL_SCREEN_FLAGS);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Open image"))
			{
				OPENFILENAMEA ofn;
				memset(openfile, 0, 260);
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = window;
				ofn.lpstrFile = openfile;
				ofn.nMaxFile = 260;
				ofn.lpstrFilter = "Image files (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
				if (GetOpenFileNameA(&ofn) == TRUE)
				{
					if (jpeg != NULL)
					{
						if (jpeg->original)
						{
							free(jpeg->original);
							free(jpeg->finalimage);
						}
					}
					uint8_t* img = stbi_load(openfile, &swidth, &sheight, &schannels, 3);
					if (img != NULL)
					{
						jpeg = initjpeg(img, swidth, sheight, value);
						root = initquad(img, swidth, 0, 0, swidth, sheight, 0);
						image_textureo = createimage(img, swidth, sheight);
						image_texturef = createimage(img, swidth, sheight);
					}
					else
						printf("Error in loading the image\n");
				}
			}
			if (openfile[0] != 0)
			{
				if (ImGui::MenuItem("Save image"))
				{
					OPENFILENAMEA ofn;
					memset(savefile, 0, 260);
					memset(&ofn, 0, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = window;
					ofn.lpstrFile = savefile;
					ofn.nMaxFile = 260;
					ofn.lpstrFilter = "Image file (*.png)\0*.png\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
					if (GetSaveFileNameA(&ofn) == TRUE)
					{
						stbi_write_png(savefile, jpeg->width, jpeg->height, 3, jpeg->finalimage, 0);
					}
				}
			}
			ImGui::EndMenuBar();
		}
		if (openfile[0] != 0 && jpeg != NULL)
		{
			ImGui::AlignTextToFramePadding();
			if (quadtree) jpegcode = false;
			ImGui::Checkbox("Use Jpeg?", &jpegcode); ImGui::SameLine();
			ImGui::Text("Factor:"); ImGui::SameLine();
			ImGui::InputInt("##inputf", &value); ImGui::SameLine();
			ImGui::Text("Block Size:"); ImGui::SameLine();
			ImGui::InputInt("##inputb", &blockn);
			ImGui::AlignTextToFramePadding();
			if (jpegcode) quadtree = false;
			ImGui::Checkbox("Use QuadTree?", &quadtree); ImGui::SameLine();
			ImGui::Text("Max Depth:"); ImGui::SameLine();
			ImGui::InputInt("##inputd", &depth); ImGui::SameLine();
			ImGui::Text("Max Error:"); ImGui::SameLine();
			ImGui::InputInt("##inpute", &error); ImGui::SameLine();
			ImGui::Checkbox("Draw Line Quad?", &drawline);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Zoom:"); ImGui::SameLine();
			ImGui::InputFloat("##inputz", &zoomv, 1, 100, "%g"); ImGui::SameLine();
			if (ImGui::Button("Compress"))
			{
				if (quadtree)
					renderquad(jpeg, root, depth, error, swidth, sheight, drawline, image_texturef);
				if (jpegcode)
					renderjpeg(jpeg, blockn, value, image_texturef);
			}
			if (zoomv < 1.f) zoomv = 1.f;
			float newwidth = ImGui::GetContentRegionAvail().x / 2;
			float newheight = ((float)jpeg->height / (float)jpeg->width) * newwidth;
			ImGui::Image((void*)(intptr_t)image_textureo, ImVec2(newwidth, newheight));
			zoomlayer(image_textureo, jpeg, newwidth, newheight, zoomv); ImGui::SameLine();
			ImGui::Image((void*)(intptr_t)image_texturef, ImVec2(newwidth - GImGui->Style.ItemSpacing.x, newheight));
			zoomlayer(image_texturef, jpeg, newwidth - GImGui->Style.ItemSpacing.x, newheight, zoomv);
		}

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SwapBuffers(gldc);
	}

	ImGui_ImplOpenGL3_Shutdown();
	wglDeleteContext(gl33_context);
	ImGui::DestroyContext();
	ImGui_ImplWin32_Shutdown();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gl33_context);
	ReleaseDC(window, gldc);
	DestroyWindow(window);
	UnregisterClassA("jpegviewer", window_class.hInstance);

	return (int)msg.wParam;
}