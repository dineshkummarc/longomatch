.intel_syntax noprefix

.section .text

.global ms_buffer_rtp_ext_header_new
  .type ms_buffer_rtp_ext_header_new, @function
  .balign 16, 0x90
ms_buffer_rtp_ext_header_new:
  jmp [_ms_buffer_rtp_ext_header_new_impl]

