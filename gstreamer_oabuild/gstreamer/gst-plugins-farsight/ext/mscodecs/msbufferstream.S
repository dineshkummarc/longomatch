.intel_syntax noprefix

.section .text

.global ms_buffer_stream_new
  .type ms_buffer_stream_new, @function
  .balign 16, 0x90
ms_buffer_stream_new:
  jmp [_ms_buffer_stream_class_new_impl]

.global ms_buffer_stream_add_buffer
  .type ms_buffer_stream_add_buffer, @function
  .balign 16, 0x90
ms_buffer_stream_add_buffer:
  mov eax, [_ms_buffer_stream_class_add_buffer_impl]
  jmp _ms_utils_invoke_mfunc

.global ms_buffer_stream_update_offset_and_size
  .type ms_buffer_stream_update_offset_and_size, @function
  .balign 16, 0x90
ms_buffer_stream_update_offset_and_size:
  mov eax, [_ms_buffer_stream_update_offset_and_size_impl]
  jmp _ms_utils_invoke_mfunc

