//author https://github.com/autergame
#include "JpegView.h"
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LARGE_INTEGER Frequencye, Starte;
	QueryPerformanceFrequency(&Frequencye);
	QueryPerformanceCounter(&Starte);

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
	glfwSwapInterval(0);

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
	bool drawline = true;
	bool quadtree = false;

	GLuint image_textureo;
	GLuint image_texturef;
	GLuint image_textureo_zoom;
	GLuint image_texturef_zoom;

	ImVec2 uv_min = ImVec2(0.f, 0.f);
	ImVec2 uv_max = ImVec2(1.f, 1.f);

	int quality = 90, block_size = 8;
	int max_depth = 10, threshold_error = 5, min_size = 8;

	int max_depthmax = 100, threshold_errormax = 100, min_sizemax = 100;

	JpegView* jpeg = nullptr;
	int swidth, sheight, schannels;

	float Deltatime = 0, Lastedtime = 0;
	char windowTitle[64] = { '\0' };

	char openFile[MAX_PATH] = { '\0' };
	char saveFile[MAX_PATH] = { '\0' };

#define TESTBIN
#if defined TESTBIN && defined _DEBUG
	const char* teststr = "C:\\Users\\autergame\\Pictures\\testpattern.png";
	uint8_t* img = stbi_load(teststr, &swidth, &sheight, &schannels, 3);
	if (img != nullptr)
	{
		memcpy(openFile, teststr, strlen(teststr));
		jpeg = init_jpeg(img, swidth, sheight);
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
				memset(openFile, 0, MAX_PATH);

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = glfwWindowNative;
				ofn.lpstrFile = openFile;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = "Image files (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = nullptr;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = nullptr;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileNameA(&ofn) == TRUE)
				{
					if (openFile[0] != '\0')
					{
						if (jpeg != nullptr)
						{
							if (jpeg->original_image)
							{
								deletemod(&jpeg->original_image);
								deletemod(&jpeg->final_image);
								for (int i = 0; i < 3; i++)
									deletemod(&jpeg->YCbCr[i]);
								deletemod(&jpeg->YCbCr);
								deletemod(&jpeg);
							}
						}

						uint8_t* img = stbi_load(openFile, &swidth, &sheight, &schannels, 3);
						if (img != nullptr)
						{
							jpeg = init_jpeg(img, swidth, sheight);
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
			if (openFile[0] != 0)
			{
				if (ImGui::MenuItem("Save image"))
				{
					OPENFILENAMEA ofn;
					memset(&ofn, 0, sizeof(ofn));
					memset(saveFile, 0, MAX_PATH);

					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = glfwWindowNative;
					ofn.lpstrFile = saveFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "Image file (*.png)\0*.png\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = nullptr;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = nullptr;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
					if (GetSaveFileNameA(&ofn) == TRUE)
					{
						stbi_write_png(saveFile, jpeg->width, jpeg->height, 3, jpeg->final_image, 0);
					}
				}
			}
			ImGui::EndMenuBar();
		}
		if (openFile[0] != 0 && jpeg != nullptr)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use Jpeg?", &jpegcomp); ImGui::SameLine();
			ImGui::Text("Factor:"); ImGui::SameLine();
			ImGui::DragInt("##inputf", &quality, 1.f, 1, 100); ImGui::SameLine();
			ImGui::Text("Block Size:"); ImGui::SameLine();
			ImGui::DragInt("##inputb", &block_size, 1.f, 1, 256);
			ImGui::AlignTextToFramePadding(); ImGui::Bullet();
			ImGui::Checkbox("Show Compression Rate?", &jpeg->compression_rate); ImGui::SameLine();
			ImGui::Text("Quality Start:"); ImGui::SameLine();
			ImGui::DragInt("##inputc", &jpeg->quality_start, 1.f, 1, 100);
			if (jpegcomp)
				quadtree = false;

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use QuadTree?", &quadtree); ImGui::SameLine();
			ImGui::Text("Max Depth:"); ImGui::SameLine();
			ImGui::DragInt("##inputd", &max_depth, 1.f, 0, max_depthmax); ImGui::SameLine();
			ImGui::Text("Error Threshold:"); ImGui::SameLine();
			ImGui::DragInt("##inpute", &threshold_error, 1.f, 0, threshold_errormax);
			ImGui::AlignTextToFramePadding(); ImGui::Bullet();
			ImGui::Text("Min Size:"); ImGui::SameLine();
			ImGui::DragInt("##inputs", &min_size, 1.f, 0, min_sizemax); ImGui::SameLine();
			ImGui::Checkbox("Draw Line Quad?", &drawline);

			jpegcomp = !quadtree;

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
			if (min_size >= min_sizemax)
			{
				min_sizemax += 100;
				min_size -= 10;
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("##zoomcheck", &usezoom); ImGui::SameLine();
			ImGui::Text("Zoom:"); ImGui::SameLine();
			ImGui::DragFloat("##inputz", &zoomv, 1.f, 1.f, zoomvmax, "%g"); ImGui::SameLine();
			ImGui::Text("Lupe size:"); ImGui::SameLine();
			ImGui::DragFloat("##inputl", &magnifiersize, 1.f, 10.f, magnifiersizemax, "%g");

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

			ImGui::SameLine();
			if (ImGui::Button("Compress"))
			{
				if (quadtree && jpegcomp)
				{

				}
				else 
				{
					if (quadtree)
						render_quadtree(jpeg, max_depth, threshold_error, min_size, drawline, image_texturef, image_texturef_zoom);
					if (jpegcomp)
						render_jpeg(jpeg, block_size, quality, image_texturef, image_texturef_zoom);
				}
			}

			float newwidth = ImGui::GetContentRegionAvail().x / 2.f - GImGui->Style.ItemSpacing.x;
			float newheight = newwidth * ((float)jpeg->height / (float)jpeg->width);
			
			bool usescrollbool = false;
			ImGui::Image((void*)(intptr_t)image_textureo, ImVec2(newwidth, newheight),
				uv_min, uv_max, tint_col, border_col);
			if (usezoom)
				usescrollbool = zoom_layer(image_textureo_zoom, jpeg, zoomv, magnifiersize, windowWidth, windowHeight);

			ImGui::SameLine();
			ImGui::Image((void*)(intptr_t)image_texturef, ImVec2(newwidth, newheight),
				uv_min, uv_max, tint_col, border_col);
			if (usezoom)
				usescrollbool = zoom_layer(image_texturef_zoom, jpeg, zoomv, magnifiersize, windowWidth, windowHeight);

			if (usescrollbool)
				usescroll = ImGuiWindowFlags_NoScrollWithMouse;
			else
				usescroll = ImGuiWindowFlags_None;
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(glfwWindow);
	}

	if (jpeg != nullptr)
	{
		if (jpeg->original_image)
		{
			deletemod(&jpeg->original_image);
			deletemod(&jpeg->final_image);
			for (int i = 0; i < 3; i++)
				deletemod(&jpeg->YCbCr[i]);
			deletemod(&jpeg->YCbCr);
			deletemod(&jpeg);
		}
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfwWindow);
	glfwTerminate();

	return 0;
}