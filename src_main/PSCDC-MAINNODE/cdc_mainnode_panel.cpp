// cdc_mainnode_panel.
#include <iostream>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <iomanip>

#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>

#include "cdc_mainnode_panel.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(_MSV_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib,"legacy_stdio_definitons.lib")
#endif

using namespace std;

// USE NVIDIA GPU.
extern "C" {
    _declspec(dllexport)
        unsigned long NvOptimusEnablement = 0x00000001;
}

bool EXIT_MAINEVENT_LOOP = false;

// Time [xxxx.xx.xx.xx:xx:xx:xx ms].
string __get_current_time() {
    auto timenow = chrono::system_clock::now();
    auto timemillisecond = chrono::duration_cast<chrono::milliseconds>(timenow.time_since_epoch()) % 1000;
    auto t = chrono::system_clock::to_time_t(timenow);
    tm _time = {};
#ifdef _WIN32
    localtime_s(&_time, &t);
#else
    localtime_r(&t, &_time);
#endif
    stringstream _sstream;
    _sstream << put_time(&_time, "[%Y.%m.%d %H:%M:%S") << " " << setfill('0') << setw(3)
        << timemillisecond.count() << " ms]";

    return _sstream.str();
}

namespace GLFW_IMGUI_PANEL {
    ImGuiWindowFlags WindowFlags = NULL;
    bool             NoClose     = false;
    
    void stringcpychar(const string& src, char* dst, size_t dstsz) {
        size_t scount = NULL;

        if (src.size() <= dstsz) {
            for (const auto data : src) {

                dst[scount] = data;
                ++scount;
            }
        }
    }
    /*
    * 代码查看面板 CL程序 & 配置文件.
    */
    void gui_assembly_codeview() {

        ImGui::Begin("CL Kernel Code", &NoClose, WindowFlags);
        {
            ImGui::Text("OpenCL 3.0 (CL.kernel) (CL.config) 5000 character.");
            ImGui::Button(u8"CL程序");

            char temp[TEXT_FILE_LENGTH] = {};
            stringcpychar(CalcProgram::cl_kernel_program, temp, TEXT_FILE_LENGTH);

            ImGui::InputTextMultiline(
                "",
                temp,
                TEXT_FILE_LENGTH,
                ImVec2(490.0f, 487.6f)
            );

            CalcProgram::cl_kernel_program = temp;
            ImGui::Text("[char]: %d", CalcProgram::cl_kernel_program.size());
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 75.0f);

        EXIT_MAINEVENT_LOOP = ImGui::Button(u8"关闭主机");
        ImGui::End();
    }

