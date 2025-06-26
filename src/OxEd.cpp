#include "OxEd.h"

// Globals
bool bRunning = false;
UINT WinResX = 1600U;
UINT WinResY = 900U;

struct HexFile 
{
    const char* FullPathName = nullptr;

    size_t FileSize = 0;
    u8* FileContents = nullptr;

    size_t HexTextSize = 0;
    char* HexText = nullptr;

    bool bOpen = false;

    void Release()
    {
        if (FullPathName) { delete[] FullPathName; }
        if (FileContents) { delete[] FileContents; }
        if (HexText) { delete[] HexText; }

        *this = {};
    }
    void SetFile(FileContentsT& NewFile)
    {
        Release();

        FullPathName = NewFile.Name;
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

        bOpen = true;
    }
};


struct OxEd_GUI
{
    struct DrawParamsT
    {
        static constexpr int DefaultRowWidth = 32;
        static constexpr int MinWidth = 1;
        static constexpr int MaxWidth = 64;

        bool bByteOffsets = false;
        bool bDisplayASCII = false;
        bool bAutoRowWidth = false;
        int RowWidth = DefaultRowWidth;
    #if OXED_CONFIG_DEBUG()
        bool bDebugShowDemo = true;
        float DebugGlobalScale = 1.0f;
    #endif // OXED_CONFIG_DEBUG()
    };
    static DrawParamsT DrawParams;
    static Array<HexFile> OpenFiles;

    static bool IsFileAlreadyOpen(const char* FileName);
    static void OpenFileDialog();

    static void ImGui_DrawMenuBar();
    static void ImGui_DrawFile(HexFile& File);
    static void ImGui_DrawOpenFiles();
    static void Draw();
    static void Init();
    static void Term();

    static void Debug_AddDefaultFiles();
};

Array<HexFile> OxEd_GUI::OpenFiles;
OxEd_GUI::DrawParamsT OxEd_GUI::DrawParams;

