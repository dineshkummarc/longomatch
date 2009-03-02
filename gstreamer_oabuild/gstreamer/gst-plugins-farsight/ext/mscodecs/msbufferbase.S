.intel_syntax noprefix

.section .text

.global ms_buffer_base_init
  .type ms_buffer_base_init, @function
  .balign 16, 0x90
ms_buffer_base_init:
  mov eax, [_ms_buffer_base_class_init_impl]
  jmp _ms_utils_invoke_mfunc

