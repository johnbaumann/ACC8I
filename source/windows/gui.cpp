#include <stdio.h>
#include <stdint.h>
#include <iostream>

#include "windows/gui.h"
#include "shared/chip8_cpu.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#include "ImGuiFileDialog.h"


#define STB_IMAGE_IMPLEMENTATION
#include "windows/stb_image.h"

//bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

bool LoadTextureFromFile(const char*, GLuint*, int*, int*);
bool ScreenToTexture(struct chip8_cpu*, GLuint*);

int my_image_width = 0;
int my_image_height = 0;
GLuint my_image_texture = 0;
unsigned char* screen_data_buffer;
GLuint screen_texture;

bool cpu_auto_tick = true;


static bool use_work_area = false;
static ImGuiWindowFlags main_window_flags;

static ImGuiWindowFlags child_window_flags;

GLFWwindow* window;
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.298f, 1.00f);

static MemoryEditor mem_edit;

char file_path_to_open[MAX_PATH];

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


struct GUI_WindowStates
{

	bool show_keypad_window = true;
	bool show_memory_editor_window = false;
	bool show_register_window = true;
	bool show_render_window = true;
} gui_windowstates;


int8_t OpenROM(const char* filename)
{
	FILE* inFile = fopen(filename, "rb");
	long file_size;

	if (inFile == NULL)
	{
		printf("Error opening file!\n");
		return -1;
	}
	else
	{
		fseek(inFile, 0, SEEK_END);
		file_size = ftell(inFile);
		if (file_size > 3583)
		{
			printf("File size too large\n");
			return -1;
		}
		else if (file_size <= 0)
		{
			printf("File is empty\n");
			return -1;
		}
		rewind(inFile);

		//while (ftell(inFile) < file_size)
		for (int i = 0; i < file_size; i++)
		{
			int data_in = fgetc(inFile);
			if (data_in == EOF)
				break;
			c8_cpu.memory[i + 0x200] = (uint8_t)data_in;
		}

		c8_cpu.cpu_halted = false;
	}
	return 0;
}

void run_cpu_cycles()
{
	uint32_t cycles_to_execute;
	// Tick CPU
	// 500Hz
	cycles_to_execute = 500 / ImGui::GetIO().Framerate;
	if (cycles_to_execute == 0)
		cycles_to_execute = 1;

	if (!c8_cpu.cpu_halted)
	{
		for (uint32_t i = 0; i < cycles_to_execute; i++)
		{
			Chip8_TickCPU(&c8_cpu);
		}
	}
	// Tick CPU

	// Delay and Sound Timers
	// 60Hz
	cycles_to_execute = 60 / ImGui::GetIO().Framerate;
	if (cycles_to_execute == 0)
		cycles_to_execute = 1;

	if (!c8_cpu.cpu_halted)
	{
		for (uint32_t i = 0; i < cycles_to_execute; i++)
		{
			Chip8_UpdateTimers(&c8_cpu);
		}
	}
	// Delay and Sound Timers
}


void GUI_Closing()
{
	// Cleanup
	free(screen_data_buffer);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}


bool GUI_DrawAllWindows()
{


	if (cpu_auto_tick)
	{
		run_cpu_cycles();
	}


	ScreenToTexture(&c8_cpu, &my_image_texture);

	bool bProgramRunning = GUI_DrawMainWindowMenu();
	GUI_DrawKeyPadWindow();
	GUI_DrawRegisterWindow();
	GUI_DrawMemoryEditorWindow();
	GUI_DrawRenderWindow();

	GUI_DrawFileDialog();

	return bProgramRunning;
}

void GUI_DrawFileDialog()
{
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(320, 180), ImGui::GetMainViewport()->Size))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			Chip8_Initialize(&c8_cpu);
			OpenROM(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}
}

void GUI_DrawKeyPadWindow()
{
	if (gui_windowstates.show_keypad_window)
	{
		if (ImGui::Begin("Keypad", &gui_windowstates.show_keypad_window, child_window_flags))
		{
			// Row 1
			if (ImGui::Button("1"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x01);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x01);
			}
			ImGui::SameLine();
			if (ImGui::Button("2"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x02);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x02);
			}
			ImGui::SameLine();
			if (ImGui::Button("3"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x03);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x03);
			}
			ImGui::SameLine();
			if (ImGui::Button("C"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x0C);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x0C);
			}

			// Row 2
			if (ImGui::Button("4"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x04);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x04);
			}
			ImGui::SameLine();
			if (ImGui::Button("5"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x05);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x05);
			}
			ImGui::SameLine();
			if (ImGui::Button("6"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x06);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x06);
			}
			ImGui::SameLine();
			if (ImGui::Button("D"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x0D);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x0D);
			}

			// Row 3
			if (ImGui::Button("7"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x07);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x07);
			}
			ImGui::SameLine();
			if (ImGui::Button("8"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x08);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x08);
			}
			ImGui::SameLine();
			if (ImGui::Button("9"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x09);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x09);
			}
			ImGui::SameLine();
			if (ImGui::Button("E"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x0E);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x0E);
			}

			// New Row
			if (ImGui::Button("A"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x0A);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x0A);
			}
			ImGui::SameLine();
			if (ImGui::Button("0"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x00);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x00);
			}
			ImGui::SameLine();
			if (ImGui::Button("B"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x0B);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x0B);
			}
			ImGui::SameLine();
			if (ImGui::Button("F"))
			{
				Chip8_KeyPressed(&c8_cpu, 0x0F);
			}
			else
			{
				Chip8_KeyReleased(&c8_cpu, 0x0F);
			}

			ImGui::End();
		}
		else
		{
			ImGui::End();
		}
	}
}

