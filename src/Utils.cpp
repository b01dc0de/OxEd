#include "OxEd.h" // OxEd.h includes Utils.h

#pragma pack(push, 1)
struct BMPFileHeader
{
    u16 Type; // Always Ascii BM
    u32 SizeInBytes; // Size (bytes) of file
    u16 Res1; // 0
    u16 Res2; // 0
    u32 OffsetBytes; // Offset (bytes) to actual pixel data
};

struct BMPInfoHeader
{
    u32 StructSize; // Size (bytes) of InfoHeader
    s32 Width;
    s32 Height; // NOTE(chris): If positive, pixel data is bottom to top
    u16 Planes; // Must be 1
    u16 BitsPerPixel; // Bits-per-pixel (0, 1, 4, 8, 16, 24, 32)
    u32 Compression; // *Should* be 0
    u32 Unused_ImgSize; // Only used if Compression is weird (not 0)
    s32 HRes; // Horizontal resolution
    s32 VRes; // Vertical resolution
    u32 ColorsUsed; // 0 for our purposes
    u32 ColorsImportant; // 0 for our purposes
};
#pragma pack(pop)

struct BMP
{
    BMPFileHeader FileHeader;
    BMPInfoHeader InfoHeader;
};

constexpr u16 SupportedBPP = 32;
constexpr u16 BitmapFileTypeValue = 0x4D42;

void WriteBMP(const char* OutFilename, const Image32& InImage)
{
    RGBA32* SwizzledImage = new RGBA32[InImage.PxCount];
    for (unsigned int PxIdx = 0; PxIdx < InImage.PxCount; PxIdx++)
    {
        SwizzledImage[PxIdx] = RGBA32::Swizzle(InImage.PixelBuffer[PxIdx]);
    }

    u32 PxBytes = sizeof(RGBA32) * InImage.PxCount;

    BMP BMP_Data = {};

    BMP_Data.FileHeader.Type = BitmapFileTypeValue;
    BMP_Data.FileHeader.SizeInBytes = sizeof(BMP_Data) + PxBytes;
    BMP_Data.FileHeader.Res1 = 0;
    BMP_Data.FileHeader.Res2 = 0;
    BMP_Data.FileHeader.OffsetBytes = sizeof(BMP_Data);

    BMP_Data.InfoHeader.StructSize = sizeof(BMPInfoHeader);
    BMP_Data.InfoHeader.Width = (s32)InImage.Width;
    BMP_Data.InfoHeader.Height = -(s32)InImage.Height;
    BMP_Data.InfoHeader.Planes = 1;
    BMP_Data.InfoHeader.BitsPerPixel = SupportedBPP;
    BMP_Data.InfoHeader.Compression = 0;
    BMP_Data.InfoHeader.Unused_ImgSize = PxBytes;
    BMP_Data.InfoHeader.HRes = 0;
    BMP_Data.InfoHeader.VRes = 0;
    BMP_Data.InfoHeader.ColorsUsed = 0;
    BMP_Data.InfoHeader.ColorsImportant = 0;

    FILE* BMP_File = nullptr;
    fopen_s(&BMP_File, OutFilename, "wb");
    if (BMP_File != nullptr)
    {
        fwrite(&BMP_Data.FileHeader, sizeof(BMP_Data.FileHeader), 1, BMP_File);
        fwrite(&BMP_Data.InfoHeader, sizeof(BMP_Data.InfoHeader), 1, BMP_File);
        fwrite(SwizzledImage, InImage.PxBytes, 1, BMP_File);
        fclose(BMP_File);
    }
    else
    {
        printf("ERROR: Cannot open file \"%s\" for write\n", OutFilename);
    }

    delete[] SwizzledImage;
}

void ReadBMP(const char* InFilename, Image32& OutImage)
{
    FILE* BMP_File = nullptr;
    fopen_s(&BMP_File, InFilename, "rb");

    if (nullptr != BMP_File)
    {
        // Get file size in bytes
        fpos_t FileSizeBytes = 0;
        {
            int Result = fseek(BMP_File, 0, SEEK_END);
            // CKA_TODO: Assert Result == 0
            fgetpos(BMP_File, &FileSizeBytes);

            // Set curr pos to beginning of file
            Result = fseek(BMP_File, 0, SEEK_SET);
        }

        // Parse .BMP file into BMP struct
        BMP ReadBMP = {};
        fread_s(&ReadBMP, sizeof(BMP), sizeof(BMP), 1, BMP_File);

        if (SupportedBPP != ReadBMP.InfoHeader.BitsPerPixel) { DebugBreak(); }

        size_t BytesRemaining = FileSizeBytes - sizeof(BMP);
        if (BitmapFileTypeValue == ReadBMP.FileHeader.Type)
        {
            u8* NewPxBuffer = new u8[BytesRemaining];
            // Extract pixel data into OutImage struct
            OutImage.Width = ReadBMP.InfoHeader.Width;
            OutImage.Height = ReadBMP.InfoHeader.Height > 0 ? ReadBMP.InfoHeader.Height : -ReadBMP.InfoHeader.Height;
            OutImage.PxCount = OutImage.Width * OutImage.Height;
            OutImage.PxBytes = BytesRemaining;
            OutImage.PixelBuffer = (RGBA32*)(NewPxBuffer);

            fread_s(OutImage.PixelBuffer, OutImage.PxBytes, OutImage.PxBytes, 1, BMP_File);

            // Unswizzle(?) the pixel data
            for (unsigned int PxIdx = 0; PxIdx < OutImage.PxCount; PxIdx++)
            {
                RGBA32& CurrPx = OutImage.PixelBuffer[PxIdx];
                CurrPx = RGBA32::Swizzle(CurrPx);
            }
        }

        fclose(BMP_File);
    }
}

void GetDebugBMP(Image32& OutImage)
{
    u32 DebugImgLength = 16;
    OutImage.Width = DebugImgLength;
    OutImage.Height = DebugImgLength;
    OutImage.PxCount = OutImage.Width * OutImage.Height;
    OutImage.PxBytes = sizeof(RGBA32) * OutImage.PxCount;
    OutImage.PixelBuffer = new RGBA32[OutImage.PxCount];

    constexpr RGBA32 Pink{ 255u, 73u, 173u, 255u };
    constexpr RGBA32 Black{ 0u, 0u, 0u, 255u };
    constexpr RGBA32 Red{ 255u, 0u, 0u, 255u };
    constexpr RGBA32 Green{ 0u, 255u, 0u, 255u };
    constexpr RGBA32 Blue{ 0u, 0u, 255u, 255u };
    constexpr RGBA32 White{ 255u, 255u, 255u, 255u };
    for (int PxIdx = 0; PxIdx < OutImage.PxCount; PxIdx++)
    {
        int PxRow = PxIdx / OutImage.Width;
        int PxCol = PxIdx % OutImage.Width;
        if (PxRow == 0 && PxCol == 0)
        {
            OutImage.PixelBuffer[PxIdx] = Red;
        }
        else if (PxRow == 0 && PxCol == OutImage.Width - 1)
        {
            OutImage.PixelBuffer[PxIdx] = Green;
        }
        else if (PxRow == OutImage.Height - 1 && PxCol == 0)
        {
            OutImage.PixelBuffer[PxIdx] = Blue;
        }
        else if (PxRow == OutImage.Height - 1 && PxCol == OutImage.Width - 1)
        {
            OutImage.PixelBuffer[PxIdx] = White;
        }
        else
        {
            bool bEvenCell = (PxRow + PxCol) % 2 == 0;
            OutImage.PixelBuffer[PxIdx] = bEvenCell ? Black : Pink;
        }
    }
}
