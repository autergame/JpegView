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

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
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
	bool qtablege = true;
	bool drawline = true;
	bool quadtree = false;
	bool quadtreepo2 = false;

	GLuint image_textureo;
	GLuint image_texturef;
	GLuint image_textureo_zoom;
	GLuint image_texturef_zoom;

	ImVec2 uv_min = ImVec2(0.f, 0.f);
	ImVec2 uv_max = ImVec2(1.f, 1.f);

	int quality = 90, block_size = 8, subsampling_index = 0;
	const char* subsampling_items[] = { 
		"4:4:4", "4:4:0", "4:2:2", "4:2:0", "4:1:1"
	};

	int max_depth = 50, threshold_error = 5;
	int min_size = 8, max_size = 32;

	int max_depthmax = 100, threshold_errormax = 100;
	int min_sizemax = 128, max_sizemax = 256;

	JpegView* jpeg = nullptr;
	int swidth, sheight, schannels;

	float Deltatime = 0, Lastedtime = 0;
	char windowTitle[64] = { '\0' };

	char openFile[MAX_PATH] = { '\0' };
	char saveFile[MAX_PATH] = { '\0' };
	char openFile_temp[MAX_PATH] = { '\0' };

#define TESTBIN
#if defined TESTBIN && defined _DEBUG
	const char* teststr = "C:\\Users\\autergame\\Pictures\\testpattern.png";
	uint8_t* img = stbi_load(teststr, &swidth, &sheight, &schannels, 3);
	if (img != nullptr)
	{
		memcpy(openFile, teststr, strlen(teststr));
		jpeg = init_jpeg(img, swidth, sheight, block_size);
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
				memset(openFile_temp, 0, MAX_PATH);

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = glfwWindowNative;
				ofn.lpstrFile = openFile_temp;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = 
					"Image Files (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0"
					"JPG Image (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0"
					"PNG Image (*.png)\0*.png\0"
					"BMP Image (*.bmp)\0*.bmp\0"
					"All Files (*.*)\0*.*\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = nullptr;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = nullptr;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileNameA(&ofn) == TRUE)
				{
					if (openFile_temp[0] != '\0')
					{
						memset(openFile, 0, MAX_PATH);
						memcpy(openFile, openFile_temp, MAX_PATH);

						if (jpeg != nullptr)
						{
							deletemod(&jpeg->original_image);
							deletemod(&jpeg->final_image);
							for (int i = 0; i < 3; i++)
								deletemod(&jpeg->YCbCr[i]);
							deletemod(&jpeg->YCbCr);
							deletemod(&jpeg);

							glDeleteTextures(1, &image_textureo);
							glDeleteTextures(1, &image_texturef);
							glDeleteTextures(1, &image_textureo_zoom);
							glDeleteTextures(1, &image_texturef_zoom);
						}

						uint8_t* img = stbi_load(openFile, &swidth, &sheight, &schannels, 3);
						if (img != nullptr)
						{
							jpeg = init_jpeg(img, swidth, sheight, block_size);
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

					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = glfwWindowNative;
					ofn.lpstrFile = saveFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "PNG Image (*.png)\0*.png\0";
					ofn.lpstrDefExt = "png";
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
					if (jpeg != nullptr)
					{
						memset(openFile, 0, MAX_PATH);

						deletemod(&jpeg->original_image);
						deletemod(&jpeg->final_image);
						for (int i = 0; i < 3; i++)
							deletemod(&jpeg->YCbCr[i]);
						deletemod(&jpeg->YCbCr);
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
				ImGui::DragInt("##inputb", &block_size, 2.f, 2, 256);
				ImGui::Text("Chroma Subsampling:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(firstcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::Combo("##list", &subsampling_index, subsampling_items, IM_ARRAYSIZE(subsampling_items));
				ImGui::Checkbox("Use Generated Quantization Table?", &qtablege);
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

			ImGui::AlignTextToFramePadding();
			ImGui::Checkbox("Use QuadTree?", &quadtree);
			if (quadtree)
			{
				ImGui::Indent();
				float secondcolumn = ImGui::GetColumnWidth(0) + (ImGui::GetColumnWidth(1) * 0.90f);
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
				ImGui::DragInt("##inputmins", &min_size, 2.f, 2, min_sizemax);
				ImGui::AlignTextToFramePadding(); ImGui::Bullet();
				ImGui::Text("Max Quad Size:"); ImGui::SameLine();
				ImGui::SetNextItemWidth(secondcolumn - GImGui->CurrentWindow->DC.CursorPos.x);
				ImGui::DragInt("##inputmaxs", &max_size, 2.f, 4, max_sizemax);			
				ImGui::Checkbox("Use Quad Size Power Of 2", &quadtreepo2);
				ImGui::Checkbox("Draw Quadrant Line?", &drawline);
				ImGui::Unindent();
			}

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
			if (max_size >= max_sizemax)
			{
				max_sizemax += 100;
				max_size -= 10;
			}

			ImGui::Columns();

			ImGui::Separator();

			if (ImGui::Button("Compress", ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
			{
				if (quadtree && jpegcomp)
				{
					render_quadtree_jpeg(jpeg, max_depth, threshold_error,
						min_size, max_size, drawline, quality, qtablege, subsampling_index);
				}
				else 
				{
					if (quadtree)
						render_quadtree(jpeg, max_depth, threshold_error,
							min_size, max_size, drawline, quadtreepo2);
					if (jpegcomp)
						render_jpeg(jpeg, block_size, quality, qtablege, subsampling_index);
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

	if (jpeg != nullptr)
	{
		deletemod(&jpeg->original_image);
		deletemod(&jpeg->final_image);
		for (int i = 0; i < 3; i++)
			deletemod(&jpeg->YCbCr[i]);
		deletemod(&jpeg->YCbCr);
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

	return 0;
}