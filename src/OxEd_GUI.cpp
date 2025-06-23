#include "OxEd.h"

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

namespace OxEd_State
{
    Array<HexFile> OpenFiles;
}
using namespace OxEd_State;

struct OxEd_DrawParams
{
    static constexpr int DefaultRowWidth = 32;
    static constexpr int MinWidth = 1;
    static constexpr int MaxWidth = 64;

    bool bLineNumbers = false;
    bool bDataOffset = false;
    bool bDisplayASCII = false;
    bool bAutoRowWidth = false;
    int RowWidth = DefaultRowWidth;
} DrawParams;

bool CheckFileAlreadyOpened(const char* FileName)
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

void OxEd_Win32OpenFileDialog()
{
    char* FileNameBuffer = new char[MAX_PATH]{};

    OPENFILENAMEA DialogState = {};
    DialogState.lStructSize = sizeof(OPENFILENAMEA);
    DialogState.hwndOwner = hWindow;
    DialogState.hInstance = nullptr;
    DialogState.lpstrFilter = nullptr;
    DialogState.lpstrCustomFilter = nullptr;
    DialogState.nFilterIndex = 0;
    DialogState.lpstrFile = FileNameBuffer;
    DialogState.nMaxFile = MAX_PATH;
    DialogState.lpstrFileTitle = nullptr;
    DialogState.lpstrInitialDir = nullptr;
    DialogState.lpstrTitle = nullptr;
    DialogState.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    DialogState.lpstrDefExt = nullptr;
    DialogState.lCustData = 0;
    DialogState.lpfnHook = nullptr;
    BOOL bResult = GetOpenFileNameA(&DialogState);
    if (bResult)
    { // User hit 'OK'
        if (!CheckFileAlreadyOpened(FileNameBuffer))
        {
            FileContentsT NewFile = {};
            ReadFileContents(FileNameBuffer, NewFile);

            if (NewFile.Contents)
            {
                HexFile NewHexFile = {};
                NewHexFile.SetFile(NewFile);
                OpenFiles.Add(NewHexFile);
            }
        }
    }
    else
    { // User canceled dialog, or other error occured
        //DWORD ExError = CommDlgExtendedError();
        //DebugBreak();
    }
}

void OxEd_ImGui_DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                OxEd_Win32OpenFileDialog();
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
                ImGui::DragInt("Row Width", &DrawParams.RowWidth, 1, OxEd_DrawParams::MinWidth, OxEd_DrawParams::MaxWidth, "%d", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::EndMenu();

        }
        ImGui::EndMainMenuBar();
    }
}

void OxEd_ImGui_DrawFile(HexFile& File)
{
    int BytesPerRow = 0;

    ImVec2 AvailRegionSize = ImGui::GetContentRegionAvail();

    if (DrawParams.bAutoRowWidth)
    {
        // TODO: Actually calculate how many bytes can be displayed with available width + current text style (monospace font)
        BytesPerRow = OxEd_DrawParams::DefaultRowWidth;
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

void OxEd_ImGui_DrawOpenFiles()
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
                    OxEd_ImGui_DrawFile(CurrFile);
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}

void OxEd_ImGui_Draw()
{
    OxEd_ImGui_DrawMenuBar();
    OxEd_ImGui_DrawOpenFiles();
}
