#include "backend/setup.c"

const int dockWidth = 64;

#include "core/console.c"

int main () {

  int centerX = graphicsInfo.outputMode.HorizontalResolution / 2;
  int centerY = graphicsInfo.outputMode.VerticalResolution / 2;

  drawRect(0, 0, dockWidth, graphicsInfo.outputMode.VerticalResolution, 0x111213);

  String in;
  while (1) {
    in = inputColored(L"(QUARTZ): ", 0xff5555, 0xffffff);
    if (streql(in, L"banana")) {
      print(L"BANANAAA!!\n");
    }
  }
  return 0;
}
