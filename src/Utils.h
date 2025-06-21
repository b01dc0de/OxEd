#ifndef UTILS_H
#define UTILS_H

struct FileContentsT
{
    const char* Name;
    size_t Size;
    u8* Contents;
};

void ReadFileContents(const char* Name, FileContentsT& _FileContents);
void Release(FileContentsT& _FileContents);

char GetHex(u8 Value);
char GetHighHex(u8 Value);
char GetLowHex(u8 Value);

#define RGB_TO_FLOAT4(R, G, B) { float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f, 1.0f }

#endif // UTILS_H

