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

%macro interrupt_common 1
    push %1
    pushaq
    cld
    mov rdi, rsp
    call interrupt_raise
    popaq
    add rsp, 0x10
    iretq
%endmacro

%macro interrupt_handler_no_error 1
align 16
interrupt_handler_%1:
    push 0
    interrupt_common %1
%endmacro

%macro interrupt_handler_error 1
align 16
interrupt_handler_%1:
    interrupt_common %1
%endmacro

interrupt_handler_no_error 0
interrupt_handler_no_error 1
interrupt_handler_no_error 2
interrupt_handler_no_error 3
interrupt_handler_no_error 4
interrupt_handler_no_error 5
interrupt_handler_no_error 6
interrupt_handler_no_error 7
interrupt_handler_error   8
interrupt_handler_no_error 9
interrupt_handler_error   10
interrupt_handler_error   11
interrupt_handler_error   12
interrupt_handler_error   13
interrupt_handler_error   14
interrupt_handler_no_error 15
interrupt_handler_no_error 16
interrupt_handler_error   17
interrupt_handler_no_error 18
interrupt_handler_no_error 19
interrupt_handler_no_error 20
interrupt_handler_error   21
interrupt_handler_no_error 22
interrupt_handler_no_error 23
interrupt_handler_no_error 24
interrupt_handler_no_error 25
interrupt_handler_no_error 26
interrupt_handler_no_error 27
interrupt_handler_no_error 28
interrupt_handler_error   29
interrupt_handler_error   30
interrupt_handler_no_error 31

%assign i 32
%rep 224
interrupt_handler_no_error i
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
