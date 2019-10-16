#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <string>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// Forward Declarations
static void initialState ();
static void mainMenu ();
static void showSettings (bool * p_open);
static void showAccounts (bool * p_open);
static void showAdvanced (bool * p_open);
static void showLedger (bool * p_open);
static void showPeers (bool * p_open);
static void createBlock (bool * p_open);
static void blockViewer (bool * p_open);
static void accountViewer (bool * p_open);
static void nodeStatistics (bool * p_open);

static void glfw_error_callback (int error, const char * description)
{
	fprintf (stderr, "Glfw Error %d: %s\n", error, description);
}

int main (int, char **)
{
	glfwSetErrorCallback (glfw_error_callback);
	if (!glfwInit ())
		return 1;

#if __APPLE__
	const char * glsl_version = "#version 150";
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
	const char * glsl_version = "#version 130";
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
	glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint (GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
	GLFWwindow * window = glfwCreateWindow (800, 600, "nano_wallet", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent (window);
	glfwSwapInterval (1);

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit () != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit () != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL () == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf (stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	IMGUI_CHECKVERSION ();
	ImGui::CreateContext ();
	ImGuiIO & io = ImGui::GetIO ();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// Setup Dear ImGui style
	ImGui::StyleColorsDark ();

	ImGui_ImplGlfw_InitForOpenGL (window, true);
	ImGui_ImplOpenGL3_Init (glsl_version);

	ImFont * font = io.Fonts->AddFontFromFileTTF ("../../deps/Fira_Mono/ttf/FiraMono-Medium.ttf", 16.0f);
	IM_ASSERT (font != NULL);

	static int accounts_state = 0;
	static int advanced_state = 0;
	static int menu_state = 0;
	static int settings_state = 0;
	static int scale_state = 1;
	
	ImVec4 clear_color = ImVec4 (0.45f, 0.55f, 0.60f, 1.00f);

	while (!glfwWindowShouldClose (window))
	{
		glfwPollEvents ();

		ImGui_ImplOpenGL3_NewFrame ();
		ImGui_ImplGlfw_NewFrame ();
		ImGui::NewFrame ();

		auto flags (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		auto flagsScroll (flags | ImGuiWindowFlags_AlwaysVerticalScrollbar);
		ImGui::SetNextWindowSizeConstraints (ImVec2 (800, 600), ImVec2 (800, 600));
		ImGui::SetNextWindowPos (ImVec2 (0, 0));
		ImGui::Begin ("Nano_wallet", 0, flags);
		ImGui::BeginGroup ();
		ImGui::BeginChild ("left pane", ImVec2 (150, 250), flags);

		ImGui::RadioButton ("Home", &menu_state, 0);
		ImGui::RadioButton ("Send", &menu_state, 1);
		ImGui::RadioButton ("Settings", &menu_state, 2);
		ImGui::RadioButton ("Accounts", &menu_state, 3);
		ImGui::RadioButton ("Advanced", &menu_state, 4);
		ImGui::EndChild ();
		ImGui::BeginChild ("testing spacing", ImVec2 (200, 0), flags);
		switch (menu_state)
		{
			case 2:
				ImGui::RadioButton ("Lock", &settings_state, 0);
				ImGui::RadioButton ("Set/Change Password", &settings_state, 1);
				ImGui::RadioButton ("Change Representative", &settings_state, 2);
				break;
			case 3:
				ImGui::RadioButton ("Use Account", &accounts_state, 0);
				ImGui::RadioButton ("Create Account", &accounts_state, 1);
				ImGui::RadioButton ("Import Wallet", &accounts_state, 2);
				ImGui::RadioButton ("View Wallet Seed", &accounts_state, 3);
				ImGui::RadioButton ("Import Adhoc", &accounts_state, 4);
				break;
			case 4:
				ImGui::RadioButton ("Ledger", &advanced_state, 0);
				ImGui::RadioButton ("Peers", &advanced_state, 1);
				ImGui::RadioButton ("Search for Receivable", &advanced_state, 2);
				ImGui::RadioButton ("Initiate Bootstrap", &advanced_state, 3);
				ImGui::RadioButton ("Refresh Wallet", &advanced_state, 4);
				ImGui::RadioButton ("Create Block", &advanced_state, 5);
				ImGui::RadioButton ("Enter Block", &advanced_state, 6);
				ImGui::RadioButton ("Block Viewer", &advanced_state, 7);
				ImGui::RadioButton ("Account Viewer", &advanced_state, 8);
				ImGui::RadioButton ("Node Statistics", &advanced_state, 9);
				ImGui::Text("Scale\t");
				ImGui::RadioButton ("Mnano", &scale_state, 30); ImGui::SameLine();
				ImGui::RadioButton ("knano", &scale_state, 27); 
				ImGui::RadioButton ("nano", &scale_state, 24); ImGui::SameLine();
				ImGui::RadioButton ("raw", &scale_state, 1); 
				auto scale ("1x10^" + std::to_string(scale_state) + " raw");
				ImGui::Text(scale.c_str());
				break;
		}
		ImGui::EndChild ();
		ImGui::EndGroup ();
		ImGui::SameLine ();
		ImGui::BeginChild ("right pane", ImVec2 (0, 0), flagsScroll);
		ImGui::EndChild ();
		ImGui::End ();

		// Rendering
		ImGui::Render ();
		int display_w, display_h;
		glfwMakeContextCurrent (window);
		glfwGetFramebufferSize (window, &display_w, &display_h);
		glViewport (0, 0, display_w, display_h);
		glClearColor (clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear (GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData ());

		glfwMakeContextCurrent (window);
		glfwSwapBuffers (window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown ();
	ImGui_ImplGlfw_Shutdown ();
	ImGui::DestroyContext ();

	glfwDestroyWindow (window);
	glfwTerminate ();

	return 0;
}

void showSettings (bool * p_open)
{
}
void showAccounts (bool * p_open)
{
}
void showAdvanced (bool * p_open)
{
}
void showLedger (bool * p_open)
{
}
void showPeers (bool * p_open)
{
}
void createBlock (bool * p_open)
{
}
void blockViewer (bool * p_open)
{
}
void accountViewer (bool * p_open)
{
}
void nodeStatistics (bool * p_open)
{
}