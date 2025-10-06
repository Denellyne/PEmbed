#include <assert.h>
#include <png.h>
#include <pngconf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFERSIZE 1024

#define printE(x, ...) fprintf(stderr, x __VA_OPT__(, ) __VA_ARGS__)

#define arraySize(x) sizeof(x) / sizeof(x[0])
#define zeroMemory(x) memset(x, 0, BUFFERSIZE)

int writeByteArray(png_bytep *, int, int, int, FILE *, char *);
uint8_t getGray(int x, int y, png_bytep *rows, int colorType) {

  uint8_t *ptr;
  uint8_t g;
  if (colorType & PNG_COLOR_MASK_ALPHA) {
    ptr = rows[y] + x * 4;
    g = ((int)ptr[0] + (int)ptr[1] + (int)ptr[2]) / 3;
    return g;
  }
  ptr = rows[y] + x * 3;
  return ((int)ptr[0] + (int)ptr[1] + (int)ptr[2]) / 3;
}
uint8_t getPixelData(int x, int y, png_bytep *rows, int colorType, int width,
                     int height) {
  if (x < 0 || x >= width || y < 0 || y >= height)
    return 0;
  if (getGray(x, y, rows, colorType) > 128 / 2)
    return 1;
  return 0;
}
uint8_t getPixel(int x, int y, png_bytep *rows, int colorType, int width,
                 int height) {
  uint8_t pixel = 0;
  for (int i = 0; i < 8; i++)
    pixel |= getPixelData(x + i, y, rows, colorType, width, height) << i;
  // printf("0x%x ", pixel);
  return pixel;
}

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

  zeroMemory(spriteHeader);
  strcpy(spriteHeader, spriteHeaderTemplate);
  strcat(spriteHeader, &filePath[startingIdx]);
  strcat(spriteHeader, spriteHeaderTemplateEnd);

  return spriteHeader;
}

int convertPng(char *filePath, FILE *out) {
  unsigned char header[8];
  FILE *file = fopen(filePath, "rb");
  if (!file) {
    printE("Unable to open file\n");
    return 0;
  }
  fread(header, 1, 8, file);
  if (png_sig_cmp(header, 0, 8))
    return fclose(file), 0; /* not a png file error */

  png_structp pngStruct =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngStruct) {
    fclose(file);
    return 0;
  }
  png_infop pngInfo = png_create_info_struct(pngStruct);
  if (!pngInfo) {
    fclose(file);
    return 0;
  }

  if (setjmp(png_jmpbuf(pngStruct))) {
    png_destroy_read_struct(&pngStruct, &pngInfo, (png_infopp)NULL);
    return fclose(file), 0; /* library error */
  }
  png_init_io(pngStruct, file);
  png_set_sig_bytes(pngStruct, 8);
  png_read_png(pngStruct, pngInfo,
               PNG_TRANSFORM_SCALE_16 | PNG_TRANSFORM_PACKING |
                   PNG_TRANSFORM_GRAY_TO_RGB,
               NULL);

  int width = png_get_image_width(pngStruct, pngInfo);
  int height = png_get_image_height(pngStruct, pngInfo);
  int colorType = png_get_color_type(pngStruct, pngInfo);
  png_bytep *rows = png_get_rows(pngStruct, pngInfo);

  if (!writeByteArray(rows, width, height, colorType, out, filePath)) {
    png_destroy_read_struct(&pngStruct, &pngInfo, NULL);
    fclose(file);
    return 0;
  }
  png_destroy_read_struct(&pngStruct, &pngInfo, (png_infopp)NULL);
  // for (int i = 0; i < height; i++)
  //   free(rows[i]);
  // free(rows);

  fclose(file);
  return 1;
}
int writeByteArray(png_bytep *rows, int width, int height, int colorType,
                   FILE *out, char *filePath) {

  char *spriteHeader = getSpriteHeader(filePath);
  if (!spriteHeader)
    return EXIT_FAILURE;

  fwrite(spriteHeader, sizeof(char), strlen(spriteHeader), out);

  int firstTime = 0;
  for (int i = 0; i < height; i++) {
    int x = 0;
    while (x < width) {
      {
        uint8_t pixel = getPixel(x, i, rows, colorType, width, height);
        if (firstTime == 0) {

          fprintf(out, "0x%x", pixel);
          firstTime = 1;
        } else
          fprintf(out, ",0x%x", pixel);
        x += 8;
      }
    }
  }
  fwrite("};\n", sizeof(char), 3, out);
  free(spriteHeader);
  return 1;
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

  for (int i = 2; i < argc; i++) {

    if (!convertPng(argv[i], out)) {
      fclose(out);
      return EXIT_FAILURE;
    }
  }

  fwrite("#endif", sizeof(char), 6, out);
  fclose(out);
  return EXIT_SUCCESS;
}