    /*
    * 节点管理面板 分布式计算节点控制.
    */
    void gui_assembly_mainpanel() {
        static bool WindowNoMove = true;
        
        ImGui::Begin("NodePanel", &NoClose, WindowFlags);
        ImGui::Text("@PomeloStarStudio 2023-2024. RCSZ.");

        ImGui::Text(u8"当前任务数据队列: %d", CALC_DATA_INDEX::TaskDataFile.size());

        ImGui::Checkbox(u8"开启实时节点状态监控.", &CalcNodes::calcnode_global_operations[0]);

        ImGui::SameLine();
        CalcNodes::calcnode_global_operations[1] = ImGui::Button(u8"刷新所有节点状态");

        CalcNodes::calcnode_global_operations[2] = ImGui::Button(u8"任务部署");

        ImGui::SameLine();
        CalcNodes::calcnode_global_operations[3] = ImGui::Button(u8"开始计算");

        ImGui::SameLine();
        CalcNodes::calcnode_global_operations[4] = ImGui::Button(u8"提取结果");

        static char filetext_temp[128] = {};

        ImGui::Text("File Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(240);
        ImGui::InputText("", filetext_temp, 128);

        ImGui::SameLine();
        if (ImGui::Button(u8"添加任务数据")) {

            if (filesystem::exists(filetext_temp))
                CALC_DATA_INDEX::TaskDataFile.push(string(filetext_temp));
            else
                cout << TIEM_CURRENT << PANEL_INFOHEAD << "Push task invalid filename." << endl;
        }
        
        for (size_t i = 0; i < CalcNodes::calcnodes_state.size(); ++i) {
            string TitleTemp = CalcNodes::calcnodes_state[i].node_name + " " + to_string(i);

            if (ImGui::CollapsingHeader(TitleTemp.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

                ImGui::PushID((int32_t)i);
                {   
                    if (ImGui::Button(u8"关闭计算节点")) {

                        CalcNodes::calcnodes_state[i].oper_close = true;
                        cout << TIEM_CURRENT << PANEL_INFOHEAD << "Close calculate node server." << endl;
                    }
                    else
                        CalcNodes::calcnodes_state[i].oper_close = false;

                    ImGui::SameLine();
                    CalcNodes::calcnodes_state[i].oper_connect = ImGui::Button(u8"连接计算节点");
                    
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputText(u8"计算节点地址 IPv4", CalcNodes::calcnodes_state[i].node_server_address, ADDRESS_CHAR_LENv4);

                    ImGui::SetNextItemWidth(200);
                    ImGui::InputInt(u8"计算节点端口 U16T", &CalcNodes::calcnodes_state[i].node_server_port);

                    if (ImGui::Button(u8"删除节点")) {

                        CalcNodes::calcnodes_state.erase(CalcNodes::calcnodes_state.begin() + i);
                        cout << TIEM_CURRENT << PANEL_INFOHEAD << "Delete calculate node." << endl;
                    }

                    if (CalcNodes::calcnodes_state[i].node_server_connect) ImGui::Text(u8"> 节点连接状态: 已连接.");
                    else                                                   ImGui::Text(u8"> 节点连接状态: 未连接.");

                    switch (CalcNodes::calcnodes_state[i].node_state) 
                    {
                    case(NULL):            { ImGui::Text(u8"> 节点计算状态: 未知."); break; }
                    case(NODE_STATE_IDLE): { ImGui::Text(u8"> 节点计算状态: 空闲中."); break; }
                    case(NODE_STATE_DTOK): { ImGui::Text(u8"> 节点计算状态: 准备就绪."); break; }
                    case(NODE_STATE_CALC): { ImGui::Text(u8"> 节点计算状态: 计算中."); break; }
                    case(NODE_STATE_WAIT): { ImGui::Text(u8"> 节点计算状态: 等待中."); break; }
                    }

                    ImGui::Text(u8"> 节点计算设备信息(Device Information):");
                    ImGui::TextWrapped(CalcNodes::calcnodes_state[i].node_device_info.c_str());
                }
                ImGui::PopID();

                CalcNodes::calcnodes_state[i].node_server_port = 
                    (CalcNodes::calcnodes_state[i].node_server_port < 1) ? 1 :
                    (CalcNodes::calcnodes_state[i].node_server_port > 65535) ? 65535 : CalcNodes::calcnodes_state[i].node_server_port;
            }
        }

        ImGui::Text(""); // '\n'
        if (ImGui::Button(u8"添加计算节点")) {

            CalcNodes::calcnode_push(CalcNodes::calcnodes_state);
            cout << TIEM_CURRENT << PANEL_INFOHEAD << "Addpush calculate node." << endl;
        }

        ImGui::Checkbox("Fixed WindowAssembly.", &WindowNoMove);
        if (WindowNoMove) {
            WindowFlags = ImGuiWindowFlags_NoMove;    // 固定位置.
            WindowFlags |= ImGuiWindowFlags_NoResize; // 固定大小.
            WindowFlags |= ImGuiWindowFlags_NoCollapse;
        }
        else
            WindowFlags = NULL;

        ImGui::End();
    }

    void draw_imgui_panel() {

        // start the ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // server Imgui GUI panel.
        gui_assembly_mainpanel();
        gui_assembly_codeview();

        // render imgui.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

// get glfw system error.
static void GLFW_ErrorCallback(int error, const char* description) {
    // glfw callback error print.
    fprintf(stderr, "[GLFWsystem]: GLFW Error %d: %s\n", error, description);
}

void imgui_system_init(GLFWwindow* winobj, const char* fonts_path) {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& GUIIO = ImGui::GetIO(); (void)GUIIO;
    GUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls.
    GUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls.

    // Setup ImGui style.
    ImGui::StyleColorsDark();
    {
        ImGuiStyle* config_style = &ImGui::GetStyle();
        ImVec4* config_colors = config_style->Colors;

        config_colors[ImGuiCol_Text]          = ImVec4(0.8f, 1.0f, 0.8f, 0.85f);
        config_colors[ImGuiCol_TitleBg]       = ImVec4(0.0f, 1.0f, 1.0f, 0.32f);
        config_colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 1.0f, 1.0f, 0.32f);
    }
    // Init SetFonts.
    auto GUI_TEXT_FONTS = ImGui::GetIO().Fonts;
    GUI_TEXT_FONTS->AddFontFromFileTTF(
        fonts_path,
        35.0f,
        NULL,
        GUI_TEXT_FONTS->GetGlyphRangesChineseFull()
    );
    ImGui::GetIO().FontGlobalScale = 0.5f;

    // 圆角窗口.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.2f);

    // Setup platform / Renderer backends.
    ImGui_ImplGlfw_InitForOpenGL(winobj, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    cout << TIEM_CURRENT << PANEL_INFOHEAD << "ImGui init." << endl;
}

void opengl_panel_init(
    const char* window_title, 
    uint32_t window_szx, 
    uint32_t window_szy,
    std::function<void()> render
) {
    // GLFW init.
    glfwSetErrorCallback(GLFW_ErrorCallback);
    if (!glfwInit())
        cout << TIEM_CURRENT << PANEL_INFOHEAD << "Panel Error InitOpenGL." << endl;

    // opengl window config. OGL2.0
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Fixed window size.
        glfwWindowHint(GLFW_SAMPLES, 4);          // 4x Samples MSAA.
    }
    
    // create window.
    GLFWmonitor* create_flag = {};
    GLFWwindow* window_object = {};

    window_object = glfwCreateWindow(window_szx, window_szy, window_title, create_flag, nullptr);
    
    // create context.
    glfwMakeContextCurrent(window_object);
    cout << TIEM_CURRENT << PANEL_INFOHEAD << "OpenGL init." << endl;

    // context => init imgui.
    imgui_system_init(window_object, "PSCDC-MAINSYSTEM/msyhbd.ttc");

    // enable vsync. 开启垂直同步.
    glfwSwapInterval(1);

    int32_t buffersize[2] = {};
    // main => oglwindow_eventloop
    while (true) {
        glfwPollEvents();

        glfwGetFramebufferSize(window_object, &buffersize[0], &buffersize[1]);
        glViewport(0, 0, buffersize[0], buffersize[1]);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render event function.
        render(); 
        if (EXIT_MAINEVENT_LOOP) {
            // main thread exit.
            cout << TIEM_CURRENT << PANEL_INFOHEAD << "Panel eventloop exit." << endl;
            break;
        }

        glfwMakeContextCurrent(window_object);
        glfwSwapBuffers(window_object);
    }

    // opengl free window.
    glfwDestroyWindow(window_object);
    glfwTerminate();
}