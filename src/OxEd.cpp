#include "OxEd.h"

// Globals
bool bRunning = false;
UINT WinResX = 1600U;
UINT WinResY = 900U;

struct HexFile 
{
    const char* FileName = nullptr;

    size_t FileSize = 0;
    u8* FileContents = nullptr;

    size_t HexTextSize = 0;
    char* HexText = nullptr;

    void Release();
    void SetFile(FileContentsT& NewFile);
};

void HexFile::Release()
{
    if (FileName) { delete[] FileName; }
    if (FileContents) { delete[] FileContents; }
    if (HexText) { delete[] HexText; }

    *this = {};
}

void HexFile::SetFile(FileContentsT& NewFile)
{
    Release();

    FileName = NewFile.Name;
    FileSize = NewFile.Size;
    FileContents = NewFile.Contents;

    // Create hex text representation of file
    {
        HexTextSize = (FileSize * 3) + 1;
        HexText = new char[HexTextSize];
        size_t WriteIdx = 0;
        for (size_t ByteIdx = 0; ByteIdx < FileSize; ByteIdx++)
        {
            HexText[WriteIdx++] = GetHighHex(FileContents[ByteIdx]);
            HexText[WriteIdx++] = GetLowHex(FileContents[ByteIdx]);
            HexText[WriteIdx++] = ' ';
        }
        HexText[WriteIdx++] = '\0';
        ASSERT(WriteIdx == HexTextSize);
    }
}

struct OxEd_GUI_DrawParams
{
    static constexpr int DefaultRowWidth = 32;
    static constexpr int MinWidth = 1;
    static constexpr int MaxWidth = 64;

    bool bLineNumbers = false;
    bool bDataOffset = false;
    bool bDisplayASCII = false;
    bool bAutoRowWidth = false;
    int RowWidth = DefaultRowWidth;
};

struct OxEd_GUI
{
    static Array<HexFile> OpenFiles;
    static OxEd_GUI_DrawParams DrawParams;

    static bool CheckFileAlreadyOpened(const char* FileName);
    static void OpenFileDialog();

    static void ImGui_DrawMenuBar();
    static void ImGui_DrawFile(HexFile& File);
    static void ImGui_DrawOpenFiles();
    static void Draw();
};

Array<HexFile> OxEd_GUI::OpenFiles;
OxEd_GUI_DrawParams OxEd_GUI::DrawParams;

bool OxEd_GUI::CheckFileAlreadyOpened(const char* FileName)
{
    bool bResult = false;
    for (int FileIdx = 0; FileIdx < OpenFiles.Num; FileIdx++)
    {
        if (strcmp(FileName, OpenFiles[FileIdx].FileName) == 0)
        {
            bResult = true;
            break;
        }
    }
    return bResult;
}

void OxEd_GUI::OpenFileDialog()
{
    FileContentsT NewFile = {};
    OxEd_PlatformT::OpenFile(NewFile);
    if (!CheckFileAlreadyOpened(NewFile.Name) && NewFile.Contents)
    {
        HexFile NewHexFile = {};
        NewHexFile.SetFile(NewFile);
        OpenFiles.Add(NewHexFile);
    }
}

void OxEd_GUI::ImGui_DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                OpenFileDialog();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::Checkbox("Line Numbers", &DrawParams.bLineNumbers);
            ImGui::Checkbox("Data Offset", &DrawParams.bDataOffset);
            ImGui::Checkbox("Display Decoded ASCII", &DrawParams.bDisplayASCII);
            ImGui::Checkbox("Auto Row Width", &DrawParams.bAutoRowWidth);

            if (!DrawParams.bAutoRowWidth)
            {
                ImGui::DragInt("Row Width", &DrawParams.RowWidth, 1, OxEd_GUI_DrawParams::MinWidth, OxEd_GUI_DrawParams::MaxWidth, "%d", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::EndMenu();

        }
        ImGui::EndMainMenuBar();
    }
}

