
//#define GRAPHICS_MOST_APPROPRIATE_H 1080
//#define GRAPHICS_MOST_APPROPRIATE_W 1920
#include "font.c"

#define GRAPHICS_MOST_APPROPRIATE_H 720
#define GRAPHICS_MOST_APPROPRIATE_W 1080

#define ASSERT_EFI_STATUS(x, n) {if(EFI_ERROR((x))) { Print(n": %r\n", x); return x; }}

struct graphicsInfo {
  EFI_GRAPHICS_OUTPUT_PROTOCOL         *protocol;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  outputMode;
  void*                                 bufferBase;
  UINT8                                 bufferSize;
} graphicsInfo;

EFI_STATUS
select_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics, OUT UINT32 *mode) {
  *mode = graphics->Mode->Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION most_appropriate_info;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
  UINTN size;

  // Initialize info of current mode
  EFI_STATUS status = uefi_call_wrapper(graphics->QueryMode, 4, graphics, *mode, &size, &info);
  ASSERT_EFI_STATUS(status, L"select_mode");
  most_appropriate_info = *info;

  for(UINT32 i = 0; i < graphics->Mode->MaxMode; i += 1) {
    EFI_STATUS status = uefi_call_wrapper(graphics->QueryMode, 4, graphics, i, &size, &info);
    ASSERT_EFI_STATUS(status, L"select_mode");
    if(info->PixelFormat != PixelRedGreenBlueReserved8BitPerColor &&
       info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor) continue;
    if(info->HorizontalResolution > GRAPHICS_MOST_APPROPRIATE_W ||
       info->VerticalResolution   > GRAPHICS_MOST_APPROPRIATE_H) continue;

    if(info->VerticalResolution   == GRAPHICS_MOST_APPROPRIATE_H &&
       info->HorizontalResolution == GRAPHICS_MOST_APPROPRIATE_W) {
      most_appropriate_info = *info;
      *mode = i;
      break;
    }

    if(info->VerticalResolution > most_appropriate_info.VerticalResolution) {
      most_appropriate_info = *info;
      *mode = i;
    }
  }
  graphicsInfo.outputMode = most_appropriate_info;
  return EFI_SUCCESS;
}

EFI_STATUS
initGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics) {
  UINT32 new_mode;
  EFI_STATUS status = select_mode(graphics, &new_mode);
  ASSERT_EFI_STATUS(status, L"init_graphics select_mode");
  status = uefi_call_wrapper(graphics->SetMode, 2, graphics, new_mode);
  ASSERT_EFI_STATUS(status, L"init_graphics SetMode");
  graphicsInfo.protocol = graphics;
  graphicsInfo.bufferBase = (void*)graphics->Mode->FrameBufferBase;
  graphicsInfo.bufferSize = graphics->Mode->FrameBufferSize;
  return EFI_SUCCESS;
}

void setPixel(int x, int y, UINT32 color) {
  x *= 4;
  y *= 4;
  int32_t *addr = graphicsInfo.bufferBase + x + y * graphicsInfo.outputMode.PixelsPerScanLine;
  *addr = color | 0xff000000;
}

void drawRect(int fromX, int fromY, int toX, int toY, UINT32 color) {
  for (int xx = fromX; xx < toX; xx++)
    for (int yy = fromY; yy < toY; yy++)
      setPixel(xx, yy, color);
}

void drawChar(CHAR16 ch, int x, int y, UINT32 color) {
  FontGlyph g = getFontGlyph(ch);
  for (int yy = 0; yy < 7; yy++) {
    int line = g[yy];
    for (int xx = 0; xx < 8; xx++) {
      if (line & 0b10000000) {
        setPixel(x + xx * 2, y + yy * 2, color);
        setPixel(x + xx * 2 + 1, y + yy * 2, color);
        setPixel(x + xx * 2, y + yy * 2 + 1, color);
        setPixel(x + xx * 2 + 1, y + yy * 2 + 1, color);
      }
      line = line << 1;
    }
  }
}

void drawString(String str, int x, int y, UINT32 color) {
  int xx = 0, yy = 4, i = 0;
  while(str[i]) {
    if (str[i] == 0x000A) {
      yy += 18;
      xx = 0;
    } else {
      drawChar(str[i], x + xx, y + yy, color);
      xx += 16;
    }
    i++;
  }
}

void drawCenteredString(String str, int x, int y, UINT32 color) {
  x -= length(str) * 8;
  int xx = 0, yy = -7, i = 0;
  while(str[i]) {
    // if (str[i] == 0x000A) {
    //   yy += 8;
    //   xx = 0;
    // } else {
      drawChar(str[i], x + xx, y + yy, color);
      xx += 16;
    // }
    i++;
  }
}
