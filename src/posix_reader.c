#include <stdlib.h>
#include "csvreader.h"

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct Position
{
    u32 Row;
    u32 Column;
} Position;

ROW_CALLBACK(UpdateRow)
{
    Position* pos = (Position*)UserData;
    ++pos->Row;
    pos->Column = 0;

    return pos->Row < 2;
}

b8 IsDigit(u8 Character)
{
    return (0 <= (Character - '0')) && ((Character - '0') <= 9);
}

void PrintFieldType(char* Field)
{
    char* CharPosition = Field;
    while (*CharPosition != '\0')
    {
        char Character = *CharPosition;
        if (!IsDigit(Character) && !(Character == '.') && !(Character == '-'))
        {
            printf(" STRING\n");
            return;
        }
        ++CharPosition;
    }
    if (CharPosition == Field)
    {
        printf(" EMPTY\n");
    }
    else
    {
        printf(" NUMERIC\n");
    }
}

FIELD_CALLBACK(PrintField)
{
    Position* pos = (Position*)UserData;
    printf("Field (%u, %u): %s", pos->Row, pos->Column, Field);
    PrintFieldType(Field);
    ++pos->Column;
}

u64
GetFileSize(const char* FileName) {
  u64 Result = 0;

  struct stat buffer;
  int status = stat(FileName, &buffer);
  if (status == 0) {
    Result = (u64)buffer.st_size;
  }

  return Result;
}

u8*
ReadEntireFile(const char* FileName) {
  FILE* File = fopen(FileName, "r");
  if (File == NULL) {
    printf("Could not open %s\n", FileName);
    exit(1);
  }

  printf("Successfully opened %s\n", FileName);

  u64 FileSize = GetFileSize(FileName);

  u8* Result = (u8*)malloc(FileSize + 1);
  if (!Result) {
    printf("malloc failed! %s (%d)", __FILE__, __LINE__);
    exit(1);
  }
  fread(Result, FileSize, 1, File);
  Result[FileSize] = 0;

  printf("Result: %s\n", Result);

  fclose(File);

  return Result;
}

#ifdef MAIN
int main(int argc, char* argv[])
{
  Position position = {0};
  reader Reader = {0};

  Reader.FieldCallback = PrintField;
  Reader.RowCallback = UpdateRow;
  Reader.UserData = &position;

  if (argc < 2)
  {
    printf("Need filename");
    return -1;
  }

  const char* FileName = argv[1];
  printf("File: %s\n", FileName);

  Reader.Buffer = ReadEntireFile(FileName);
  Reader.BufferSize = (u32)strlen((const char*)Reader.Buffer);

  ParseCsvFile(&Reader);

  free(Reader.Buffer);

  return 0;
}
#endif
