.intel_syntax noprefix

.section .text

.global ms_encoder_init
  .type ms_encoder_init, @function
  .balign 16, 0x90
ms_encoder_init:
  mov eax, 0
  jmp _ms_utils_invoke_vfunc

.global ms_encoder_encode
  .type ms_encoder_encode, @function
  .balign 16, 0x90
ms_encoder_encode:
  mov eax, 4
  jmp _ms_utils_invoke_vfunc

.global ms_encoder_set_bitrate
  .type ms_encoder_set_bitrate, @function
  .balign 16, 0x90
ms_encoder_set_bitrate:
  mov eax, 16
  jmp _ms_utils_invoke_vfunc

.global ms_encoder_set_lossrate
  .type ms_encoder_set_lossrate, @function
  .balign 16, 0x90
ms_encoder_set_lossrate:
  mov eax, 20
  jmp _ms_utils_invoke_vfunc

.global ms_decoder_init
  .type ms_decoder_init, @function
  .balign 16, 0x90
ms_decoder_init:
  mov eax, 0
  jmp _ms_utils_invoke_vfunc

.global ms_decoder_decode
  .type ms_decoder_decode, @function
  .balign 16, 0x90
ms_decoder_decode:
  mov eax, 4
  jmp _ms_utils_invoke_vfunc

