;
; Copyright (c) 2026-present, SkillerRaptor
;
; SPDX-License-Identifier: MIT
;

bits 64

section .text

global load_idt

extern interrupt_raise

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

load_idt:
    lidt [rdi]
    ret

%macro interrupt_handler 1
align 16
interrupt_handler_%1:
    %if (%1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17 || %1 == 21 || %1 == 29 || %1 == 30)
    %else
    push 0
    %endif
    push %1
    pushaq
    cld
    mov rdi, rsp
    call interrupt_raise
    popaq
    add rsp, 0x10
    iretq
%endmacro

%assign i 0
%rep 256
interrupt_handler i
%assign i i+1
%endrep

section .data

%macro interrupt_handler_address 1
dq interrupt_handler_%1
%endmacro

global interrupt_handlers
interrupt_handlers:
	%assign i 0
	%rep 256
	interrupt_handler_address i
	%assign i i+1
	%endrep
