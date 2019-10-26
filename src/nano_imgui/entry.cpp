#include "nano_imgui.h"

#include <iostream>

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

static void glfw_error_callback (int error, const char * description)
{
	fprintf (stderr, "Glfw Error %d: %s\n", error, description);
}

int main (int argc, char * argv[])
{
	constexpr unsigned version_major = 1;
	constexpr unsigned version_minor = 0;
	constexpr unsigned version_patch = 0;
	std::string version_string = fmt::format ("{}.{}.{}", version_major, version_minor, version_patch);
	std::string version_string_full = fmt::format ("{}.{}.{}, Built {}", version_major, version_minor, version_patch, __DATE__);

	boost::program_options::options_description options ("Command line options", 160);
	options.add_options () ("help", "Print out options");
	options.add_options () ("config_path", boost::program_options::value<std::string> ()->default_value ("./config-nano-imgui.toml"), "Path to the optional configuration file, including the file name");
	options.add_options () ("config", boost::program_options::value<std::vector<std::string>> ()->multitoken (), "Pass configuration values. This takes precedence over any values in the configuration file. This option can be repeated multiple times.");
	options.add_options () ("generate_config", "Write configuration to stdout, populated with commented defaults.");
	boost::program_options::variables_map vm;
	boost::program_options::store (boost::program_options::command_line_parser (argc, argv).options (options).allow_unregistered ().run (), vm);
	boost::program_options::notify (vm);
	std::vector<std::string> config_overrides;

	if (vm.count ("help") != 0)
	{
		std::cout << options << std::endl;
		std::exit (0);
	}
	if (vm.count ("generate_config"))
	{
		nano_imgui::config conf;
		std::cout << conf.export_documented () << std::endl;
		std::exit (0);
	}
	if (vm.count ("config"))
	{
		config_overrides = vm["config"].as<std::vector<std::string>> ();
	}

	// Convert from posix path format to native
	std::string config_path (vm["config_path"].as<std::string> ());
	boost::filesystem::path config_path_parsed (config_path);
	nano_imgui::config conf (config_path_parsed.string (), config_overrides);

	// Log configuration
	auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt> ("nano-imgui.log", 1024 * 1024 * 5, 5);
	auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt> ();
	auto logger = std::make_shared<spdlog::logger> ("logger");
	logger->sinks ().push_back (rotating_sink);
	if (conf.wallet.log_to_stderr)
	{
		logger->sinks ().push_back (console_sink);
	}
	logger->flush_on (spdlog::level::info);

	if (conf.config_file_exists (config_path))
	{
		logger->info ("Config file loaded successfully: {}", config_path);
	}
	else
	{
		logger->info ("Config file not found, using defaults");
	}

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

	static int menu_state = 0;
	static int scale_state = 0;

	ImVec4 clear_color = ImVec4 (0.45f, 0.55f, 0.60f, 1.00f);

	while (!glfwWindowShouldClose (window))
	{
		glfwPollEvents ();

		ImGui_ImplOpenGL3_NewFrame ();
		ImGui_ImplGlfw_NewFrame ();
		ImGui::NewFrame ();
		auto flags (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		ImGui::SetNextWindowSizeConstraints (ImVec2 (800, 600), ImVec2 (800, 600));
		ImGui::SetNextWindowPos (ImVec2 (0, 0));
		ImGui::Begin ("Nano_wallet", 0, flags);
		ImGui::BeginChild ("Heder", ImVec2 (0, 75));
		char buf[128];
		sprintf_s (buf, "Status: %s, Blocks %i", (menu_state == 0 ? "Disconnected" : menu_state == 1 ? "Connected" : menu_state == 2 ? "Generating_Work" : "Syncing"), ImGui::GetFrameCount ());
		ImGui::Text (buf);
		ImGui::EndChild ();
		ImGui::BeginGroup ();
		ImGui::BeginChild ("left pane", ImVec2 (200, 300), ImGuiWindowFlags_None);
		ImGui::RadioButton ("Home", &menu_state, 0);
		ImGui::RadioButton ("Send", &menu_state, 1);
		ImGui::RadioButton ("Settings", &menu_state, 2);
		ImGui::RadioButton ("Accounts", &menu_state, 3);
		ImGui::RadioButton ("Advanced", &menu_state, 4);
		ImGui::EndChild ();
		ImGui::BeginChild ("left pane2", ImVec2 (200, 0), ImGuiWindowFlags_None);
		ImGui::Text ("Scale");
		ImGui::RadioButton ("nano", &scale_state, 30);
		ImGui::SameLine ();
		ImGui::RadioButton ("cnano", &scale_state, 28);
		ImGui::RadioButton ("piconano", &scale_state, 24);
		ImGui::SameLine ();
		ImGui::RadioButton ("plank", &scale_state, 0);
		ImGui::Text ("1x10^%i plank", scale_state);
		ImGui::EndChild ();
		ImGui::EndGroup ();

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
