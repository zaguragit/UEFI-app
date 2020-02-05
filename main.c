#include "backend/setup.c"

#define dockWidth 64

#include "core/console.c"

int main () {

  int centerX = graphicsInfo.outputMode.HorizontalResolution / 2;
  int centerY = graphicsInfo.outputMode.VerticalResolution / 2;

  drawRect(0, 0, dockWidth, graphicsInfo.outputMode.VerticalResolution, 0x111213);

  String in;
  loop() {
    in = inputColored(L"[quartz]: ", 0xff5555, 0xffffff);
    if (streql(in, L"banana")) {
      print(L"BANANAAA!!\n");
    }
  }
  return 0;
}