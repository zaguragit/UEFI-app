#include <efi.h>
#include <efilib.h>
#include <efiprot.h>

#include "types.c"
#include "string.c"
#include "graphics.c"
#include "asm.c"


struct bootState {
    UINTN                 memoryMapSize;
    EFI_MEMORY_DESCRIPTOR *memoryMap;
    UINTN                 mapKey;
    UINTN                 descriptorSize;
    UINT32                descriptorVersion;
} bootState;

int main();
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  EFI_STATUS status;
  uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
  status = SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
  if (EFI_ERROR(status)) Print(L"[ERROR] Coundn't stop watchdog\r\n");
  uefi_call_wrapper(ST->ConIn->Reset, 2, ST->ConIn, FALSE);

  EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics;
  EFI_GUID graphics_proto = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &graphics_proto, NULL, (void **)&graphics);
  initGraphics(graphics);

  bootState.memoryMap = LibMemoryMap(&bootState.memoryMapSize,
                                       &bootState.mapKey,
                                       &bootState.descriptorSize,
                                       &bootState.descriptorVersion);
  uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, ImageHandle, bootState.mapKey);
  SystemTable->RuntimeServices->SetVirtualAddressMap(bootState.memoryMapSize,
                                                     bootState.descriptorSize,
                                                     bootState.descriptorVersion,
                                                     bootState.memoryMap);
  return main();
}
