#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFERSIZE 1024

#define printE(x, ...) fprintf(stderr, x __VA_OPT__(, ) __VA_ARGS__)

#define arraySize(x) sizeof(x) / sizeof(x[0])
#define zeroMemory(x) memset(x, 0, BUFFERSIZE)

char *getSpriteHeader(char *filePath) {
  int filePathLength = strlen(filePath);
  filePath[filePathLength - 4] = '\0';
  filePathLength -= 4;
  int startingIdx = 0;
  for (int i = 0; i < filePathLength; i++)
    if (filePath[i] == '/')
      startingIdx = i + 1;

  filePathLength -= startingIdx;
  char spriteHeaderTemplate[] = "static uint8_t ";
  char spriteHeaderTemplateEnd[] = "[] = {\n";

  if (arraySize(spriteHeaderTemplate) + filePathLength +
          arraySize(spriteHeaderTemplateEnd) >
      1024) {
    printE("Buffer exceeded max buffer size\n");
    return NULL;
  }

  char *spriteHeader = malloc(sizeof(char) * (BUFFERSIZE));

  if (!spriteHeader) {
    printE("Unable to allocate memory for sprite header\n");
    return NULL;
  }
  printf("%s\n", &filePath[startingIdx]);

  zeroMemory(spriteHeader);
  strcpy(spriteHeader, spriteHeaderTemplate);
  strcat(spriteHeader, &filePath[startingIdx]);
  strcat(spriteHeader, spriteHeaderTemplateEnd);

  return spriteHeader;
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printE("Not enough arguments provided\n");
    return EXIT_FAILURE;
  }

  FILE *out = fopen(argv[1], "w");
  if (!out) {
    printE("Unable to create output file\n");
    return EXIT_FAILURE;
  }
  char *header =
      "#ifndef SPRITESXBM\n#define SPRITESXBM\n\n#include <stdint.h>\n#define "
      "SPRITEHEIGHT 32\n#define SPRITEWIDTH 32\n";

  fwrite(header, sizeof(header[0]), strlen(header), out);

  char buffer[BUFFERSIZE];

  for (int i = 2; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    if (!file) {
      printE("Unable to open file\n");

      fclose(out);
      return EXIT_FAILURE;
    }

    zeroMemory(buffer);

    char *spriteHeader = getSpriteHeader(argv[i]);
    if (!spriteHeader)
      return EXIT_FAILURE;

    fwrite(spriteHeader, sizeof(char), strlen(spriteHeader), out);

    fwrite("};\n", sizeof(char), 3, out);
    free(spriteHeader);
    fclose(file);
  }

  fwrite("#endif", sizeof(char), 6, out);
  fclose(out);
  return EXIT_SUCCESS;
}
