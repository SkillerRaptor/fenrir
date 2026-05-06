;
; Copyright (c) 2026-present, SkillerRaptor
;
; SPDX-License-Identifier: MIT
;

bits 64

section .text

global syscall_entry

extern syscall_handler

%macro pushaq 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

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

%define cpu_kernel_rsp 0x10
%define cpu_user_rsp 0x18

syscall_entry:
    swapgs
    mov [gs:cpu_user_rsp], rsp
    mov rsp, [gs:cpu_kernel_rsp]
    pushaq
    mov rdi, rsp
    call syscall_handler
    popaq
    mov rsp, [gs:cpu_user_rsp]
    swapgs
    o64 sysret
