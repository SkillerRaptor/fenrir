;
; Copyright (c) 2026-present, SkillerRaptor
;
; SPDX-License-Identifier: MIT
;

bits 64

section .text

global switch_process

%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

switch_process:
    cli
    mov rsp, rdi
    popaq
    add rsp, 0x10
    iretq
