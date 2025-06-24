#include "OxEd.h"
#include "OxEd_GUI.h"

// Globals
bool bRunning = false;
UINT WinResX = 1600U;
UINT WinResY = 900U;

struct OxEd
{
    static void ImGui_Init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.IniFilename = nullptr;
        io.FontGlobalScale = 1.0f;

        OxEd_PlatformT::ImGui_Init();
        OxEd_GfxT::ImGui_Init();
    }
    static void ImGui_Term()
    {
        OxEd_GfxT::ImGui_Term();
        OxEd_PlatformT::ImGui_Term();
        ImGui::DestroyContext();
    }
    static void ImGui_FrameBegin()
    {
        OxEd_GfxT::ImGui_NewFrame();
        OxEd_PlatformT::ImGui_NewFrame();
        ImGui::NewFrame();
    }
    static void ImGui_FrameEnd()
    {
        ImGui::Render();
        OxEd_GfxT::ImGui_RenderDrawData(ImGui::GetDrawData());
    }
    static void Draw()
    {
        ImGui_FrameBegin();

        OxEd_ImGui_Draw();
    #if _DEBUG
        static bool bImGuiShowDemoWindow = true;
        if (bImGuiShowDemoWindow)
        {
            ImGui::ShowDemoWindow();
        }
    #endif // _DEBUG

        ImGui_FrameEnd();
    }
    static void Tick()
    {
        OxEd_PlatformT::Tick();
        OxEd_GfxT::FrameBegin();
        Draw();
        OxEd_GfxT::FrameEnd();
    }
    static void Run()
    {
    #define OXED_RUN_ASSERT(Exp, Stage, Name) if (!(Exp)) { fprintf(stdout, "[error][fatal]: %s failed! (%s)", Stage, Name); return; }

        OXED_RUN_ASSERT(OxEd_PlatformT::Init(), "Platform initialization", OxEd_PlatformT::Name());
        OXED_RUN_ASSERT(OxEd_GfxT::Init(), "Graphics backend initialization", OxEd_GfxT::Name());
        ImGui_Init();

		bRunning = true;
        while (bRunning)
        {
            Tick();
        }

        ImGui_Term();
        OxEd_GfxT::Term();
        OxEd_PlatformT::Term();
    }
};

void OxEd_Run()
{
    OxEd::Run();
}
