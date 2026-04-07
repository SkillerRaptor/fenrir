;
; Copyright (c) 2026-present, SkillerRaptor
;
; SPDX-License-Identifier: MIT
;

bits 64

section .text

global _start

extern main

_start:
    call main
loop:
    jmp loop
    ; TODO: Call exit syscall here