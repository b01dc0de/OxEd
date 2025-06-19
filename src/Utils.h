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

#endif // UTILS_H