void OxEd_GUI::ImGui_DrawFile(HexFile& File)
{
    int BytesPerRow = 0;

    ImVec2 AvailRegionSize = ImGui::GetContentRegionAvail();

    if (DrawParams.bAutoRowWidth)
    {
        // TODO: Actually calculate how many bytes can be displayed with available width + current text style (monospace font)
        BytesPerRow = OxEd_GUI_DrawParams::DefaultRowWidth;
    }
    else
    {
        BytesPerRow = DrawParams.RowWidth;
    }

    ASSERT(BytesPerRow != 0);
    int NumLines = File.FileSize / BytesPerRow + (File.FileSize % BytesPerRow == 0 ? 0 : 1);
    int LineWidth = BytesPerRow * 3;
    auto Hack_GetWidthDecimal = [](int Value) -> int
    {
        int Width = 0;
        while (Value > 0)
        {
            Value /= 10;
            Width++;
        }
        return Width;
    };
    int MaxLineNoWidth = Hack_GetWidthDecimal(NumLines + 1);

    ImVec4 LineNumbersColor = RGB_TO_FLOAT4(166, 227, 161);
    ImVec4 DataOffsetsColor = RGB_TO_FLOAT4(220, 199, 123);
    ImVec4 HexDataColor = RGB_TO_FLOAT4(198, 166, 247);

    {
        ImGui::BeginChild("ActiveFile_Contents");
        ImGui::PushStyleColor(ImGuiCol_Text, HexDataColor);
        for (int LineIdx = 0; LineIdx < NumLines; LineIdx++)
        {
            size_t BeginIdx = LineWidth * LineIdx;
            size_t EndIdx = Clamp((size_t)(BeginIdx + LineWidth - 1), (size_t)0, File.HexTextSize - 1);
            if (DrawParams.bLineNumbers)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, LineNumbersColor);
                char TextOutBuffer[sizeof("OxAABBCCDD ")] = {};
                int WriteIdx = sprintf_s(TextOutBuffer, "%*d", MaxLineNoWidth, LineIdx + 1);
                ImGui::TextUnformatted(TextOutBuffer, TextOutBuffer + WriteIdx);
                ImGui::SameLine();
                ImGui::PopStyleColor();
            }
            if (DrawParams.bDataOffset)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, DataOffsetsColor);
                char TextOutBuffer[sizeof("OxAABBCCDD ")] = {};
                int WriteIdx = sprintf_s(TextOutBuffer, "0x%08X", LineIdx * BytesPerRow);
                ImGui::TextUnformatted(TextOutBuffer, TextOutBuffer + WriteIdx);
                ImGui::SameLine();
                ImGui::PopStyleColor();
            }

            const char* LineBegin = File.HexText + BeginIdx;
            const char* LineEnd = File.HexText + EndIdx;
            ImGui::TextUnformatted(LineBegin, LineEnd);
        }
        ImGui::PopStyleColor();

        ImGui::EndChild();
    }
}

void OxEd_GUI::ImGui_DrawOpenFiles()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyDefault_ | ImGuiTabBarFlags_Reorderable;
    tab_bar_flags |= ImGuiTabBarFlags_DrawSelectedOverline;

    bool bFileOpen = OpenFiles.Num > 0;
    if (bFileOpen && ImGui::Begin("OxEd_ImGui_DrawFile", nullptr, flags))
    {
        if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
        {
            for (int FileIdx = 0; FileIdx < OpenFiles.Num; FileIdx++)
            {
                HexFile& CurrFile = OpenFiles[FileIdx];

                ImGuiTabItemFlags tab_flags = 0; // ImGuiTabItemFlags_UnsavedDocument
                bool visible = ImGui::BeginTabItem(CurrFile.FileName, nullptr, tab_flags);

                if (visible)
                {
                    ImGui_DrawFile(CurrFile);
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}

void OxEd_GUI::Draw()
{
    ImGui_DrawMenuBar();
    ImGui_DrawOpenFiles();

    #if _DEBUG
        static bool bImGuiShowDemoWindow = true;
        if (bImGuiShowDemoWindow)
        {
            ImGui::ShowDemoWindow();
        }
    #endif // _DEBUG
}

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
        OxEd_GUI::Draw();
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
