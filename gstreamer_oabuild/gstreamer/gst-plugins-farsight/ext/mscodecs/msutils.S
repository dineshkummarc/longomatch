.intel_syntax noprefix

.section .text

.global _ms_utils_invoke_mfunc
  .type _ms_utils_invoke_mfunc, @function
  .balign 16, 0x90
_ms_utils_invoke_mfunc:
  mov ecx, [esp + 4]

  pop edx
  mov [esp], edx

  jmp eax

.global _ms_utils_invoke_vfunc
  .type _ms_utils_invoke_vfunc, @function
  .balign 16, 0x90
_ms_utils_invoke_vfunc:
  mov ecx, [esp + 4]

  pop edx
  mov [esp], edx

  mov edx, [ecx + 0]
  jmp [edx + eax]

