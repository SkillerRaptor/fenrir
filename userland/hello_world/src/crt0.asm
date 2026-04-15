;
; Copyright (c) 2026-present, SkillerRaptor
;
; SPDX-License-Identifier: MIT
;

bits 64

section .text

global _start

extern main
extern syscall

_start:
    call main

    mov rdi, 0x01
    call syscall