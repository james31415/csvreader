#include <stdint.h>

#define Kilobytes(U) ((U) * 1024LL)
#define Megabytes(U) (Kilobytes(U) * 1024LL)
#define Gigabytes(U) (Megabytes(U) * 1024LL)

typedef int32_t s32;
typedef uint32_t u32;
typedef uint8_t u8;
typedef uint8_t b8;

#ifdef _DEBUG
#define Assert(Expression) if (!(Expression)) *(int*)0 = 0;
#else
#define Assert(Expression)
#endif

#define FIELD_CALLBACK(name) void name(char* Field, void* UserData)
typedef FIELD_CALLBACK(field_callback);

#define ROW_CALLBACK(name) b8 name(void* UserData)
typedef ROW_CALLBACK(row_callback);

typedef struct reader
{
    s32 FileID;
    u32 BufferPosition;
    u32 BufferSize;

    field_callback* FieldCallback;
    row_callback* RowCallback;

    void* UserData;

    u8* Buffer;
} reader;

u8 PeekCharacterFromFile(reader* Reader)
{
    u8 Result = 0;
    Assert(Reader->BufferPosition < Reader->BufferSize);

    Result = Reader->Buffer[Reader->BufferPosition];

    return Result;
}

u8 GetCharacterFromFile(reader* Reader)
{
    u8 Result = PeekCharacterFromFile(Reader);
    ++Reader->BufferPosition;

    return Result;
}

void Unread(reader* Reader)
{
    --Reader->BufferPosition;
}

b8 IsFieldTerminator(u8 Character)
{
    return (Character == ',') || (Character == '\n');
}

b8 IsSpace(u8 Character)
{
    return Character == ' ' || Character == '\t';
}

void ConsumeSpaces(reader* Reader)
{
    u8 Character;
    do
    {
        Character = GetCharacterFromFile(Reader);
    } while (IsSpace(Character));
    Unread(Reader);
}

b8 IsBadSimpleFieldCharacter(u8 Character)
{
    return IsSpace(Character) || IsFieldTerminator(Character) || (Character == '"');
}

void ParseSimpleField(reader* Reader)
{
    char Buffer[256];
    u8 Index = 0;

    u8 Character = GetCharacterFromFile(Reader);

    if (IsBadSimpleFieldCharacter(Character))
    {
        Unread(Reader);
        return;
    }

    Buffer[Index] = Character;
    ++Index;

    Character = GetCharacterFromFile(Reader);
    while (!IsBadSimpleFieldCharacter(Character) && Index < 255)
    {
        Buffer[Index] = Character;
        ++Index;

        Character = GetCharacterFromFile(Reader);
    }
    Unread(Reader);
    Buffer[Index] = '\0';

    if (Reader->FieldCallback)
        Reader->FieldCallback(Buffer, Reader->UserData);
}

b8 ProcessDoubleQuote(reader* Reader, u8 Character)
{
    if ((Character == '"') && (PeekCharacterFromFile(Reader) == '"'))
    {
        GetCharacterFromFile(Reader);
        return 1;
    }

    return 0;
}

u8 ParseSubField(reader* Reader, char* Buffer, u32* Index, u32 BufferLen)
{
    u8 Character = GetCharacterFromFile(Reader);
    while ((Character != '"') && *Index < BufferLen)
    {
        Buffer[*Index] = Character;
        ++(*Index);

        Character = GetCharacterFromFile(Reader);
    }

    return Character;
}

void ParseQuotedField(reader* Reader)
{
    GetCharacterFromFile(Reader);

    char Buffer[256];

    u32 Index = 0;

    u8 Character = ParseSubField(Reader, Buffer, &Index, 255);
    while (ProcessDoubleQuote(Reader, Character) && Index < 255)
    {
        Buffer[Index] = '"';
        ++Index;

        Character = ParseSubField(Reader, Buffer, &Index, 255);
    }
    Assert(Character == '"');

    Buffer[Index] = '\0';

    if (Reader->FieldCallback)
        Reader->FieldCallback(Buffer, Reader->UserData);

}

void ParseRawField(reader* Reader)
{
    u8 Character = PeekCharacterFromFile(Reader);
    if (Character == '"')
    {
        ParseQuotedField(Reader);
    }
    else
    {
        ParseSimpleField(Reader);
    }
}

void ParseRawString(reader* Reader)
{
    ConsumeSpaces(Reader);
    ParseRawField(Reader);

    u8 Character = PeekCharacterFromFile(Reader);
    if (!IsFieldTerminator(Character))
    {
        ConsumeSpaces(Reader);
    }
}

void ParseCsvFile(reader* Reader)
{
    while (PeekCharacterFromFile(Reader))
    {
        u8 Character;
        do
        {
            ParseRawString(Reader);
            Character = GetCharacterFromFile(Reader);
        } while (Character == ',');

        if (Reader->RowCallback)
        {
            if (!Reader->RowCallback(Reader->UserData))
                return;
        }

        Assert((Character == '\n'));
    }
}