bool OxEd_GUI::IsFileAlreadyOpen(const char* FileName)
{
    bool bResult = false;
    for (int FileIdx = 0; FileIdx < OpenFiles.Num; FileIdx++)
    {
        if (strcmp(FileName, OpenFiles[FileIdx].FullPathName) == 0)
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
    if (!IsFileAlreadyOpen(NewFile.Name) && NewFile.Contents)
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
            ImGui::Checkbox("Byte Offsets", &DrawParams.bByteOffsets);
            ImGui::Checkbox("Display Decoded ASCII", &DrawParams.bDisplayASCII);
            ImGui::Checkbox("Auto Row Width", &DrawParams.bAutoRowWidth);

            if (!DrawParams.bAutoRowWidth)
            {
                ImGui::DragInt("Row Width", &DrawParams.RowWidth, 1, DrawParamsT::MinWidth, DrawParamsT::MaxWidth, "%d", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::EndMenu();
        }
    #if OXED_CONFIG_DEBUG()
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::Checkbox("Show Demo", &DrawParams.bDebugShowDemo);

            constexpr float MinGlobalScale = 0.25f;
            constexpr float MaxGlobalScale = 4.0f;
            ImGuiIO& io = ImGui::GetIO();
            ImGui::DragFloat("Global Scale", &io.FontGlobalScale, MinGlobalScale * 0.5f, MinGlobalScale, MaxGlobalScale, "%.2f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::EndMenu();
        }
    #endif // OXED_CONFIG_DEBUG()
        ImGui::EndMainMenuBar();
    }
}

bool CharIsPrintable(char Value)
{
    return Value >= 0x20;
}

void OxEd_GUI::ImGui_DrawFile(HexFile& File)
{
    //ImGui::GetFrameHeightWithSpacing()

    int BytesPerRow = 0;

    // TODO: Assert using a monospace font
    //ImGuiStyle& Style = ImGui::GetStyle();
    ImVec2 TotalRegionSize = ImGui::GetContentRegionAvail();
    ImVec2 LeftPaneSize = TotalRegionSize;
    //ImVec2 RightPaneSize{ 0, 0 };

    ImVec2 GlyphSize = ImGui::CalcTextSize(" ");

    /*
    float NumGlyphsX = TotalRegionSize.x / GlyphSize.x;
    float NumGlyphsY = TotalRegionSize.y / GlyphSize.y;

    if (DrawParams.bDisplayASCII)
    {
        LeftPaneSize.x *= 0.5f;
        //RightPaneSize = LeftPaneSize;
        RightPaneSize = { 0, 0 };

        //LeftPaneSize = { 0, 0 };
        //RightPaneSize = { 0, 0 };
    }
    */

    if (DrawParams.bAutoRowWidth)
    {
        BytesPerRow = LeftPaneSize.x / GlyphSize.x;
    }
    else
    {
        BytesPerRow = DrawParams.RowWidth;
    }

    ASSERT(BytesPerRow != 0);
    int NumLines = File.FileSize / BytesPerRow + (File.FileSize % BytesPerRow == 0 ? 0 : 1);
    int LineWidth = BytesPerRow * 3;

    ImVec4 LineNumbersColor = RGB_TO_FLOAT4(166, 227, 161);
    ImVec4 DataOffsetsColor = RGB_TO_FLOAT4(220, 199, 123);
    ImVec4 HexDataColor = RGB_TO_FLOAT4(198, 166, 247);
    ImVec4 ASCIIColor = RGB_TO_FLOAT4(115, 211, 254);

    {
        ImGui::BeginChild("ImGui_DrawFile__Left", LeftPaneSize, ImGuiChildFlags_ResizeX);
        ImGui::PushStyleColor(ImGuiCol_Text, HexDataColor);
        for (int LineIdx = 0; LineIdx < NumLines; LineIdx++)
        {
            size_t BeginIdx = LineWidth * LineIdx;
            size_t EndIdx = Clamp((size_t)(BeginIdx + LineWidth - 1), (size_t)0, File.HexTextSize - 1);
            if (DrawParams.bByteOffsets)
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
    /*
    if (DrawParams.bDisplayASCII)
    {
        ImGui::SameLine();
        ImGui::BeginChild("ImGui_DrawFile__Right", RightPaneSize, ImGuiChildFlags_None);
        ImGui::PushStyleColor(ImGuiCol_Text, ASCIIColor);
        for (int LineIdx = 0; LineIdx < NumLines; LineIdx++)
        {
            size_t BeginIdx = BytesPerRow * LineIdx;
            char AsciiBuffer[DrawParamsT::MaxWidth + 1] = {};
            int CharIdx = 0;
            while ((CharIdx < BytesPerRow) && (BeginIdx + CharIdx < File.FileSize))
            {
                if (CharIsPrintable(File.FileContents[BeginIdx + CharIdx]))
                {
                    AsciiBuffer[CharIdx] = File.FileContents[BeginIdx + CharIdx];
                }
                else
                {
                    AsciiBuffer[CharIdx] = ' ';
                }
                CharIdx++;
            }
            AsciiBuffer[CharIdx] = '\0';
            ImGui::TextUnformatted(AsciiBuffer, AsciiBuffer + CharIdx);
        }
        ImGui::PopStyleColor();
        ImGui::EndChild();
    }
    */
}

void OxEd_GUI::ImGui_DrawOpenFiles()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    //float ImGui::GetTextLineHeight()
    //ImGui::GetFontSize();

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

                bool bSelected = ImGui::BeginTabItem(CurrFile.FullPathName, &CurrFile.bOpen, ImGuiTabItemFlags_None);
                if (bSelected)
                {
                    ImGui_DrawFile(CurrFile);
                    ImGui::EndTabItem();
                }
                if (!CurrFile.bOpen)
                {
                    ImGui::SetTabItemClosed(CurrFile.FullPathName);
                    CurrFile.Release();
                    OpenFiles.Remove(FileIdx);
                    FileIdx--;
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

    #if OXED_CONFIG_DEBUG()
        static bool bImGuiShowDemoWindow = true;
        if (OpenFiles.Num == 0 && bImGuiShowDemoWindow)
        {
            ImGui::ShowDemoWindow();
        }
    #endif // OXED_CONFIG_DEBUG()
}

void OxEd_GUI::Init()
{
#define ENABLE_DEFAULT_DEBUG_FILE() (1)
#if OXED_CONFIG_DEBUG() && ENABLE_DEFAULT_DEBUG_FILE()
    Debug_AddDefaultFiles();
#endif // OXED_CONFIG_DEBUG()
}

void OxEd_GUI::Term()
{
    for (int Idx = 0; Idx < OpenFiles.Num; Idx++)
    {
        OpenFiles[Idx].Release();
    }
}

void OxEd_GUI::Debug_AddDefaultFiles()
{
    FileContentsT DebugFile = {};

    static bool bUseLongFile = true;
    if (bUseLongFile)
    {
        constexpr int LongFileSize = 256 * 256;
        DebugFile.Name = new char[FileContentsT::MaxNameSize];
        sprintf(DebugFile.Name, "[debug_long]");
        DebugFile.Size = LongFileSize;
    }
    else
    {
        constexpr int ShortFileSize = 256;
        DebugFile.Name = new char[FileContentsT::MaxNameSize];
        sprintf(DebugFile.Name, "[debug_short]");
        DebugFile.Size = ShortFileSize;

    }
    DebugFile.Contents = new u8[DebugFile.Size];
    for (int Idx = 0; Idx < DebugFile.Size; Idx++)
    {
        DebugFile.Contents[Idx] = (u8)(Idx % 256);
    }

    HexFile DebugFileHex = {};
    DebugFileHex.SetFile(DebugFile);
    OpenFiles.Add(DebugFileHex);
}

struct OxEd
{
    static void ImGui_Init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Setup IO
        ImGuiIO& IO = ImGui::GetIO();
        IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        IO.IniFilename = nullptr;
        IO.FontGlobalScale = 1.0f;

        OxEd_PlatformT::ImGui_Init();
        OxEd_GfxT::ImGui_Init();

        // Setup Style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(OxEd_PlatformT::Scale());
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
        OxEd_GUI::Init();

		bRunning = true;
        while (bRunning)
        {
            Tick();
        }

        OxEd_GUI::Term();
        ImGui_Term();
        OxEd_GfxT::Term();
        OxEd_PlatformT::Term();
    }
};

void OxEdMain()
{
    OxEd::Run();
}
