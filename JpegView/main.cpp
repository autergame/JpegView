//author https://github.com/autergame
#include "JpegView.h"

double GetTimeSinceStart(LARGE_INTEGER Frequencye, LARGE_INTEGER Starte)
{
	LARGE_INTEGER t_End;
	QueryPerformanceCounter(&t_End);
	return (float)(t_End.QuadPart - Starte.QuadPart) / Frequencye.QuadPart;
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	scanf_s("press enter to exit.");
	exit(1);
}

int main()
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
	int FULL_SCREEN_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

	float zoomv = 2.f;
	float magnifiersize = 200.f;

	bool jpegcode = true;
	bool drawline = false;
	bool quadtree = false;

	GLuint image_textureo;
	GLuint image_texturef;

	int value = 90, blockn = 8;
	int depth = 10, error = 5;

	JpegView* jpeg = nullptr;
    quadnode* root = nullptr;
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
		jpeg = initjpeg(img, swidth, sheight);
		root = initquad(img, swidth, 0, 0, swidth, sheight, 0);
		image_textureo = createimage(img, swidth, sheight);
		image_texturef = createimage(jpeg->finalimage, swidth, sheight);
	}
	else
		printf("Error in loading the image\n");
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
		ImGui::Begin("Main", 0, FULL_SCREEN_FLAGS);
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
							if (jpeg->originalimage)
							{
								delete jpeg->originalimage;
								delete jpeg->finalimage;
								delete jpeg->red;
								delete jpeg->green;
								delete jpeg->blue;
								delete jpeg->DCTCosTable;
							}
						}
						uint8_t* img = stbi_load(openFile, &swidth, &sheight, &schannels, 3);
						if (img != nullptr)
						{
							jpeg = initjpeg(img, swidth, sheight);
							root = initquad(img, swidth, 0, 0, swidth, sheight, 0);
							image_textureo = createimage(img, swidth, sheight);
							image_texturef = createimage(jpeg->finalimage, swidth, sheight);
						}
						else
							printf("Error in loading the image\n");
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
						stbi_write_png(saveFile, jpeg->width, jpeg->height, 3, jpeg->finalimage, 0);
					}
				}
			}
			ImGui::EndMenuBar();
		}
		if (openFile[0] != 0 && jpeg != nullptr)
		{
			ImGui::AlignTextToFramePadding();

			if (quadtree) 
				jpegcode = false;

			ImGui::Checkbox("Use Jpeg?", &jpegcode); ImGui::SameLine();
			ImGui::Text("Factor:"); ImGui::SameLine();
			ImGui::InputInt("##inputf", &value); ImGui::SameLine();
			ImGui::Text("Block Size:"); ImGui::SameLine();
			ImGui::InputInt("##inputb", &blockn);
			ImGui::AlignTextToFramePadding();

			if (jpegcode) 
				quadtree = false;

			ImGui::Checkbox("Use QuadTree?", &quadtree); ImGui::SameLine();
			ImGui::Text("Max Depth:"); ImGui::SameLine();
			ImGui::InputInt("##inputd", &depth); ImGui::SameLine();
			ImGui::Text("Max Error:"); ImGui::SameLine();
			ImGui::InputInt("##inpute", &error); ImGui::SameLine();
			ImGui::Checkbox("Draw Line Quad?", &drawline);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Zoom:"); ImGui::SameLine();
			ImGui::SliderFloat("##inputz", &zoomv, 1, 100, "%g"); ImGui::SameLine();
			ImGui::Text("Lupe size:"); ImGui::SameLine();
			ImGui::SliderFloat("##inputl", &magnifiersize, 10, 1000, "%g"); ImGui::SameLine();
			if (ImGui::Button("Compress"))
			{
				if (quadtree)
					renderquad(jpeg, root, depth, error, swidth, sheight, drawline, image_texturef);
				if (jpegcode)
					renderjpeg(jpeg, blockn, value, image_texturef);
			}

			float newwidth = ImGui::GetContentRegionAvail().x / 2;
			float newheight = ((float)jpeg->height / (float)jpeg->width) * newwidth;

			ImGui::Image((void*)(intptr_t)image_textureo, ImVec2(newwidth, newheight));
			zoomlayer(image_textureo, jpeg, zoomv, magnifiersize);

			ImGui::SameLine();
			ImGui::Image((void*)(intptr_t)image_texturef, ImVec2(newwidth - GImGui->Style.ItemSpacing.x, newheight));
			zoomlayer(image_texturef, jpeg, zoomv, magnifiersize);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(glfwWindow);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfwWindow);
	glfwTerminate();

	if (jpeg->originalimage)
	{
		delete jpeg->originalimage;
		delete jpeg->finalimage;
		delete jpeg->red;
		delete jpeg->green;
		delete jpeg->blue;
		delete jpeg->DCTCosTable;
	}

	return 0;
}