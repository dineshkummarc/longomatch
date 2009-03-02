.intel_syntax noprefix

.section .text

.global ms_audio_healer_start
  .type ms_audio_healer_start, @function
  .balign 16, 0x90
ms_audio_healer_start:
  mov eax, 4
  jmp _ms_utils_invoke_vfunc

.global ms_audio_healer_push_samples
  .type ms_audio_healer_push_samples, @function
  .balign 16, 0x90
ms_audio_healer_push_samples:
  mov eax, 28
  jmp _ms_utils_invoke_vfunc

.global ms_audio_healer_pull_samples
  .type ms_audio_healer_pull_samples, @function
  .balign 16, 0x90
ms_audio_healer_pull_samples:
  mov eax, 32
  jmp _ms_utils_invoke_vfunc