void GUI_DrawMemoryEditorWindow()
{
	if (gui_windowstates.show_memory_editor_window)
	{
		ImGui::SetNextWindowPos(ImVec2(50, 30), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
		mem_edit.DrawWindow("Memory Editor", c8_cpu.memory, sizeof(c8_cpu.memory));
		gui_windowstates.show_memory_editor_window = mem_edit.Open;
	}
}

void GUI_DrawRegisterWindow()
{
	if (gui_windowstates.show_register_window)
	{
		ImGui::SetNextWindowPos(ImVec2(50, 30), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Registers", &gui_windowstates.show_register_window, child_window_flags))
		{
			ImGui::Text("PC = %i, 0x%X", c8_cpu.pc, c8_cpu.pc);
			ImGui::Text("Instruction = %X", c8_cpu.memory[c8_cpu.pc] << 8 | c8_cpu.memory[c8_cpu.pc + 1]);
			ImGui::Checkbox("CPU Running", &cpu_auto_tick);
			ImGui::SameLine();
			if (ImGui::Button("Tick"))
			{
				Chip8_TickCPU(&c8_cpu);
				Chip8_UpdateTimers(&c8_cpu);
			}
			if (ImGui::CollapsingHeader("V"))
			{
				for (int v = 0; v < 16; v++)
				{
					ImGui::Text("V%X: %i, 0x%X", v, c8_cpu.cpureg_V[v], c8_cpu.cpureg_V[v]);
				}
			}
			ImGui::Text("I: %i, 0x%X", c8_cpu.cpureg_I, c8_cpu.cpureg_I);
			ImGui::End();
		}
		else
		{
			ImGui::End();
		}
	}
}

bool GUI_DrawMainWindowMenu()
{
	bool bProgramRunning = true;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O"))
			{
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".ch8", ".");
			}
			if (ImGui::MenuItem("Close", "Ctrl+W")) { Chip8_Initialize(&c8_cpu); }
			if (ImGui::MenuItem("Exit", "Alt+F4")) { bProgramRunning = false; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Keypad", NULL, &gui_windowstates.show_keypad_window);
			ImGui::MenuItem("Registers", NULL, &gui_windowstates.show_register_window);
			ImGui::MenuItem("Memory Editor", NULL, &gui_windowstates.show_memory_editor_window);
			ImGui::MenuItem("Video Out", NULL, &gui_windowstates.show_render_window);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	else
	{
		ImGui::End();
	}

	return bProgramRunning;
}

void GUI_DrawRenderWindow()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (gui_windowstates.show_render_window)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(CHIP8_SCREEN_WIDTH + (ImGuiStyleVar_WindowBorderSize*2), CHIP8_SCREEN_HEIGHT + 35), ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::TwoToOneRatio);

		if (ImGui::Begin("Video Out", &gui_windowstates.show_render_window, child_window_flags))
		{
			ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(ImGui::GetWindowWidth() - 70, ImGui::GetWindowHeight() - 35));

			/*ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
			if (ImGui::Begin("Example: Simple overlay", (bool*)true, window_flags))
			{
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}
			else
			{
				ImGui::End();
			}*/

			ImGui::End();
		}
		else
		{
			ImGui::End();
		}
	}
}

int GUI_Init()
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif


	// Create window with graphics context
	window = glfwCreateWindow(1280, 720, "ACC8I", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
	bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
	glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
	glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);


	//bool ret = LoadTextureFromFile("MyImage01.jpg", &my_image_texture, &my_image_width, &my_image_height);
	screen_data_buffer = (unsigned char*)malloc(CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT * 3);
	glGenTextures(1, &screen_texture);

	return 0;
}

bool GUI_Render()
{
	ImGuiIO& io = ImGui::GetIO();

	if (glfwWindowShouldClose(window))
		return false;

	// Poll and handle events (inputs, window resize, etc.)
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	glfwPollEvents();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (!GUI_DrawAllWindows())
		return false;

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
	glFlush();

	return true;
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

bool ScreenToTexture(struct chip8_cpu* cpu, GLuint* out_texture)
{
	if (screen_data_buffer != NULL)
	{
		for (int i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT; i++)
		{
			int index = i * 3;

			screen_data_buffer[index] = (c8_cpu.screen[i] != 0) ? 0xFF : 0x00;
			screen_data_buffer[index + 1] = screen_data_buffer[index];
			screen_data_buffer[index + 2] = screen_data_buffer[index];
		}
	}
	else
	{
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, screen_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CHIP8_SCREEN_WIDTH, CHIP8_SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, screen_data_buffer);

	*out_texture = screen_texture;

	return true;
}