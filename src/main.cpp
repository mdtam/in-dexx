// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include "boyer-moore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "portable-file-dialogs.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

boyerMoore bm;

// Directory recursive
std::vector<std::string> readDirectory(std::string directory) {
    // travel thru a directory gathering all the file and directory naems
    vector<string> fileList;
    DIR* dir;
    struct dirent* ent;

    // open a directory
    if ((dir = opendir(directory.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL)  // loop until the directory is traveled thru
        {
            // push directory or filename to the list
            fileList.push_back(ent->d_name);
        }
        // close up
        closedir(dir);
    }
    // return the filelust
    return fileList;
}

void readDirectoryRecursive(string directory, vector<string>* fullList) {
    // get the "root" directory's directories
    vector<string> fileList = readDirectory(directory);

    // loop thru the list
    for (vector<string>::iterator i = fileList.begin(); i != fileList.end(); ++i) {
        // test for . and .. directories (this and back)
        if (strcmp((*i).c_str(), ".") &&
            strcmp((*i).c_str(), "..")) {
            // i use stringstream here, not string = foo; string.append(bar);
            stringstream fullname;
            fullname << directory << "/" << (*i);

            fullList->push_back(fullname.str());
            bm.fileIn(fullname.str());
            // readDirectoryRecursive(fullname.str(), fullList);
        }
    }
}

vector<string> targetFiles;
vector<LLI> fileMatches;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

char Target_Dir[3000] = "";
char Search_Phrase[50000] = "";

static void ShowFullscreenUI(bool* p_open) {
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (ImGui::Begin("In-Dexx Search", p_open, flags)) {
        ImGui::InputTextWithHint("Target Directory", "your/path/goes/here", Target_Dir, IM_ARRAYSIZE(Target_Dir));
        ImGui::SameLine();
        if (ImGui::Button("Select...")) {
            auto dir = pfd::select_folder("Select any directory", pfd::path::home()).result();
            for (int i = 0; i < IM_ARRAYSIZE(Target_Dir) && dir.size() > 0; i++) {
                if (i < (int)dir.size()) {
                    Target_Dir[i] = dir[i];
                } else {
                    Target_Dir[i] = '\0';
                }
            }
        }

        ImGui::InputTextWithHint("Search Phrase", "to be or not to be", Search_Phrase, IM_ARRAYSIZE(Search_Phrase));
        ImGui::SameLine();
        if (ImGui::Button("Search")) {
            if (Target_Dir[0] == '\0') {
                ImGui::OpenPopup("empty_target_dir");
            } else if (Search_Phrase[0] == '\0') {
                ImGui::OpenPopup("empty_search");
            } else {
                // search
                bm.clean();
                targetFiles.clear();
                fileMatches.clear();
                bm.patIn(Search_Phrase);
                readDirectoryRecursive(Target_Dir, &targetFiles);

                bm.search(&fileMatches);
            }
        }
        if (ImGui::BeginPopup("empty_target_dir")) {
            ImGui::Text("Target Directory is Empty");
            ImGui::Separator();
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup("empty_search")) {
            ImGui::Text("Search Phrase is Empty");
            ImGui::EndPopup();
        }

        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        if (ImGui::BeginTable("table1", 2, flags)) {
            ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Matches", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            for (int row = 0; row < (int)targetFiles.size(); row++) {
                if (fileMatches[row]) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(targetFiles[row].c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(to_string(fileMatches[row]).c_str());
                }
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("Close"))
            *p_open = false;

        bool show_demo_window = false;
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    ImGui::End();
}

int main(int, char**) {
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
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    // --- In-Dexx code starts here!

    // Our state
    bool show_ui_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        if (show_ui_window) {
            ShowFullscreenUI(&show_ui_window);
        } else {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
