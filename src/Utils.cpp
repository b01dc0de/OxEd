#include "OxEd.h" // OxEd.h includes Utils.h

void ReadFileContents(const char* Name, FileContentsT& _FileContents)
{
    // TODO(CKA): Support 64-bit file sizes
    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, Name, "rb");

    if (FileHandle)
    {
        _FileContents.Name = Name;

        fseek(FileHandle, 0, SEEK_END);
        long FileSize = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (FileSize)
        {
            _FileContents.Size = FileSize;
            _FileContents.Contents = new u8[FileSize];
            fread_s(_FileContents.Contents, _FileContents.Size, _FileContents.Size, 1, FileHandle);
        }
        else
        {
            _FileContents.Size = 0;
            _FileContents.Contents = nullptr;
        }

        fclose(FileHandle);
    }
}

void Release(FileContentsT& _FileContents)
{
    if (_FileContents.Name) { delete[] _FileContents.Name; }
    if (_FileContents.Contents) { delete[] _FileContents.Contents; }
    _FileContents = {};
}

char GetHex(u8 Value)
{
    if (0 <= Value && Value <= 0xF)
    {
        if (Value <= 9)
        {
            return Value + 0x30;
        }
        else
        {
            return (Value - 10) + 0x41;
        }
    }
}

char GetHighHex(u8 Value)
{
    return GetHex((Value & 0xF0) >> 4);
}

char GetLowHex(u8 Value)
{
    return GetHex(Value & 0x0F);
}


