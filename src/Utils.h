#ifndef UTILS_H
#define UTILS_H

struct FileContentsT
{
    static constexpr int MaxNameSize = 260;
    char* Name;
    size_t Size;
    u8* Contents;
};

void ReadFileContents(char* Name, FileContentsT& _FileContents);
void Release(FileContentsT& _FileContents);

template <typename T>
struct Array
{
    size_t Capacity;
    size_t Num;
    T* Data;

    static constexpr size_t InitCapacity = 32;
    static constexpr float GrowthFactor = 2.0f;
    Array()
    {
        Capacity = InitCapacity;
        Num = 0;
        Data = new T[Capacity];
    }
    ~Array()
    {
        if (Data)
        {
            delete[] Data;
        }
        Capacity = 0;
        Num = 0;
        Data = nullptr;
    }

    void Grow()
    {
        T* OldData = Data;

        Capacity = (size_t)(Capacity * GrowthFactor);
        Data = new T[Capacity];

        memcpy(Data, OldData, sizeof(T) * Num);
        delete[] OldData;
    }

    void Add(T& NewItem)
    {
        if (Num >= Capacity)
        {
            Grow();
        }
        Data[Num++] = NewItem;
    }
    void Remove(size_t Idx)
    {
        if (Idx < Num)
        {
            for (size_t ShiftIdx = Idx; ShiftIdx < Num - 1; ShiftIdx++)
            {
                Data[ShiftIdx] = Data[ShiftIdx + 1];
            }
            Num--;
        }
    }

    T* operator*()
    {
        return Data;
    }
    T& operator[](size_t Idx)
    {
        return Data[Idx];
    }
};

char GetHex(u8 Value);
char GetHighHex(u8 Value);
char GetLowHex(u8 Value);

#define RGB_TO_FLOAT4(R, G, B) { float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f, 1.0f }

#endif // UTILS_H

