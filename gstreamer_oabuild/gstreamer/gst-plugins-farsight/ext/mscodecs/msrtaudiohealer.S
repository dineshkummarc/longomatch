.intel_syntax noprefix

.section .text

.global ms_rtaudio_healer_new
  .type ms_rtaudio_healer_new, @function
  .balign 16, 0x90
ms_rtaudio_healer_new:
  jmp [_ms_rtaudio_healer_class_new_impl]

.global ms_rtaudio_healer_free
  .type ms_rtaudio_healer_free, @function
  .balign 16, 0x90
ms_rtaudio_healer_free:
  jmp [_ms_rtaudio_healer_class_free_impl]

