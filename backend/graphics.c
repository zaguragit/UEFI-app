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
select_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics, OUT u32 *mode) {
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
	u32 new_mode;
	EFI_STATUS status = select_mode(graphics, &new_mode);
	ASSERT_EFI_STATUS(status, L"init_graphics select_mode");
	status = uefi_call_wrapper(graphics->SetMode, 2, graphics, new_mode);
	ASSERT_EFI_STATUS(status, L"init_graphics SetMode");
	graphicsInfo.protocol = graphics;
	graphicsInfo.bufferBase = (void*)graphics->Mode->FrameBufferBase;
	graphicsInfo.bufferSize = graphics->Mode->FrameBufferSize;
	return EFI_SUCCESS;
}

void setPixel(u32 x, u32 y, u32 color) {
	x = x << 2;
	y = y << 2;
	i32 *addr = graphicsInfo.bufferBase + x + y * graphicsInfo.outputMode.PixelsPerScanLine;
	*addr = color | 0xff000000;
}

void drawRect(i32 fromX, i32 fromY, u32 toX, u32 toY, u32 color) {
	u32 truColor = color | 0xff000000;
	for (i32 y = fromY << 2; y < toY << 2; y += 4) {
		int xx = fromX << 2;
		i32 *addr = graphicsInfo.bufferBase + xx + y * graphicsInfo.outputMode.PixelsPerScanLine;
    	for (i32 x = xx; x < toX << 2; x += 4) *addr++ = truColor;
	}
}

void drawChar(CHAR16 ch, int x, int y, u32 color) {
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

void drawString(String str, u32 x, u32 y, u32 color) {
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

void drawCenteredString(String str, u32 x, u32 y, u32 color) {
  x -= length(str) * 8;
  u32 xx = 0, yy = -7, i = 0;
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